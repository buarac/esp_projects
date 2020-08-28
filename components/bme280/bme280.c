#include "esp_log.h"
#include "bme280.h"
#include <string.h>

static const char *TAG = "bme280";

static inline esp_err_t bme280_wait_measure_done(bme280_t* bme) {
    ESP_LOGV(TAG, "waiting for measure done");

    uint8_t measure_done = 0;
    bme280_status_t     status;
    bme280_ctrl_temp_t  ctrl;

    while ( !measure_done ) {

        status.data = bme280_readreg(bme, BME280_REG_STATUS);
        ctrl.data = bme280_readreg(bme, BME280_REG_CTRL_TEMP);
        if ( ctrl.bits.mode == BME280_SLEEP_MODE && status.bits.measuring == 0 ) {
            measure_done = 1;
        }
    }
    ESP_LOGD(TAG, "measure done");
    return ESP_OK;
}

static inline esp_err_t bme280_wait_nvm_copied(bme280_t* bme) {
    ESP_LOGV(TAG, "waiting for nvm data copied");

    uint8_t nvm_copied = 0;
    bme280_status_t status;

    while ( !nvm_copied ) {
        status.data = bme280_readreg(bme, BME280_REG_STATUS);
        if ( status.bits.im_update == 0 ) {
            nvm_copied = 1;
        }
    }
    ESP_LOGD(TAG, "nvm data copied");
    return ESP_OK;
}

inline void bme280_read_status(bme280_t* bme, bme280_status_t* status) {
    status->data = bme280_readreg(bme, BME280_REG_STATUS);
}

uint8_t bme280_readreg(bme280_t* bme, uint8_t reg) {
    ESP_LOGV(TAG, "bme280_readreg(%02x)", reg);

    uint8_t in_data = reg;
    uint8_t out_data = 0;

    if ( i2cdev_read(&bme->i2cdev, &in_data, 1, &out_data, 1) != ESP_OK ) {
        ESP_LOGE(TAG, "failed to read bme280's reg");
        return 0;
    }
    return out_data;
}

void bme280_writereg(bme280_t* bme, uint8_t reg, uint8_t data) {
    ESP_LOGV(TAG, "bme280_writereg(%02x)->%02x", reg, data);

    uint8_t in_data = reg;
    uint8_t out_data= data;

    if ( i2cdev_write(&bme->i2cdev, &in_data, 1, &out_data, 1) != ESP_OK ) {
        ESP_LOGE(TAG, "failed to write bme280's reg");
    }
}

inline uint8_t bme280_read_chip_id(bme280_t* bme) {
    ESP_LOGV(TAG, "bme280_read_chip_id");

    return bme280_readreg(bme, BME280_REG_CHIP_ID);
}

inline void bme280_soft_reset(bme280_t *bme) {
    ESP_LOGV(TAG, "bme280_soft_reset");

    bme280_writereg(bme, BME280_REG_RESET, BME280_RESET_VALUE);
}

bme280_t* bme280_setup(i2cdev_params_t* i2c_params, bme280_params_t*bme280_params) {
    ESP_LOGV(TAG, "bme280_setup");

    bme280_t* bme = NULL;

    bme = malloc(sizeof(bme280_t));
    if ( bme == NULL ) {
        ESP_LOGE(TAG, "failed to malloc bme object");
        return NULL;
    }

    memcpy(&bme->params, bme280_params, sizeof(bme280_params_t));
    memcpy(&bme->i2cdev, i2c_params, sizeof(i2cdev_params_t));

    return bme;
}

void bme280_done(bme280_t* bme) {
    ESP_LOGV(TAG, "bme280_done");

    if ( bme == NULL ) {
        return;
    }
    free((void*)bme);
}

esp_err_t bme280_init_device(bme280_t *bme) {
    ESP_LOGV(TAG, "bme280_init_device");

    esp_err_t ret = ESP_OK;

    if (bme == NULL)
    {
        ESP_LOGE(TAG, "bme280 object is null");
        return ESP_FAIL;
    }

    ret = i2cdev_setup(&bme->i2cdev);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "i2c device init failed");
        return ret;
    }


    uint8_t chip_id = bme280_read_chip_id(bme);
    if ( chip_id != BME280_CHIP_ID ) {
        ESP_LOGE(TAG, "Not a bme280 device, chid id = %02x, expected = %02x", chip_id, BME280_CHIP_ID);
        return ESP_FAIL;
    }

    bme280_soft_reset(bme);
    bme280_wait_nvm_copied(bme);
    return bme280_read_calib_data(bme);
}


esp_err_t bme280_init_params(bme280_t *bme, bme280_params_t *params)
{
    ESP_LOGV(TAG, "bme280_init_params");

    if (bme == NULL || params == NULL)
    {
        ESP_LOGE(TAG, "bme or params null");
        return ESP_ERR_INVALID_ARG;
    }

    bme280_config_t config;
    bme280_ctrl_temp_t ctrl_temp;
    bme280_ctrl_humi_t ctrl_humi;
    u_int8_t reg;
    esp_err_t ret;

    config.bits.standby = params->standby;
    config.bits.filter = params->filter;
    ctrl_humi.bits.over_samp_humi = params->over_samp_humi;
    ctrl_temp.bits.over_samp_temp = params->over_samp_temp;
    ctrl_temp.bits.over_samp_pres = params->over_samp_pres;
    ctrl_temp.bits.mode = params->mode;

    // config
    reg = BME280_REG_CONFIG;
    ret = i2c_device_write(&bme->device, &reg, 1, &config.data, 1);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "failed to write (%d) to config register", config.data);
        return ret;
    }
    // ctrl humi
    reg = BME280_REG_CTRL_HUMI;
    ret = i2c_device_write(&bme->device, &reg, 1, &ctrl_humi.data, 1);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "failed to write (%d) to ctrl humi register", ctrl_humi.data);
        return ret;
    }

    // ctrl temp
    reg = BME280_REG_CTRL_TEMP;
    ret = i2c_device_write(&bme->device, &reg, 1, &ctrl_temp.data, 1);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "failed to write (%d) to ctrl temp register", ctrl_temp.data);
        return ret;
    }

    return ret;
}


esp_err_t bme280_read_calib_data(bme280_t *bme)
{
    ESP_LOGV(TAG, "bme280_read_calib_data");

    esp_err_t ret = ESP_OK;
    i2c_device_t *dev = &bme->device;
    bme280_calib_data_t calib;
    // read data
    uint8_t reg = 0x88;
    uint8_t val[24];

    ret = i2c_device_read(dev, &reg, 1, val, 24);
    if ( ret != ESP_OK ) {
        ESP_LOGE(TAG, "failed to read calib data");
        return ret;
    }
    // temperature & pressure
    calib.dig_t1 = (uint16_t)(val[1] << 8 | val[0]);
    calib.dig_t2 = (int16_t)(val[3] << 8 | val[2]);
    calib.dig_t3 = (int16_t)(val[5] << 8 | val[4]);
    calib.dig_p1 = (uint16_t)(val[7] << 8 | val[6]);
    calib.dig_p2 = (int16_t)(val[9] << 8 | val[8]);
    calib.dig_p3 = (int16_t)(val[11] << 8 | val[10]);
    calib.dig_p4 = (int16_t)(val[13] << 8 | val[12]);
    calib.dig_p5 = (int16_t)(val[15] << 8 | val[14]);
    calib.dig_p6 = (int16_t)(val[17] << 8 | val[16]);
    calib.dig_p7 = (int16_t)(val[19] << 8 | val[18]);
    calib.dig_p8 = (int16_t)(val[21] << 8 | val[20]);
    calib.dig_p9 = (int16_t)(val[23] << 8 | val[22]);
    // humidity
    ret = i2c_device_read_reg_uint8(dev, 0xa1, &calib.dig_h1);
    if ( ret != ESP_OK ) {
        ESP_LOGE(TAG, "failed to read calib data");
        return ret;
    }
    reg = 0xe1;
    memset(val, 0, 7);
    ret = i2c_device_read(dev, &reg, 1, val, 7);
    if ( ret != ESP_OK ) {
        ESP_LOGE(TAG, "failed to read calib data");
        return ret;
    }
    calib.dig_h2 = (int16_t)(val[1] << 8 | val[0]);
    calib.dig_h3 = (uint8_t)(val[2]);
    calib.dig_h4 = (int16_t)(val[3] << 4 | (val[4] & 0x0f));
    calib.dig_h5 = (int16_t)(((val[4] & 0xf0) >> 4) | val[5] << 4 );
    calib.dig_h6 = (int8_t)(val[6]);

    memcpy(&bme->calib, &calib, sizeof(bme280_calib_data_t));

    return ret;
}

static inline uint32_t bme280_compensate_humidity(bme280_t* bme, int32_t humi, int32_t fine_temp) {
    ESP_LOGV(TAG, "bme280_compensate_humidity");
    
    int32_t v_x1_u32r;
    
    v_x1_u32r = (fine_temp - ((int32_t)76800));

    v_x1_u32r = (((((humi << 14) - (((int32_t)bme->calib.dig_h4) << 20) - (((int32_t)bme->calib.dig_h5) * v_x1_u32r)) + ((int32_t)16384)) >> 15) * (((((((v_x1_u32r * ((int32_t)bme->calib.dig_h6)) >> 10) * (((v_x1_u32r * ((int32_t)bme->calib.dig_h3)) >> 11) + ((int32_t)32768))) >> 10) + ((int32_t)2097152)) * ((int32_t)bme->calib.dig_h2) + 8192) >> 14));
    v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) * ((int32_t)bme->calib.dig_h1)) >> 4));
    v_x1_u32r = (v_x1_u32r < 0 ? 0 : v_x1_u32r);
    v_x1_u32r = (v_x1_u32r > 419430400 ? 419430400 : v_x1_u32r);
    v_x1_u32r = ( v_x1_u32r >> 12 );

    ESP_LOGD(TAG, "compensate humidity output");
    ESP_LOGD(TAG, "fine_temp = %d", fine_temp);
    ESP_LOGD(TAG, "raw humi  = %d", humi);
    ESP_LOGD(TAG, "comp humi = %d", v_x1_u32r);

    return v_x1_u32r;

}

static inline uint32_t bme280_compensate_pressure(bme280_t* bme, int32_t pres, int32_t fine_temp) {
    ESP_LOGV(TAG, "bme280_compensate_pressure");

    int64_t var1, var2, p;

    var1 = ((int64_t)fine_temp) - 128000;
    var2 = var1 * var1 * (int64_t)bme->calib.dig_p6;
    var2 = var2 + ((var1*(int64_t)bme->calib.dig_p5)<<17);
    var2 = var2 + (((int64_t)bme->calib.dig_p4)<<35);
    var1 = ((var1 * var1 * (int64_t)bme->calib.dig_p3)>>8);
    var1 = (((((int64_t)1)<<47)+var1))*((int64_t)bme->calib.dig_p1)>>33; 
    if (var1 == 0) {
        p = 0;
    }
    else {
        p = 1048576-pres;
        p = (((p<<31)-var2)*3125)/var1;
        var1 = (((int64_t)bme->calib.dig_p9) * (p>>13) * (p>>13)) >> 25; 
        var2 = (((int64_t)bme->calib.dig_p8) * p) >> 19;
        p = ((p + var1 + var2) >> 8) + (((int64_t)bme->calib.dig_p7)<<4);
    }

    ESP_LOGD(TAG, "compensate pressure output");
    ESP_LOGD(TAG, "fine_temp = %d", fine_temp);
    ESP_LOGD(TAG, "raw pres  = %d", pres);
    ESP_LOGD(TAG, "comp pres = %llu", p);

    return p;
}

static inline int32_t bme280_compensate_temperature(bme280_t *bme, int32_t temp, int32_t *fine_temp)
{
    ESP_LOGV(TAG, "bme280_compensate_temperature");
    int32_t var1, var2;
    int32_t comp_temp;

    var1 = ((temp >> 3) - (bme->calib.dig_t1 << 1));
    var1 *= (bme->calib.dig_t2 >> 11);

    var2 = ((temp >> 4) - bme->calib.dig_t1);
    var2 = (var2 * var2) >> 12;
    var2*= (bme->calib.dig_t3 >> 14);

    *fine_temp = var1 + var2;
    comp_temp = (*fine_temp * 5 + 128) >> 8;

    ESP_LOGD(TAG, "compensate temperature output");
    ESP_LOGD(TAG, "var1 = %d", var1);
    ESP_LOGD(TAG, "var2 = %d", var2);
    ESP_LOGD(TAG, "fine_temp = %d", *fine_temp);
    ESP_LOGD(TAG, "raw temp  = %d", temp);
    ESP_LOGD(TAG, "comp_temp = %d", comp_temp);


    return comp_temp;
}

esp_err_t bme280_read_forced(bme280_t *bme, bme280_measure_t *measure)
{
    ESP_LOGV(TAG, "bme280_read_forced");

    esp_err_t ret = ESP_OK;
    bme280_raw_data_t raw_data;
    bme280_measure_t m;
    int32_t fine_temp;
    int32_t comp_temp;
    uint32_t comp_humi;
    uint32_t comp_pres;

    ret = bme280_read_raw_forced(bme, &raw_data);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "failed to read raw data");
        return ret;
    }

    comp_temp = bme280_compensate_temperature(bme, raw_data.temp, &fine_temp);
    comp_humi = bme280_compensate_humidity(bme, raw_data.humi, fine_temp);
    comp_pres = bme280_compensate_pressure(bme, raw_data.pres, fine_temp);
    m.temp = (float)comp_temp / 100.0f;
    m.humi = (float)comp_humi / 1024.0f;
    m.pres = (float)comp_pres / 256.0f / 100.0f;

    memset(measure, 0, sizeof(bme280_measure_t));
    memcpy(measure, &m, sizeof(bme280_measure_t));

    return ret;
}

esp_err_t bme280_read_raw_forced(bme280_t *bme, bme280_raw_data_t *raw_data)
{
    ESP_LOGV(TAG, "bme280_read_raw_forced");

    bme280_ctrl_temp_t ctrl;
    uint8_t reg;
    esp_err_t ret = ESP_OK;

    reg = BME280_REG_CTRL_TEMP;
    ret = i2c_device_read(&bme->device, &reg, 1, &ctrl.data, 1);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "failed to read ctrl temp reg");
        return ret;
    }
    ctrl.bits.mode = BME280_FORCED_MODE;
    ret = i2c_device_write(&bme->device, &reg, 1, &ctrl.data, 1);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "failed to write (%d) to ctrl temp reg", ctrl.data);
        return ret;
    }

    ret = bme280_wait_measure_done (bme);

    return bme280_read_raw(bme, raw_data);
}

esp_err_t bme280_read_raw(bme280_t *bme, bme280_raw_data_t *raw_data)
{
    ESP_LOGV(TAG, "bme280_read_raw");

    esp_err_t ret = ESP_OK;

    uint8_t reg = BME280_REG_RAW_DATA;
    uint8_t val[8];

    ret = i2c_device_read(&bme->device, &reg, 1, val, 8);
    if (ret == ESP_OK)
    {
        memset(raw_data, 0, sizeof(bme280_raw_data_t));
        raw_data->pres = (uint32_t)(val[0] << 12 | val[1] << 4 | val[2] >> 4);
        raw_data->temp = (uint32_t)(val[3] << 12 | val[4] << 4 | val[5] >> 4);
        raw_data->humi = (uint32_t)(val[6] << 8 | val[7]);

        ESP_LOGD(TAG, "raw data output");
        ESP_LOGD(TAG, "raw temp = %d, %04x", raw_data->temp, raw_data->temp);
        ESP_LOGD(TAG, "raw pres = %d, %04x", raw_data->pres, raw_data->pres);
        ESP_LOGD(TAG, "raw humi = %d, %04x", raw_data->humi, raw_data->humi);
    }
    return ret;
}
