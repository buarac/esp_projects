#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_now.h"
#include "esp_sleep.h"
#include "esp_crc.h"
#include <string.h>
#include "bridge_espnow.h"


static const char *TAG = "BRIDGE_ESPNOW";

static xQueueHandle s_bridge_espnow_event_queue;

static uint8_t broadcast_mac[ESP_NOW_ETH_ALEN] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
/*
extern "C" {
    void app_main(void);
};
*/

/* WiFi should start before using ESPNOW */
static void bridge_wifi_init(void) {
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    ESP_ERROR_CHECK( esp_wifi_set_mode(ESPNOW_WIFI_MODE) );
    ESP_ERROR_CHECK( esp_wifi_start());
}

/*
static void example_espnow_deinit() {
    vSemaphoreDelete(s_bridge_espnow_event_queue);
    esp_now_deinit();
}
*/


// espnow send callback
static void bridge_espnow_send_cb(const uint8_t *mac_addr, esp_now_send_status_t status) {
    bridge_espnow_event_t evt;
    bridge_espnow_event_send_cb_t *send_cb = &evt.info.send_cb;

    if (mac_addr == NULL) {
        ESP_LOGE(TAG, "Send cb arg error");
        return;
    }

    evt.id = BRIDGE_ESPNOW_SEND_CB;
    memcpy(send_cb->mac_addr, mac_addr, ESP_NOW_ETH_ALEN);
    send_cb->status = status;
    if (xQueueSend(s_bridge_espnow_event_queue, &evt, portMAX_DELAY) != pdTRUE) {
        ESP_LOGW(TAG, "Send send queue fail");
    }
}

// espnow receive callback
static void bridge_espnow_recv_cb(const uint8_t *mac_addr, const uint8_t *data, int len) {
    bridge_espnow_event_t evt;
    bridge_espnow_event_recv_cb_t *recv_cb = &evt.info.recv_cb;

    if (mac_addr == NULL || data == NULL || len <= 0) {
        ESP_LOGE(TAG, "Receive cb arg error");
        return;
    }

    evt.id = BRIDGE_ESPNOW_RECV_CB;
    memcpy(recv_cb->mac_addr, mac_addr, ESP_NOW_ETH_ALEN);
    recv_cb->data = malloc(len);
    if (recv_cb->data == NULL) {
        ESP_LOGE(TAG, "Malloc receive data fail");
        return;
    }
    memcpy(recv_cb->data, data, len);
    recv_cb->data_len = len;
    if (xQueueSend(s_bridge_espnow_event_queue, &evt, portMAX_DELAY) != pdTRUE) {
        ESP_LOGW(TAG, "Send receive queue fail");
        free(recv_cb->data);
    }
}


static void bridge_espnow_event_task(void *pvParameter) {
    bridge_espnow_event_t evt;

    while (xQueueReceive(s_bridge_espnow_event_queue, &evt, portMAX_DELAY) == pdTRUE) {
        switch (evt.id) {
            case BRIDGE_ESPNOW_SEND_CB: {
                ESP_LOGI(TAG, "ESP-NOW Send event");
                break;
            }
            case BRIDGE_ESPNOW_RECV_CB: {
                bridge_espnow_event_recv_cb_t* recv_cb = &evt.info.recv_cb;
                ESP_LOGI(TAG, "ESP-NOW Receive event from: "MACSTR", len: %d", MAC2STR(recv_cb->mac_addr), recv_cb->data_len);
                ESP_LOG_BUFFER_HEXDUMP(TAG, recv_cb->data, recv_cb->data_len, ESP_LOG_WARN);
                pattern_t* pat = (pattern_t*)recv_cb->data;
                uint16_t recv_crc = pat->crc;
                pat->crc = 0;
                uint16_t calc_crc = esp_crc16_le(UINT16_MAX, (uint8_t*)recv_cb->data, recv_cb->data_len);
                if ( recv_crc == calc_crc ) {
                    ESP_LOGI(TAG, "CRC OK");
                }
                else {
                    ESP_LOGE(TAG, "RECEIVED CRC=%04x, COMPUTED CRC=%04x", recv_crc, calc_crc);
                }
                free(recv_cb->data);
                break;
            }
            default: {
                ESP_LOGE(TAG, "callback error, event.id=%d", evt.id);
                break;
            }
        }
    }
}


static esp_err_t bridge_espnow_init(void) {

    s_bridge_espnow_event_queue = xQueueCreate(ESPNOW_QUEUE_SIZE, sizeof(bridge_espnow_event_t));
    if (s_bridge_espnow_event_queue == NULL) {
        ESP_LOGE(TAG, "Create mutex fail");
        return ESP_FAIL;
    }

    /* Initialize ESPNOW and register sending and receiving callback function. */
    ESP_ERROR_CHECK( esp_now_init() );
    ESP_ERROR_CHECK( esp_now_register_send_cb(bridge_espnow_send_cb) );
    ESP_ERROR_CHECK( esp_now_register_recv_cb(bridge_espnow_recv_cb) );

    /* Set primary master key. */
    // ESP_ERROR_CHECK( esp_now_set_pmk((uint8_t *)CONFIG_ESPNOW_PMK) );

    /* Add broadcast peer information to peer list. */
    esp_now_peer_info_t *peer = malloc(sizeof(esp_now_peer_info_t));
    if (peer == NULL) {
        ESP_LOGE(TAG, "Malloc peer information fail");
        vSemaphoreDelete(s_bridge_espnow_event_queue);
        esp_now_deinit();
        return ESP_FAIL;
    }
    memset(peer, 0, sizeof(esp_now_peer_info_t));
//    peer->channel = CONFIG_ESPNOW_CHANNEL;
    peer->channel = 1;
    peer->ifidx = ESPNOW_WIFI_IF;
    peer->encrypt = false;
    memcpy(peer->peer_addr, broadcast_mac, ESP_NOW_ETH_ALEN);
    ESP_ERROR_CHECK( esp_now_add_peer(peer) );
    free(peer);

    xTaskCreate(bridge_espnow_event_task, "bridge_espnow_event_task", 2048, NULL, 4, NULL);

    return ESP_OK;
}

void app_main(void) {

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK( nvs_flash_erase() );
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );

    bridge_wifi_init();
    bridge_espnow_init();
}
