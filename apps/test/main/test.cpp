#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_now.h"
#include "esp_sleep.h"
#include <string.h>
#include "bme280_device.h"
#include "sensors.h"
#include "esp_crc.h"

static const char *TAG = "TEST";

#define BME280_PORT CONFIG_BME280_I2C_PORT
#define BME280_ADDR CONFIG_BME280_I2C_ADDR
#define BME280_SDA  CONFIG_BME280_I2C_SDA
#define BME280_SCL  CONFIG_BME280_I2C_SCL
#define BME280_FREQ CONFIG_BME280_I2C_FREQ

const BME280_Params bme280_params {
        .filter = BME280_FILTER_OFF,
        .mode = BME280_SLEEP_MODE,
        .over_samp_temp = BME280_OVERSAMPLING_1,
        .over_samp_humi = BME280_OVERSAMPLING_1,
        .over_samp_pres = BME280_OVERSAMPLING_1,
        .standby = BME280_STANDBY_1000
};

const I2C_Params bme280_i2c_params {
    .addr = BME280_ADDR,
    .port = BME280_PORT,
    .sda  = BME280_SDA,
    .scl  = BME280_SCL,
    .freq = BME280_FREQ
};

extern "C" {
    void app_main(void);
};

#define WAKEUP_TIME_SEC 120
static RTC_DATA_ATTR uint32_t measureId = 0;
static uint8_t broadcast_mac[ESP_NOW_ETH_ALEN] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
static uint8_t data_sent;

static void example_wifi_init(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_AP) );
    ESP_ERROR_CHECK( esp_wifi_start());

        // channel;
    uint8_t primary_channel;
    wifi_second_chan_t secondary_channel;

    ESP_ERROR_CHECK(esp_wifi_get_channel(&primary_channel, &secondary_channel));
    ESP_LOGI(TAG, "Primary channel %d", primary_channel);

    esp_wifi_set_promiscuous(true);
    esp_wifi_set_channel(11, WIFI_SECOND_CHAN_NONE);
    esp_wifi_set_promiscuous(false);    

    ESP_ERROR_CHECK(esp_wifi_get_channel(&primary_channel, &secondary_channel));
    ESP_LOGI(TAG, "Primary channel %d", primary_channel);
}

static void example_espnow_send_cb(const uint8_t *mac_addr, esp_now_send_status_t status) {
    ESP_LOGI(TAG, "espnow sent data to " MACSTR ", status=%d", MAC2STR(mac_addr), status);
    data_sent = 1;
}

static void example_espnow_recv_cb(const uint8_t *mac_addr, const uint8_t *data, int len) {
    ESP_LOGI(TAG, "espnow received data from " MACSTR ", len = %d", MAC2STR(mac_addr), len);
    ESP_LOG_BUFFER_HEXDUMP(TAG, data, len, ESP_LOG_WARN);
}

static esp_err_t example_espnow_init(void) {
    ESP_ERROR_CHECK( esp_now_init() );
    ESP_ERROR_CHECK( esp_now_register_send_cb(example_espnow_send_cb) );
    ESP_ERROR_CHECK( esp_now_register_recv_cb(example_espnow_recv_cb) );

    /* Add broadcast peer information to peer list. */
    esp_now_peer_info_t *peer = (esp_now_peer_info_t*)malloc(sizeof(esp_now_peer_info_t));
    if (peer == NULL) {
        ESP_LOGE(TAG, "Malloc peer information fail");
        return ESP_FAIL;
    }
    memset(peer, 0, sizeof(esp_now_peer_info_t));
    peer->channel = 11;
    peer->ifidx = ESP_IF_WIFI_AP;
    peer->encrypt = false;
    memcpy(peer->peer_addr, broadcast_mac, ESP_NOW_ETH_ALEN);
    ESP_ERROR_CHECK( esp_now_add_peer(peer) );
    free(peer);

    return ESP_OK;
}

static void read_and_send_measure() {
    data_sent = 1;
    BME280_Device dev(bme280_i2c_params);

    if ( !dev.Init(bme280_params)  ) {
        ESP_LOGE(TAG, "failed to init bme280 device");
        return;
    }
    ESP_LOGI(TAG, "BME280 device initialized");
    data_sent = 0;
    sensor_info_t sensor;
    ESP_LOGI(TAG, "start measure");
        memset((void*)&sensor, 0, sizeof(sensor_info_t));
        sensor.id = 0x03;
        sensor.magic = SENSOR_MAGIC_ID;
        sensor.type = SENSOR_BME280;
        BME280_Data data = dev.ReadDataForced();
        sensor.data.bme280.temp = data.temperature;
        sensor.data.bme280.humi = data.humidity;
        sensor.data.bme280.pres = data.pressure;
        sensor.crc16 = esp_crc16_le(UINT16_MAX, (uint8_t*)&sensor, sizeof(sensor_info_t));
        ESP_LOGI(TAG, "measure done");
        ESP_LOGI(TAG, "-----------------------");
        ESP_LOGI(TAG, "measure id : %d", ++measureId);
        ESP_LOGI(TAG, "temperature: %.2f", sensor.data.bme280.temp);
        ESP_LOGI(TAG, "humidity   : %.2f", sensor.data.bme280.humi);
        ESP_LOGI(TAG, "pressure   : %.2f", sensor.data.bme280.pres);

        if ( esp_now_send(broadcast_mac, (uint8_t*)&sensor, sizeof(sensor_info_t)) != ESP_OK ) {
            ESP_LOGE(TAG, "send espnow packet failed");
        }

    while(data_sent!= 1) { 
        vTaskDelay(1/portTICK_RATE_MS);
    } 
}

void app_main(void) {

    ESP_LOGV(TAG, "app_main");

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK( nvs_flash_erase() );
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );


    example_wifi_init();
    ESP_ERROR_CHECK(example_espnow_init());    

    read_and_send_measure();

    esp_now_deinit();

    ESP_LOGI(TAG, "enabling timer wakeup, %d sec", WAKEUP_TIME_SEC);
    esp_sleep_enable_timer_wakeup(WAKEUP_TIME_SEC * 1000000);
    ESP_LOGI(TAG, "entering deep sleep");
    esp_deep_sleep_start();
}
