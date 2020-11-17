#include "bridge.h"

static const char *TAG = "BRIDGE_MQTT";

static esp_err_t bridge_mqtt_event_handler_cb(esp_mqtt_event_handle_t event)
{
    // your_context_t *context = event->context;
    switch (event->event_id) {        
        case MQTT_EVENT_BEFORE_CONNECT:
            ESP_LOGD(TAG, "MQTT_EVENT_BEFORE_CONNECT");
            break;
        case MQTT_EVENT_CONNECTED:
            ESP_LOGD(TAG, "MQTT_EVENT_CONNECTED");
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGD(TAG, "MQTT_EVENT_DISCONNECTED");
            break;
        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGD(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGD(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGD(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGD(TAG, "MQTT_EVENT_DATA");
            ESP_LOGD(TAG, "TOPIC=%.*s", event->topic_len, event->topic);
            ESP_LOGD(TAG, "DATA=%.*s", event->data_len, event->data);
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGD(TAG, "MQTT_EVENT_ERROR");
            break;
        default:
            ESP_LOGD(TAG, "Other event id:%d", event->event_id);
            break;
    }
    return ESP_OK;
}

static void bridge_mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    ESP_LOGV(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    bridge_mqtt_event_handler_cb(event_data);
}

esp_err_t bridge_mqtt_app_start(void) {
    ESP_LOGV(TAG, "bridge_mqtt_app_start");

    esp_mqtt_client_config_t bridge_mqtt_cfg = {
        .uri = BRIDGE_MQTT_URI,
    };
    bridge_mqtt_client = esp_mqtt_client_init(&bridge_mqtt_cfg);
    esp_mqtt_client_register_event(bridge_mqtt_client, ESP_EVENT_ANY_ID, bridge_mqtt_event_handler, bridge_mqtt_client);
    esp_mqtt_client_start(bridge_mqtt_client);    
    return ESP_OK;
}