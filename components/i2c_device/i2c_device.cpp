#include "esp_log.h"
#include "i2c_device.h"
#include <string.h>

static const char* TAG = "I2C";

I2C_Device::I2C_Device(uint8_t addr, i2c_port_t port, uint8_t sda, uint8_t scl, uint32_t freq) {
    ESP_LOGV(TAG, "Constructor");
    this->addr = addr;
    this->port = port;
    this->sda = sda;
    this->scl = scl;
    this->freq = freq;
}

I2C_Device::I2C_Device(I2C_Params params) {
    ESP_LOGV(TAG, "Constructor(params)");
    this->addr = params.addr;
    this->port = params.port;
    this->sda = params.sda;
    this->scl = params.scl;
    this->freq = params.freq;
}


esp_err_t I2C_Device::Init() {
    ESP_LOGV(TAG, "Init()");

    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = this->sda;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_io_num = this->scl;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = this->freq;
    i2c_param_config(this->port, &conf);
    return i2c_driver_install(this->port, conf.mode,0, 0, 0);    
}

esp_err_t I2C_Device::Read(uint8_t* out_data, size_t out_size, uint8_t* rd_data, size_t rd_size) {
    ESP_LOGV(TAG, "Read(len=%d)", rd_size);

    if ( (rd_data==NULL) || (rd_size <= 0) ) {
        ESP_LOGE(TAG, "i2c device read invalid args");
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t ret = ESP_OK;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    if ( (out_data != NULL ) && ( out_size > 0 ) ) {
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (  this->addr << 1 ) | I2C_MASTER_WRITE, ACK_CHECK_EN);
        i2c_master_write(cmd, out_data, out_size, ACK_CHECK_EN);
    }
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, ( this->addr << 1 ) | I2C_MASTER_READ, ACK_CHECK_EN);
    i2c_master_read(cmd, rd_data, rd_size, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);

    ret = i2c_master_cmd_begin(this->port, cmd, 1000/portTICK_RATE_MS);
    if ( ret != ESP_OK ) {
        ESP_LOGE(TAG, "Fail to read device ( port = %d, addr = %02x )", this->port, this->addr);
    }
    i2c_cmd_link_delete(cmd);
    return ret;
}

esp_err_t I2C_Device::Write(uint8_t* out_data, size_t out_size, uint8_t* wr_data, size_t wr_size) {
    ESP_LOGV(TAG, "Write(len=%d)", wr_size);
    
    if ( (wr_data==NULL) || (wr_size <= 0) ) {
        ESP_LOGE(TAG, "i2c device write invalid args");
        return ESP_ERR_INVALID_ARG;
    }    

    esp_err_t ret = ESP_OK;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, ( this->addr << 1 ) | I2C_MASTER_WRITE, ACK_CHECK_EN);
    if ( out_data != NULL && out_size > 0 ) {
        i2c_master_write(cmd, out_data, out_size, ACK_CHECK_EN);
    }
    i2c_master_write(cmd, wr_data, wr_size, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(this->port, cmd, 1000/portTICK_RATE_MS);
    if ( ret != ESP_OK ) {
        ESP_LOGE(TAG, "Fail to write decice ( port = %d, addr = %02x )", this->port, this->addr);
    }
    i2c_cmd_link_delete(cmd);    

    return ret;    
}