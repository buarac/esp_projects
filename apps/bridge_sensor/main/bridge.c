#include "bridge.h"

static const char *TAG = "BRIDGE_MAIN";




// MAIN
void app_main(void) {
    ESP_LOGV(TAG, "main");
    
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK( nvs_flash_erase() );
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );


    ESP_ERROR_CHECK(bridge_wifi_init());
    ESP_ERROR_CHECK(bridge_mqtt_app_start());
    ESP_ERROR_CHECK(bridge_espnow_init());

    while(1) {
        /*
        sprintf(topic, BRIDGE_MQTT_TOPIC_TEMP, id);
        sprintf(message, "%2.2f", 23.78);
        msg_id = esp_mqtt_client_publish(bridge_mqtt_client, topic, message, 0, 1, 0);
        ESP_LOGI(TAG, "publish message %d to %s", msg_id, topic);

        sprintf(topic, BRIDGE_MQTT_TOPIC_HUMI, id);
        sprintf(message, "%2.2f", 69.69);
        msg_id = esp_mqtt_client_publish(bridge_mqtt_client, topic, message, 0, 1, 0);
        ESP_LOGI(TAG, "publish message %d to %s", msg_id, topic);

        sprintf(topic, BRIDGE_MQTT_TOPIC_PRES, id);
        sprintf(message, "%4.2f", 1021.34);
        msg_id = esp_mqtt_client_publish(bridge_mqtt_client, topic, message, 0, 1, 0);
        ESP_LOGI(TAG, "publish message %d to %s", msg_id, topic);

        bridge_espnow_send_test();
        */
        uint32_t free_heap_mem = esp_get_free_heap_size();
        ESP_LOGW(TAG, "Free heap memory: %zu byte(s)", free_heap_mem);
        ESP_LOGW(TAG, "");
        vTaskDelay(30000/portTICK_RATE_MS);
    }
}