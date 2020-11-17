#include "bridge.h"

static const char *TAG = "BRIDGE_EVNT";

esp_err_t bridge_event_init() {
    ESP_LOGV(TAG, "bridge_event_init()");
    bridge_event_queue = xQueueCreate ( BRIDGE_EVENT_QUEUE_SIZE, sizeof(bridge_event_t) );
    if ( bridge_event_queue == NULL) {
        ESP_LOGE(TAG, "failed to create 'bridge_event_queue'");
        return ESP_FAIL;
    }

    xTaskCreate(bridge_event_task, "bridge_event_task", 2048, NULL, 4, NULL);

    return ESP_OK;
}

esp_err_t bridge_event_deinit() {
    ESP_LOGV(TAG, "bridge_event_deinit()");
    if ( bridge_event_queue != NULL ) {
        vSemaphoreDelete(bridge_event_queue);
    }
    return ESP_OK;
}

esp_err_t bridge_event_publish(bridge_event_t* evt) {
    ESP_LOGV(TAG, "bridge_event_publish");

    if ( bridge_event_queue != NULL ) {
        if (xQueueSend(bridge_event_queue, evt, portMAX_DELAY) != pdTRUE) {
            ESP_LOGE(TAG, "Event publish failed");
            return ESP_FAIL;
        }
    }
    else {
        ESP_LOGW(TAG, "Event queue is null, event ignored");
    }
    return ESP_OK;
}

static sensor_type_t bridge_espnow_recv_event_parse(bridge_espnow_recv_event_t* recv) {
    ESP_LOGV(TAG, "bridge_event_parse");


    if ( recv == NULL ) {
        ESP_LOGE(TAG, "BRIDGE_ESPNOW_RECV_EVENT to parse is null");
        return SENSOR_UNKNOWN;
    }

    //bridge_espnow_recv_event_t* recv = evt->info.espnow_recv;    

    if ( recv->data_len != sizeof(sensor_info_t) ) {
        ESP_LOGE(TAG, "BRIDGE_ESPNOW_RECV_EVENT bad event len: %d, excpected=%d", recv->data_len, sizeof(sensor_info_t));
        return SENSOR_UNKNOWN;
    }

    sensor_info_t* sensor = (sensor_info_t*)recv->data;

    if ( sensor->type != SENSOR_BME280 ) {
        ESP_LOGE(TAG, "sensor type %d not implemented yet", sensor->type );
        return SENSOR_NOT_IMPLEMENTED;
    }


    if ( sensor->magic != SENSOR_MAGIC_ID ) {
        ESP_LOGE(TAG, "bad sensor magic id: %d, expected %d", sensor->magic, SENSOR_MAGIC_ID );
        return SENSOR_BAD_DATA;
    }

    uint16_t crc16 = sensor->crc16;
    sensor->crc16 = 0;
    uint16_t comp_crc16 = esp_crc16_le(UINT16_MAX, (uint8_t*)sensor, sizeof(sensor_info_t));
    if ( comp_crc16 != crc16 ) {
        ESP_LOGE(TAG, "bad sensor crc16: %d, expected %d", crc16, comp_crc16);
        return SENSOR_BAD_DATA;
    }

    return SENSOR_BME280;
} 

void bridge_event_task(void *pvParameter) {
    ESP_LOGV(TAG, "bridge_event_task");

    bridge_event_t evt;

    while (xQueueReceive(bridge_event_queue, &evt, portMAX_DELAY) == pdTRUE) {
        //ESP_LOG_BUFFER_HEXDUMP(TAG, &evt, sizeof(bridge_event_t), ESP_LOG_INFO);
        ESP_LOGW(TAG, "New event %d", evt.id);
        switch(evt.id) {
            case BRIDGE_ESPNOW_RECV_EVENT: 
            {
                bridge_espnow_recv_event_t* recv = (bridge_espnow_recv_event_t*)&evt.info.espnow_recv;
                //ESP_LOGI(TAG, "New event:");
                //ESP_LOG_BUFFER_HEXDUMP(TAG, recv, sizeof(bridge_espnow_recv_event_t), ESP_LOG_INFO);

                int parse = bridge_espnow_recv_event_parse(recv);
                if ( parse != SENSOR_BME280 ) {
                    ESP_LOGE(TAG, "parse error %d, event ignored", parse);
                    ESP_LOG_BUFFER_HEXDUMP(TAG, recv->data, recv->data_len, ESP_LOG_WARN);
                }
                else {
                    // send data to mqtt
                    char topic[64];
                    char message[16];
                    char influx[128];

                    bridge_espnow_add_peer(recv->mac_addr);

                    sensor_info_t* sensor = (sensor_info_t*)recv->data;

                    sprintf(topic, BRIDGE_MQTT_TOPIC_TEMP, sensor->id);
                    sprintf(message, "%2.2f", sensor->data.bme280.temp);
                    sprintf(influx, BRIDGE_MQTT_INFLUX_DATA_FORMAT, "temperature", sensor->id, message);

                    //esp_mqtt_client_publish(bridge_mqtt_client, topic, message, 0, 1, 0);
                    ESP_LOGI(TAG, "publishing to topic:%s message:%s", topic, influx);
                    esp_mqtt_client_publish(bridge_mqtt_client, topic, influx, 0, 1, 0);

                    sprintf(topic, BRIDGE_MQTT_TOPIC_HUMI, sensor->id);
                    sprintf(message, "%2.2f", sensor->data.bme280.humi);
                    sprintf(influx, BRIDGE_MQTT_INFLUX_DATA_FORMAT, "humidity", sensor->id, message);

                    //esp_mqtt_client_publish(bridge_mqtt_client, topic, message, 0, 1, 0);
                    ESP_LOGI(TAG, "publishing to topic:%s message:%s", topic, influx);
                    esp_mqtt_client_publish(bridge_mqtt_client, topic, influx, 0, 1, 0);

                    sprintf(topic, BRIDGE_MQTT_TOPIC_PRES, sensor->id);
                    sprintf(message, "%4.2f", sensor->data.bme280.pres);
                    sprintf(influx, BRIDGE_MQTT_INFLUX_DATA_FORMAT, "pressure", sensor->id, message);

                    //esp_mqtt_client_publish(bridge_mqtt_client, topic, message, 0, 1, 0);
                    ESP_LOGI(TAG, "publishing to topic:%s message:%s", topic, influx);
                    esp_mqtt_client_publish(bridge_mqtt_client, topic, influx, 0, 1, 0);
                    //esp_mqtt_client_publish(bridge_mqtt_client, topic, message, 0, 1, 0);

                    
                    sprintf(topic, BRIDGE_MQTT_TOPIC_HEAP_MEM, sensor->id);
                    sprintf(message, "%u", esp_get_free_heap_size());
                    sprintf(influx, BRIDGE_MQTT_INFLUX_DATA_FORMAT, "heap_mem", sensor->id, message);
                    ESP_LOGI(TAG, "publishing to topic:%s message:%s", topic, influx);
                    esp_mqtt_client_publish(bridge_mqtt_client, topic, influx, 0, 1, 0);

                    ++measurementCount;
                }
                if ( recv != NULL && recv->data != NULL ) {
                    free(recv->data);
                }
                break;
            }
            default: 
                ESP_LOGI(TAG, "event %d not implemented", evt.id);
                break;
        }
    }
}