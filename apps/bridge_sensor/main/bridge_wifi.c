#include "bridge.h"

static const char *TAG = "BRIDGE_WIFI";


//
// Init Wifi connection
// 
static EventGroupHandle_t s_bridge_wifi_event_group;
static int s_bridge_retry_num = 0;

// wifi event handler
static void bridge_wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    ESP_LOGV(TAG, "bridge_wifi_event_handler");

    if ( event_base == WIFI_EVENT ) {
        ESP_LOGV(TAG, "bridge_wifi_event_handler::WIFI_EVENT");
        if ( event_id == WIFI_EVENT_STA_START ) {
            ESP_LOGI(TAG, "bridge_wifi_event_handler::WIFI_EVENT_STA_START");
            esp_wifi_connect();
        }
        else if ( event_id == WIFI_EVENT_STA_DISCONNECTED ) {
            if ( s_bridge_retry_num < BRIDGE_WIFI_MAXIMUM_RETRY ) {
                ESP_LOGI(TAG, "ridge_wifi_event_handler::WIFI_EVENT_STA_DISCONNECTED");
                esp_wifi_connect();
                ++s_bridge_retry_num;
            }
            else {
                xEventGroupSetBits(s_bridge_wifi_event_group, BRIDGE_WIFI_FAIL_BIT);
            }
        }
    }
    else if ( event_base == IP_EVENT ) {
        ESP_LOGV(TAG, "bridge_wifi_event_handler::IP_EVENT");
        if ( event_id == IP_EVENT_STA_GOT_IP ) {
            ip_event_got_ip_t* event = (ip_event_got_ip_t*)event_data;
            ESP_LOGI(TAG, "got ip: "IPSTR"", IP2STR(&event->ip_info.ip));
            s_bridge_retry_num = 0;
            xEventGroupSetBits(s_bridge_wifi_event_group, BRIDGE_WIFI_CONNECTED_BIT);
        }
    }
}


esp_err_t bridge_wifi_init(void) {
    ESP_LOGV(TAG, "bridge_wifi_init");
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );

    s_bridge_wifi_event_group = xEventGroupCreate();
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &bridge_wifi_event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &bridge_wifi_event_handler,
                                                        NULL,
                                                        &instance_got_ip));

 //   ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    //
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = BRIDGE_WIFI_SSID,
            .password = BRIDGE_WIFI_PASSWORD,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            .pmf_cfg = {
                .capable = true,
                .required= false,
            },
        },
    };
    ESP_ERROR_CHECK( esp_wifi_set_mode(BRIDGE_WIFI_MODE) );
    ESP_ERROR_CHECK(esp_wifi_set_config(BRIDGE_WIFI_IF, &wifi_config) );
    ESP_ERROR_CHECK( esp_wifi_start());
    ESP_LOGI(TAG, "wifi_init_sta finished.");
    ESP_LOGI(TAG, "connecting...");
    EventBits_t bits = xEventGroupWaitBits(s_bridge_wifi_event_group,
            BRIDGE_WIFI_CONNECTED_BIT | BRIDGE_WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    esp_err_t err = ESP_OK;
    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & BRIDGE_WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "connected to ap SSID:%s", BRIDGE_WIFI_SSID);
        err = ESP_OK;
    } else if (bits & BRIDGE_WIFI_FAIL_BIT) {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s", BRIDGE_WIFI_SSID);
        err = ESP_FAIL;
    } else {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
        err = ESP_FAIL;
    }
    /* The event will not be processed after unregister */
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id));
    vEventGroupDelete(s_bridge_wifi_event_group);

    // channel;
    uint8_t primary_channel;
    wifi_second_chan_t secondary_channel;

    ESP_ERROR_CHECK(esp_wifi_get_channel(&primary_channel, &secondary_channel));
    ESP_LOGI(TAG, "Primary channel %d", primary_channel);


    return err;
}