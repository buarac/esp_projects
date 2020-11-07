/* ESPNOW Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#ifndef ESPNOW_CAPTEUR_H
#define ESPNOW_CAPTEUR_H

/* ESPNOW can work in both station and softap mode. It is configured in menuconfig. */

//#define ESPNOW_WIFI_MODE WIFI_MODE_AP
#define ESPNOW_WIFI_MODE WIFI_MODE_STA
//#define ESPNOW_WIFI_MODE WIFI_MODE_STA
#define ESPNOW_WIFI_IF   ESP_IF_WIFI_STA

typedef struct {
    float   temp;
    float   humi;
    float   pres;
} bme280_data_t;

typedef struct {
    uint16_t        id;
    uint16_t        crc;
    bme280_data_t   data;
} capteur_data_t;


#endif // ESPNOW_CAPTEUR_H
