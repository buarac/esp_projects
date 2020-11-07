/* Deep sleep wake up example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_sleep.h"
#include "esp_log.h"
#include "esp32/ulp.h"
#include "driver/touch_pad.h"
#include "driver/adc.h"
#include "driver/rtc_io.h"
#include "soc/sens_periph.h"
#include "soc/rtc.h"

#define LED_PIN 2
#define LED_ON  1
#define LED_OFF 0
#define GPIO_OUTPUT_PIN_SEL  (1ULL<<LED_PIN)

#define WAKE_UP_SEC 30

static const char *TAG = "DEEP_SLEEP";

static RTC_DATA_ATTR struct timeval sleep_enter_time;

void led_init() {
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);
}

void led(uint8_t state) {
    gpio_set_level(LED_PIN, state);
}


void app_main(void)
{
    led_init();
    led(LED_ON);

    struct timeval now;
    gettimeofday(&now, NULL);
    int sleep_time_ms = (now.tv_sec - sleep_enter_time.tv_sec) * 1000 + (now.tv_usec - sleep_enter_time.tv_usec) / 1000;

    switch (esp_sleep_get_wakeup_cause()) {
        case ESP_SLEEP_WAKEUP_TIMER: {
            ESP_LOGI(TAG, "Wake up from timer. Time spent in deep sleep: %dms", sleep_time_ms);
            break;
        }
        case ESP_SLEEP_WAKEUP_UNDEFINED:
        default:
            ESP_LOGI(TAG, "Not a deep sleep reset");
    }

    ESP_LOGI(TAG, "Enabling timer wakeup, %ds", WAKE_UP_SEC);
    esp_sleep_enable_timer_wakeup(WAKE_UP_SEC * 1000000);


    ESP_LOGI(TAG, "Entering deep sleep");
    gettimeofday(&sleep_enter_time, NULL);

    led(LED_OFF);
    esp_deep_sleep_start();
}



