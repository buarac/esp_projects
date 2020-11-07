#ifndef _BRIDGE_H_
#define _BRIDGE_H_

#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <assert.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"
#include "freertos/event_groups.h"
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

esp_mqtt_client_handle_t bridge_mqtt_client;

esp_err_t bridge_mqtt_app_start(void);

// ESPNOW

esp_err_t bridge_espnow_init(void);
esp_err_t bridge_espnow_send_test(void);




#endif // _BRIDGE_H_