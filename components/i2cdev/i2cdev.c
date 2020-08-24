#include "esp_log.h"
#include "i2cdev.h"
#include <string.h>

static const char* TAG = "i2cdev";



esp_err_t i2cdev_setup(i2cdev_params_t* dev) {
    ESP_LOGV(TAG, "i2cdev_setup");

    if ( (dev == NULL) || (dev->port < 0) || (dev->port >= I2C_NUM_MAX) ) {
        ESP_LOGE(TAG, "bad args");
        return ESP_ERR_INVALID_ARG;
    }

    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = dev->sda_pin;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_io_num = dev->scl_pin;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = dev->freq;
    i2c_param_config(dev->port, &conf);
    return i2c_driver_install(dev->port, conf.mode,0, 0, 0);
}

esp_err_t i2cdev_read(i2cdev_params_t* dev, void* out_data, size_t out_size, void* in_data, size_t in_size) {
    ESP_LOGV(TAG, "i2cdev_read(len=%d)", in_size);

    if ( (dev==NULL) || (in_data==NULL) || (in_size <= 0) ) {
        ESP_LOGE(TAG, "i2c device read invalid args");
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t ret = ESP_OK;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    if ( (out_data != NULL ) && ( out_size > 0 ) ) {
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, ( dev->addr << 1 ) | I2C_MASTER_WRITE, ACK_CHECK_EN);
        i2c_master_write(cmd, out_data, out_size, ACK_CHECK_EN);
    }
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, ( dev->addr << 1 ) | I2C_MASTER_READ, ACK_CHECK_EN);
    i2c_master_read(cmd, in_data, in_size, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);

    ret = i2c_master_cmd_begin(dev->port, cmd, 1000/portTICK_RATE_MS);
    if ( ret != ESP_OK ) {
        ESP_LOGE(TAG, "Fail to read device ( port = %d, addr = %02x )", dev->port, dev->addr);
    }
    i2c_cmd_link_delete(cmd);
    return ret;
}

esp_err_t i2cdev_write(i2cdev_params_t* dev, void* reg_data, size_t reg_size, void* out_data, size_t out_size) { 
    ESP_LOGV(TAG, "i2cdev_write(len=%d)", out_size);
    
    if ( (dev==NULL) || (out_data==NULL) || (out_size <= 0) ) {
        ESP_LOGE(TAG, "i2c device write invalid args");
        return ESP_ERR_INVALID_ARG;
    }    

    esp_err_t ret = ESP_OK;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, ( dev->addr << 1 ) | I2C_MASTER_WRITE, ACK_CHECK_EN);
    if ( reg_data != NULL && reg_size > 0 ) {
        i2c_master_write(cmd, reg_data, reg_size, ACK_CHECK_EN);
    }
    i2c_master_write(cmd, out_data, out_size, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(dev->port, cmd, 1000/portTICK_RATE_MS);
    if ( ret != ESP_OK ) {
        ESP_LOGE(TAG, "Fail to write decice ( port = %d, addr = %02x )", dev->port, dev->addr);
    }
    i2c_cmd_link_delete(cmd);    

    return ret;
}
