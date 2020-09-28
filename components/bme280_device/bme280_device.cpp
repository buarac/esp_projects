#include "esp_log.h"
#include "bme280_device.h"
#include <string.h>

static const char *TAG = "BME280";

BME280_Device::BME280_Device(uint8_t addr, i2c_port_t port, uint8_t sda, uint8_t scl, uint32_t freq)
    :I2C_Device(addr, port, sda, scl, freq) {

    ESP_LOGV(TAG, "Constructor");

    lastError = ESP_OK;
    regName = 0;
    regData = 0;
}

uint8_t BME280_Device::ReadReg(uint8_t reg) {
    regName = reg;
    lastError = Read(&regName, 1, &regData, 1);
    return regData;
}

void BME280_Device::WriteReg(uint8_t reg, uint8_t val) {
    regName = reg;
    regData = val;
    lastError = Write(&regName, 1, &regData, 1);
}

esp_err_t BME280_Device::GetLastError()  {
    return lastError;
}

bool BME280_Device::Init() {
    ESP_LOGV(TAG, "Init() default params");

    BME280_Params params = {
        .filter = BME280_FILTER_OFF,
        .mode = BME280_SLEEP_MODE,
        .over_samp_temp = BME280_OVERSAMPLING_1,
        .over_samp_humi = BME280_OVERSAMPLING_1,
        .over_samp_pres = BME280_OVERSAMPLING_1,
        .standby = BME280_STANDBY_1000
    };

    return Init(params);
}

bool BME280_Device::Init(BME280_Params params) {
    ESP_LOGV(TAG, "Init()");

    lastError = I2C_Device::Init();
    if ( lastError != ESP_OK ) return false;
    ESP_LOGD(TAG, "i2c device initialized");

    if ( !IsBME280() ) {
        ESP_LOGE(TAG, "not a bme280 device");
        return false;
    }
    ESP_LOGD(TAG, "valid bme280 device");

    SoftReset();
    WaitNVMCopied();
    ReadCalibData();

    // apply params
    BME280_Config   config;
    BME280_TempCtrl tempCtrl;
    BME280_HumiCtrl humiCtrl;

    config.bits.standby = params.standby;
    config.bits.filter = params.filter;
    humiCtrl.bits.over_samp_humi = params.over_samp_humi;
    tempCtrl.bits.over_samp_temp = params.over_samp_temp;
    tempCtrl.bits.over_samp_pres = params.over_samp_pres;
    tempCtrl.bits.mode = params.mode;

    WriteReg(BME280_REG_CONFIG, config.data);
    WriteReg(BME280_REG_CTRL_HUMI, humiCtrl.data);
    WriteReg(BME280_REG_CTRL_TEMP, tempCtrl.data);

    ESP_LOGD(TAG, "params applied");

    return ( lastError == ESP_OK );
}

bool BME280_Device::IsBME280() {
    uint8_t chipId = ReadReg(BME280_REG_CHIP_ID);
    if ( chipId != BME280_CHIP_ID) {
        ESP_LOGE(TAG, "Not a bme280 device, found chip id = %02x ( expected = %02x )", chipId, BME280_CHIP_ID);
        return false;
    }
    return true;
}

bool BME280_Device::SoftReset() {
    ESP_LOGV(TAG, "SoftReset");

    WriteReg(BME280_REG_RESET, BME280_RESET_VALUE);
    return ( lastError == ESP_OK );
}

void BME280_Device::WaitNVMCopied() {
    ESP_LOGV(TAG, "WaitNVMCopied");

    uint8_t nvm_copied = 0;
    BME280_Status status;

    while ( !nvm_copied ) {
        status.data = ReadReg(BME280_REG_STATUS);
        if ( status.bits.im_update == 0 ) {
            nvm_copied = 1;
        }
    }
    ESP_LOGD(TAG, "NVM data copied");
}

void BME280_Device::WaitMeasureDone() {
    ESP_LOGV(TAG, "WaitMeasureDone");

    uint8_t measure_done = 0;
    BME280_Status       status;
    BME280_TempCtrl     ctrl;

    while ( !measure_done ) {
        status.data = ReadReg(BME280_REG_STATUS);
        ctrl.data = ReadReg(BME280_REG_CTRL_TEMP);
        if ( ctrl.bits.mode == BME280_SLEEP_MODE && status.bits.measuring == 0 ) {
            measure_done = 1;
        }
    }
    ESP_LOGD(TAG, "measure is done");
}

void BME280_Device::ReadCalibData() {
    ESP_LOGV(TAG, "ReadCalibData");

    // read data
    uint8_t reg = 0x88;
    uint8_t val[24];

    lastError = Read(&reg, 1, val, 24);
    if ( lastError != ESP_OK ) {
        ESP_LOGE(TAG, "failed to read calib data");
        return;
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
    calib.dig_h1 = ReadReg(0xA1);
    reg = 0xe1;
    memset(val, 0, 7);
    lastError = Read(&reg, 1, val, 7);
    if ( lastError != ESP_OK ) {
        ESP_LOGE(TAG, "failed to read calib data");
        return;
    }
    calib.dig_h2 = (int16_t)(val[1] << 8 | val[0]);
    calib.dig_h3 = (uint8_t)(val[2]);
    calib.dig_h4 = (int16_t)(val[3] << 4 | (val[4] & 0x0f));
    calib.dig_h5 = (int16_t)(((val[4] & 0xf0) >> 4) | val[5] << 4 );
    calib.dig_h6 = (int8_t)(val[6]);

    ESP_LOGD(TAG, "calib data readed");
}


uint32_t BME280_Device::CompensateHumidity(int32_t humi, int32_t fine_temp) {
    ESP_LOGV(TAG, "CompensateHumidity");
    
    int32_t v_x1_u32r;
    
    v_x1_u32r = (fine_temp - ((int32_t)76800));

    v_x1_u32r = (((((humi << 14) - (((int32_t)calib.dig_h4) << 20) - (((int32_t)calib.dig_h5) * v_x1_u32r)) + ((int32_t)16384)) >> 15) * (((((((v_x1_u32r * ((int32_t)calib.dig_h6)) >> 10) * (((v_x1_u32r * ((int32_t)calib.dig_h3)) >> 11) + ((int32_t)32768))) >> 10) + ((int32_t)2097152)) * ((int32_t)calib.dig_h2) + 8192) >> 14));
    v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) * ((int32_t)calib.dig_h1)) >> 4));
    v_x1_u32r = (v_x1_u32r < 0 ? 0 : v_x1_u32r);
    v_x1_u32r = (v_x1_u32r > 419430400 ? 419430400 : v_x1_u32r);
    v_x1_u32r = ( v_x1_u32r >> 12 );

    ESP_LOGD(TAG, "compensate humidity output");
    ESP_LOGD(TAG, "fine_temp = %d", fine_temp);
    ESP_LOGD(TAG, "raw humi  = %d", humi);
    ESP_LOGD(TAG, "comp humi = %d", v_x1_u32r);

    return v_x1_u32r;
}

uint32_t BME280_Device::CompensatePressure(int32_t pres, int32_t fine_temp) {
    ESP_LOGV(TAG, "CompensatePressure");

    int64_t var1, var2, p;

    var1 = ((int64_t)fine_temp) - 128000;
    var2 = var1 * var1 * (int64_t)calib.dig_p6;
    var2 = var2 + ((var1*(int64_t)calib.dig_p5)<<17);
    var2 = var2 + (((int64_t)calib.dig_p4)<<35);
    var1 = ((var1 * var1 * (int64_t)calib.dig_p3)>>8);
    var1 = (((((int64_t)1)<<47)+var1))*((int64_t)calib.dig_p1)>>33; 
    if (var1 == 0) {
        p = 0;
    }
    else {
        p = 1048576-pres;
        p = (((p<<31)-var2)*3125)/var1;
        var1 = (((int64_t)calib.dig_p9) * (p>>13) * (p>>13)) >> 25; 
        var2 = (((int64_t)calib.dig_p8) * p) >> 19;
        p = ((p + var1 + var2) >> 8) + (((int64_t)calib.dig_p7)<<4);
    }

    ESP_LOGD(TAG, "compensate pressure output");
    ESP_LOGD(TAG, "fine_temp = %d", fine_temp);
    ESP_LOGD(TAG, "raw pres  = %d", pres);
    ESP_LOGD(TAG, "comp pres = %llu", p);

    return p;
}

int32_t BME280_Device::CompensateTemperature(int32_t temp, int32_t *fine_temp) {
    ESP_LOGV(TAG, "CompensateTemperature");
    int32_t var1, var2;
    int32_t comp_temp;

    var1 = ((temp >> 3) - (calib.dig_t1 << 1));
    var1 *= (calib.dig_t2 >> 11);

    var2 = ((temp >> 4) - calib.dig_t1);
    var2 = (var2 * var2) >> 12;
    var2*= (calib.dig_t3 >> 14);

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

BME280_RawData BME280_Device::ReadRawData() {
    ESP_LOGV(TAG, "ReadRawData");

    BME280_RawData rawData = { 0, 0, 0};

    uint8_t reg = BME280_REG_RAW_DATA;
    uint8_t val[8];

    lastError = Read(&reg, 1, val, 8);
    if ( lastError != ESP_OK ) {
        ESP_LOGE(TAG, "failed to read raw data");
        return rawData;
    }

    rawData.pres = (uint32_t)(val[0] << 12 | val[1] << 4 | val[2] >> 4);
    rawData.temp = (uint32_t)(val[3] << 12 | val[4] << 4 | val[5] >> 4);
    rawData.humi = (uint32_t)(val[6] << 8 | val[7]);

    ESP_LOGD(TAG, "raw data output");
    ESP_LOGD(TAG, "raw temp = %d, %04x", rawData.temp, rawData.temp);
    ESP_LOGD(TAG, "raw pres = %d, %04x", rawData.pres, rawData.pres);
    ESP_LOGD(TAG, "raw humi = %d, %04x", rawData.humi, rawData.humi);

    return rawData;
}

void BME280_Device::StartForcedMeasure() {
    ESP_LOGV(TAG, "StartForcedMeasure");

    BME280_TempCtrl ctrl;

    ctrl.data = ReadReg(BME280_REG_CTRL_TEMP);
    ctrl.bits.mode = BME280_FORCED_MODE;
    WriteReg(BME280_REG_CTRL_TEMP, ctrl.data);
}

BME280_Data BME280_Device::ReadDataForced() {
    ESP_LOGV(TAG, "ReadDataForced");

    int32_t fine_temp;
    int32_t comp_temp;
    uint32_t comp_humi;
    uint32_t comp_pres;

    StartForcedMeasure();
    WaitMeasureDone();
    BME280_RawData rawData = ReadRawData();

    BME280_Data data;

    comp_temp = CompensateTemperature(rawData.temp, &fine_temp);
    comp_humi = CompensateHumidity(rawData.humi, fine_temp);
    comp_pres = CompensatePressure(rawData.pres, fine_temp);
    data.temperature = (float)comp_temp / 100.0f;
    data.humidity = (float)comp_humi / 1024.0f;
    data.pressure = (float)comp_pres / 256.0f / 100.0f;

    return data;
}