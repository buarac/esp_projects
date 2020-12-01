#include "bridge.h"

static const char *TAG = "BRIDGE_MAIN";

i2c_bus_handle_t i2c_bus_create(uint16_t data_1, uint32_t data_2) {
    i2c_bus_t* bus = calloc(1, sizeof(i2c_bus_t));
    bus->data1 = data_1;
    bus->data2 = data_2;
    return (i2c_bus_handle_t)bus;
}

esp_err_t i2c_bus_delete(i2c_bus_handle_t bus) {
    if ( bus == NULL ) {
        return ESP_FAIL;
    }
    free(bus);
    return ESP_OK;
}

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
        //bridge_dump_per_task_heap_info();
        vTaskDelay(60000/portTICK_RATE_MS);
    }
}