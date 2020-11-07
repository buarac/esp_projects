#ifndef _SENSORS_H_
#define _SENSORS_H__

#define SENSOR_MAGIC_ID 0x1389

typedef enum {
    SENSOR_BME280 = 0x01,
    SENSOR_BME680
} sensor_type_t;

typedef struct {
    float   temp;
    float   humi;
    float   pres;
} sensor_bme280_t;

typedef struct {
    float   temp;
    float   humi;
    float   pres;
} sensor_bme680_t;

typedef union {
    sensor_bme280_t bme280;
    sensor_bme680_t bme680;
} sensor_data_t;

// SENSORS
typedef struct {
    uint16_t        id;
    uint16_t        magic;
    uint16_t        crc16;
    sensor_type_t   type;
    sensor_data_t   data;
} sensor_info_t;

#endif // _SENSORS_H
