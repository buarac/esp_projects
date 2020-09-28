#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_now.h"
#include "esp_sleep.h"
#include <string.h>
//#include "i2cdev.h"
//#include "i2c_device.h"
//#include "bme280.h"
#include "bme280_device.h"


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

extern "C" {
    void app_main(void);
};

void app_main(void) {

    ESP_LOGV(TAG, "app_main");

    BME280_Device dev(BME280_ADDR, BME280_PORT, BME280_SDA, BME280_SCL, BME280_FREQ);

    if ( !dev.Init(bme280_params)  ) {
        ESP_LOGE(TAG, "failed to init bme280 device");
        return;
    }
    ESP_LOGI(TAG, "BME280 device initialized");

    BME280_Data data;
    int measureId = 1;

    while(true) {
        data = dev.ReadDataForced();
        printf("-----------------------\n");
        printf("measure id : %d\n", measureId++);
        printf("temperature: %.2f\n", data.temperature);
        printf("humidity   : %.2f\n", data.humidity);
        printf("pressure   : %.2f\n", data.pressure);
        vTaskDelay(60000/portTICK_RATE_MS);
    }    
}

/*
static inline uint8_t rd_reg(i2c_device_t* i2c, uint8_t reg) {
    uint8_t reg_data = reg;
    uint8_t data;

    if ( i2c_device_read(i2c, &reg_data, 1, &data, 1) != ESP_OK ) {
        ESP_LOGE(TAG, "failed to read i2c device");
        data = 0;
    }
    return data;
}

static inline void wr_reg(i2c_device_t* i2c, uint8_t reg, uint8_t data) {
    uint8_t reg_data = reg;
    uint8_t data_data= data;

    if ( i2c_device_write(i2c, &reg_data, 1, &data_data, 1) != ESP_OK ) {
        ESP_LOGE(TAG, "failed to write i2c device");
    }
}

void bme280_soft_reset(i2c_device_t* i2c) {
    wr_reg(i2c, 0xe0, 0xb6);
}

bool bme280_check_chid_id(i2c_device_t* i2c) {
    bool bBme280 = false;

    uint8_t data = rd_reg(i2c, 0xd0);
    if ( data == 0x60 ) {
        bBme280 = true;
    }
    return bBme280;
}


void app_main(void) {
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK( nvs_flash_erase() );
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );


    I2C_Device dev(BME280_ADDR, BME280_PORT, BME280_SDA, BME280_SCL, BME280_FREQ);

    ESP_ERROR_CHECK(dev.Init());
*/
/*
    bme280_t* bme = bme280_setup(&bme280_i2c_params, NULL);

    ESP_ERROR_CHECK(bme280_init_device(bme));
*/
/*
    ESP_LOGV(TAG, "init i2c device");
    i2c_device_t* device = i2c_device_create(BME280_ADDR, BME280_PORT, BME280_SDA, BME280_SCL, BME280_FREQ);
    ESP_ERROR_CHECK ( i2c_device_init ( device ) );


    if ( !bme280_check_chid_id(device) ) {
        ESP_LOGE(TAG, "Not a bme280 device");
    }
    else {
        ESP_LOGD(TAG, "A bme280 device");
    }

    bme280_soft_reset(device);

    if ( !bme280_check_chid_id(device) ) {
        ESP_LOGE(TAG, "Not a bme280 device");
    }
    else {
        ESP_LOGD(TAG, "A bme280 device");
    }

    i2c_device_deinit(device);
}
*/