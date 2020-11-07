#include "bridge.h"

static const char *TAG = "BRIDGE_ESPN";
static uint8_t broadcast_mac[ESP_NOW_ETH_ALEN] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

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
    ESP_LOGI(TAG, "Recv data from "MACSTR"", MAC2STR(mac_addr));
    if ( len == sizeof(sensor_info_t) ) {
        sensor_info_t* sensor = (sensor_info_t*)data;

        if ( sensor->magic == SENSOR_MAGIC_ID && 
             sensor->type == SENSOR_BME280 ) {

            uint16_t crc = sensor->crc16;
            sensor->crc16 = 0;
            uint16_t cal_crc = esp_crc16_le(UINT16_MAX, (uint8_t*)sensor, sizeof(sensor_info_t));    
            if ( crc != cal_crc ) {
                ESP_LOGE(TAG, "bad crc, message ignored");
            }    
            else {
                char topic[64];
                char message[16];

                bridge_espnow_add_peer(mac_addr);

                sprintf(topic, BRIDGE_MQTT_TOPIC_TEMP, sensor->id);
                sprintf(message, "%2.2f", sensor->data.bme280.temp);
                esp_mqtt_client_publish(bridge_mqtt_client, topic, message, 0, 1, 0);

                sprintf(topic, BRIDGE_MQTT_TOPIC_HUMI, sensor->id);
                sprintf(message, "%2.2f", sensor->data.bme280.humi);
                esp_mqtt_client_publish(bridge_mqtt_client, topic, message, 0, 1, 0);

                sprintf(topic, BRIDGE_MQTT_TOPIC_PRES, sensor->id);
                sprintf(message, "%4.2f", sensor->data.bme280.pres);
                esp_mqtt_client_publish(bridge_mqtt_client, topic, message, 0, 1, 0);

                /*    
                char *ok_str = "OK";
                if ( esp_now_send(mac_addr, (uint8_t*)ok_str, 2) != ESP_OK ) {
                    ESP_LOGE(TAG, "send ack failed");
                }
                */
            }
        }
    }
}

esp_err_t bridge_espnow_init(void) {
    ESP_LOGV(TAG, "bridge_espnow_init");

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