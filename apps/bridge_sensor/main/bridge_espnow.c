#include "bridge.h"

static const char *TAG = "BRIDGE_ESPN";
static uint8_t broadcast_mac[ESP_NOW_ETH_ALEN] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

static EventGroupHandle_t s_bridge_espnow_event_group;

esp_err_t bridge_espnow_add_peer(const uint8_t* mac_addr) {
    ESP_LOGV(TAG, "adding peer "MACSTR"", MAC2STR(mac_addr));

    if ( !esp_now_is_peer_exist(mac_addr) ) {
        esp_now_peer_info_t* peer = malloc(sizeof(esp_now_peer_info_t));
        if ( peer == NULL ) {
            ESP_LOGE(TAG, "malloc peer fail");
            return ESP_FAIL;
        }
        memset(peer, 0, sizeof(esp_now_peer_info_t));
        peer->channel = BRIDGE_WIFI_CHANNEL;
        peer->ifidx = BRIDGE_WIFI_IF;
        peer->encrypt = false;
        memcpy(peer->peer_addr, mac_addr, ESP_NOW_ETH_ALEN);
        ESP_ERROR_CHECK( esp_now_add_peer(peer) );
        free(peer);    
        ESP_LOGD(TAG, "peer added.");    
    } else {
        ESP_LOGD(TAG, "peer already exists.");
    }


    return ESP_OK;
}

static void bridge_espnow_send_cb(const uint8_t *mac_addr, esp_now_send_status_t status) {
    ESP_LOGI(TAG, "Send data to "MACSTR", status: %d", MAC2STR(mac_addr), status);
}

static void bridge_espnow_recv_cb(const uint8_t *mac_addr, const uint8_t *data, int len) {
    ESP_LOGI(TAG, "Recv data from "MACSTR" for %d byte(s)", MAC2STR(mac_addr), len);

    bridge_event_t evt;
    bridge_espnow_recv_event_t* recv = &evt.info.espnow_recv;

    evt.id = BRIDGE_ESPNOW_RECV_EVENT;
    memcpy(recv->mac_addr, mac_addr, ESP_NOW_ETH_ALEN);

    if ( len > 0 ) {
        recv->data = malloc ( len );
        if ( recv->data == NULL ) {
            ESP_LOGE(TAG, "failed to malloc memory for BRIDGE_ESPNOW_RECV_EVENT");
        }
        else {
            memcpy(recv->data, data, len);
            recv->data_len = len;

            //ESP_LOGI(TAG, "new recv event len = %d", recv->data_len);
            //ESP_LOG_BUFFER_HEXDUMP(TAG, recv, sizeof(bridge_espnow_recv_event_t), ESP_LOG_INFO);
            //ESP_LOG_BUFFER_HEXDUMP(TAG, &evt, sizeof(bridge_event_t), ESP_LOG_WARN);

            if ( bridge_event_publish(&evt) != ESP_OK ) {
                ESP_LOGE(TAG, "failed to publish BRIDGE_ESPNOW_RECV_EVENT");
                free(recv->data);
            }
        }
    }
}

esp_err_t bridge_espnow_init(void) {
    ESP_LOGV(TAG, "bridge_espnow_init");

    s_bridge_espnow_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK( esp_now_init() );
    ESP_ERROR_CHECK( esp_now_register_send_cb(bridge_espnow_send_cb) );
    ESP_ERROR_CHECK( esp_now_register_recv_cb(bridge_espnow_recv_cb) );
    ESP_ERROR_CHECK( bridge_espnow_add_peer(broadcast_mac));

    return ESP_OK;
}


esp_err_t bridge_espnow_send_test(void) {
    uint8_t data[32];
    esp_err_t ret = ESP_OK;

    esp_fill_random(data, 32);
    if ( esp_now_send(broadcast_mac, data, 32) != ESP_OK ) {
        ESP_LOGE(TAG, "bridge_espnow_send_test error");
        ret = ESP_FAIL;
    }
    return ret;
}