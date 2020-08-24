#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_now.h"
#include "esp_sleep.h"
#include <string.h>
#include "i2cdev.h"

static const char* TAG = "i2cdev";

void app_main(void) {
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK( nvs_flash_erase() );
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );

    i2cdev_params_t p = {
        0x40,
        0,
        21, 22,
        100000
    };

    ret = i2cdev_setup(&p);
    ESP_LOGI(TAG, "i2c device setup %d", ret);
}