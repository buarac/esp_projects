#ifndef _BRIDGE_H_
#define _BRIDGE_H_

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <assert.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"
#include "freertos/event_groups.h"
#include "esp_heap_task_info.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_now.h"
#include "esp_crc.h"
#include "mqtt_client.h"

#include "sensors.h"

// 

typedef void* bridge_handle_t;
typedef void* sensor_handler_t;
typedef void* i2c_bus_handler_t;




// WIFI
#define BRIDGE_WIFI_CONNECTED_BIT   BIT0
#define BRIDGE_WIFI_FAIL_BIT        BIT1
#define BRIDGE_WIFI_MAXIMUM_RETRY   10
#define BRIDGE_WIFI_MODE            WIFI_MODE_APSTA
#define BRIDGE_WIFI_IF              ESP_IF_WIFI_STA
#define BRIDGE_WIFI_CHANNEL         11
#define BRIDGE_WIFI_SSID            "Buar_2.4G"
#define BRIDGE_WIFI_PASSWORD        "304460300055447"


esp_err_t bridge_wifi_init(void); 

// MQTT
#define BRIDGE_MQTT_URI "mqtt://raspberrypi.local"
#define BRIDGE_MQTT_TOPIC_TEMP "/sensors/%d/temperature"
#define BRIDGE_MQTT_TOPIC_HUMI "/sensors/%d/humidity"
#define BRIDGE_MQTT_TOPIC_PRES "/sensors/%d/pressure"
#define BRIDGE_MQTT_TOPIC_HEAP_MEM "/sensors/%d/heap_mem"

#define BRIDGE_MQTT_INFLUX_DATA_FORMAT "%s,device_id=%d value=%s"

esp_mqtt_client_handle_t bridge_mqtt_client;

esp_err_t bridge_mqtt_app_start(void);

// ESPNOW
#define BRIDGE_ESPNOW_ACK_RECEIVED_BIT   BIT0
esp_err_t bridge_espnow_init(void);
esp_err_t bridge_espnow_send_test(void);
esp_err_t bridge_espnow_add_peer(const uint8_t* mac_addr);


// BRIDGE
uint32_t measurementCount;
xQueueHandle bridge_event_queue;
#define BRIDGE_EVENT_QUEUE_SIZE   50

typedef enum {
    BRIDGE_UNKNOWN_EVENT,
    BRIDGE_ESPNOW_SEND_EVENT,
    BRIDGE_ESPNOW_RECV_EVENT
} bridge_event_type_id_t;


typedef struct {
    uint8_t mac_addr[ESP_NOW_ETH_ALEN];
    esp_now_send_status_t status;
} bridge_espnow_send_event_t;

typedef struct {
    uint8_t     mac_addr[ESP_NOW_ETH_ALEN];
    uint8_t     *data;
    uint16_t    data_len;
} bridge_espnow_recv_event_t;

typedef union {
    bridge_espnow_send_event_t  espnow_send;
    bridge_espnow_recv_event_t  espnow_recv;
} bridge_event_info_t;

typedef struct {
    bridge_event_type_id_t      id;
    bridge_event_info_t         info;
} bridge_event_t;



esp_err_t bridge_event_init();
esp_err_t bridge_event_deinit();
esp_err_t bridge_event_publish(bridge_event_t* evt);
void bridge_event_task(void *pvParameter);

// TASK(s)
#define BRIDGE_MAX_TASK_NUM 20      
#define BRIDGE_MAX_BLOCK_NUM 20 

void bridge_dump_per_task_heap_info(void);

#endif // _BRIDGE_H_