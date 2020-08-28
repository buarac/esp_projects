#include "esp_log.h"
#include "i2cdev.h"
#include <string.h>

static const char* TAG = "i2c_device";

//////////////////
i2c_device_t* i2c_device_create(uint8_t addr, i2c_port_t port, uint8_t sda, uint8_t scl, uint32_t freq) {
    i2c_device_t* device = malloc(sizeof(i2c_device_t));
    if ( device == NULL ) {
        ESP_LOGE(TAG, "failed to create i2c device object");
        return NULL;
    }

    device->addr = addr;
    device->port = port;
    device->freq = freq;
    device->sda = sda;
    device->scl = scl;

    return device;
}

void i2c_device_deinit(i2c_device_t* i2c) {
    // deinstall driver

    //
    if ( i2c != NULL ) {
        free(i2c);
    }
}

esp_err_t i2c_device_init(i2c_device_t* i2c) {
    ESP_LOGV(TAG, "i2c_device_init");

    if ( i2c == NULL ) {
        ESP_LOGE(TAG, "i2c device object is null");
        return ESP_ERR_INVALID_ARG;
    }

    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = i2c->sda;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_io_num = i2c->scl;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = i2c->freq;
    i2c_param_config(i2c->port, &conf);
    return i2c_driver_install(i2c->port, conf.mode,0, 0, 0);    
}

esp_err_t i2c_device_read(i2c_device_t* i2c, void* out_data, size_t out_size, void* in_data, size_t in_size) {
    ESP_LOGV(TAG, "i2c_device_read(len=%d)", in_size);

    if ( (i2c==NULL) || (in_data==NULL) || (in_size <= 0) ) {
        ESP_LOGE(TAG, "i2c device read invalid args");
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t ret = ESP_OK;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    if ( (out_data != NULL ) && ( out_size > 0 ) ) {
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, ( i2c->addr << 1 ) | I2C_MASTER_WRITE, ACK_CHECK_EN);
        i2c_master_write(cmd, out_data, out_size, ACK_CHECK_EN);
    }
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, ( i2c->addr << 1 ) | I2C_MASTER_READ, ACK_CHECK_EN);
    i2c_master_read(cmd, in_data, in_size, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);

    ret = i2c_master_cmd_begin(i2c->port, cmd, 1000/portTICK_RATE_MS);
    if ( ret != ESP_OK ) {
        ESP_LOGE(TAG, "Fail to read device ( port = %d, addr = %02x )", i2c->port, i2c->addr);
    }
    i2c_cmd_link_delete(cmd);
    return ret;
}

esp_err_t i2c_device_write(i2c_device_t* i2c, void* reg_data, size_t reg_size, void* out_data, size_t out_size) {
    ESP_LOGV(TAG, "i2c_device_write(len=%d)", out_size);
    
    if ( (i2c==NULL) || (out_data==NULL) || (out_size <= 0) ) {
        ESP_LOGE(TAG, "i2c device write invalid args");
        return ESP_ERR_INVALID_ARG;
    }    

    esp_err_t ret = ESP_OK;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, ( i2c->addr << 1 ) | I2C_MASTER_WRITE, ACK_CHECK_EN);
    if ( reg_data != NULL && reg_size > 0 ) {
        i2c_master_write(cmd, reg_data, reg_size, ACK_CHECK_EN);
    }
    i2c_master_write(cmd, out_data, out_size, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(i2c->port, cmd, 1000/portTICK_RATE_MS);
    if ( ret != ESP_OK ) {
        ESP_LOGE(TAG, "Fail to write decice ( port = %d, addr = %02x )", i2c->port, i2c->addr);
    }
    i2c_cmd_link_delete(cmd);    

    return ret;    
}