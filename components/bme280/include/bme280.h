#ifndef _BME280_DEVICE_H_
#define _BME280_DEVICE_H_

#include "i2cdev.h"



// BME280 registers
#define BME280_REG_CHIP_ID      0xd0
#define BME280_REG_RESET        0xe0
#define BME280_REG_CONFIG       0xf5
#define BME280_REG_STATUS       0xf3
#define BME280_REG_CTRL_TEMP    0xf4
#define BME280_REG_CTRL_HUMI    0xf2
#define BME280_REG_PRES_MSB     0xf7
#define BME280_REG_PRES_LSB     0xf8
#define BME280_REG_PRES_XLSB    0xf9
#define BME280_REG_TEMP_MSB     0xfa
#define BME280_REG_TEMP_LSB     0xfb
#define BME280_REG_TEMP_XLSB    0xfc
#define BME280_REG_HUM_MSB      0xfd
#define BME280_REG_HUM_LSB      0xfe

#define BME280_REG_RAW_DATA BME280_REG_PRES_MSB

// BME 280 CHID ID
#define BME280_CHIP_ID  0x60
// BME 280 SOFT RESET WORD
#define BME280_RESET_VALUE 0xb6

typedef enum {
    BME280_STANDBY_0_5 = 0,
    BME280_STANDBY_62_5,
    BME280_STANDBY_125,
    BME280_STANDBY_250,
    BME280_STANDBY_500,
    BME280_STANDBY_1000,
    BME280_STANDBY_10,
    BME280_STANDBY_20,
} bme280_standby_t;

typedef enum {
    BME280_OVERSAMPLING_SKIPPED = 0,
    BME280_OVERSAMPLING_1,
    BME280_OVERSAMPLING_2,
    BME280_OVERSAMPLING_4,
    BME280_OVERSAMPLING_16
} bme280_oversampling_t;

typedef enum {
    BME280_SLEEP_MODE = 0,
    BME280_FORCED_MODE = 1,
    BME280_NORMAL_MODE = 3
} bme280_mode_t;

typedef enum {
    BME280_FILTER_OFF = 0,
    BME280_FILTER_2,
    BME280_FILTER_4,
    BME280_FILTER_8,
    BME280_FILTER_16
} bme280_filter_t;

typedef struct {
    bme280_filter_t         filter;
    bme280_mode_t           mode;
    bme280_oversampling_t   over_samp_temp;
    bme280_oversampling_t   over_samp_humi;
    bme280_oversampling_t   over_samp_pres;
    bme280_standby_t        standby;
} bme280_params_t;

typedef union {
    struct {
        uint8_t dummy:2;
        uint8_t filter:3;
        uint8_t standby:3;
    } bits;
    uint8_t data;
} bme280_config_t;

typedef union {
    struct {
        uint8_t     mode:2;
        uint8_t     over_samp_pres:3;
        uint8_t     over_samp_temp:3;
    } bits;
    uint8_t data;
} bme280_ctrl_temp_t;

typedef union {
    struct {
        uint8_t     over_samp_humi:3;
        uint8_t     dummy:5;
    } bits;
    uint8_t data;
} bme280_ctrl_humi_t;

typedef union {
    struct {
        uint8_t im_update:1;
        uint8_t dummy:2;
        uint8_t measuring:1;
        uint8_t dummy2:4;
    } bits;
    uint8_t data;
} bme280_status_t;

typedef struct {
    uint32_t    temp;
    uint32_t    pres;
    uint32_t    humi;
} bme280_raw_data_t;

typedef struct {
 //   uint32_t id;
    float   temp;
    float   humi;
    float   pres;
} bme280_measure_t;

typedef struct {
    // donnees de calibrage
    uint16_t        dig_t1;
    int16_t         dig_t2;
    int16_t         dig_t3;
    uint16_t        dig_p1;
    int16_t         dig_p2;
    int16_t         dig_p3;
    int16_t         dig_p4;
    int16_t         dig_p5;
    int16_t         dig_p6;
    int16_t         dig_p7;
    int16_t         dig_p8;
    int16_t         dig_p9;
    // donnees de calibrage humidite
    uint8_t         dig_h1;
    int16_t         dig_h2;
    uint8_t         dig_h3;
    int16_t         dig_h4;
    int16_t         dig_h5;
    int8_t          dig_h6;
} bme280_calib_data_t;

typedef struct {
    i2cdev_params_t     i2cdev;
    bme280_calib_data_t calib;
    bme280_params_t     params;
} bme280_t;


///////////////////////////////
typedef struct {
    i2c_device_t*       i2c;
    bme280_calib_data_t cailb;
    bme280_config_t     config;
    bme280_status_t     status;
    bme280_ctrl_temp_t  ctrl_temp;
    bme280_ctrl_humi_t  ctrl_humi;
    uint8_t             chip_id;
} bme280_device_t;


bme280_device_t* bme280_create(uint8_t addr, i2c_port_t port, uint8_t sda, uint8_t scl, uint32_t freq);
esp_err_t bme280_init(bme280_device_t* bme);
void    bme280_deinit(bme280_device_t* bme);
uint8_t bme280_rd_reg(bme280_device_t* bme, uint8_t reg);
void    bme280_wr_reg(bme280_device_t* bme, uint8_t reg, uint8_t regval);

void bme280_get_status(bme280_device_t* bme);
void bme280_get_config(bme280_device_t* bme);
void bme280_get_ctrl_temp(bme280_device_t* bme);
void bme280_get_ctrl_humi(bme280_device_t* bme);
void bme280_soft_reset(bme280_device_t* bme);
boolean bme280_check_chid_id(bme280_device_t* bme);

///////////////////////////////

uint8_t bme280_readreg(bme280_t* bme, uint8_t reg);
void    bme280_writereg(bme280_t* bme, uint8_t reg, uint8_t data);

bme280_t* bme280_setup(i2cdev_params_t* i2c_params, bme280_params_t* bme280_params);
void      bme280_done(bme280_t* bme);

esp_err_t bme280_init_device(bme280_t* bme);

uint8_t bme280_read_chip_id(bme280_t* bme);  
void bme280_soft_reset(bme280_t* bme);
void bme280_read_status(bme280_t* bme, bme280_status_t* status);

esp_err_t bme280_read_calib_data(bme280_t* bme);


esp_err_t bme280_read_raw(bme280_t* bme, bme280_raw_data_t* raw_data);
esp_err_t bme280_read_raw_forced(bme280_t* bme, bme280_raw_data_t* raw_data);
esp_err_t bme280_read_forced(bme280_t* bme, bme280_measure_t* measure);

//int32_t bme280_compensate_temperature(bme280_t *bme, int32_t temp, int32_t *fine_temp);

#endif // _BME280_DEVICE_H_