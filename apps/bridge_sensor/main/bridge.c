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

    measurementCount = 0;

    ESP_ERROR_CHECK(bridge_event_init());
    ESP_ERROR_CHECK(bridge_wifi_init());
    ESP_ERROR_CHECK(bridge_mqtt_app_start());
    ESP_ERROR_CHECK(bridge_espnow_init());

    while(1) {
        
        uint32_t free_heap_mem = esp_get_free_heap_size();
        ESP_LOGW(TAG, "Free heap memory : %zu byte(s)", free_heap_mem);
        ESP_LOGW(TAG, "Measurement count: %zu", measurementCount);
        ESP_LOGW(TAG, "-------------------------------");
        bridge_dump_per_task_heap_info();
        vTaskDelay(30000/portTICK_RATE_MS);
    }
}