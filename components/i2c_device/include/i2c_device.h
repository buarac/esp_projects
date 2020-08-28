#ifndef _I2C_DEVICE_H_
#define _I2C_DEVICE_H_

#include "driver/i2c.h"

#define ACK_CHECK_EN    0x1    /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS   0x0    /*!< I2C master will not check ack from slave */
#define ACK_VAL         0x0    /*!< I2C ack value */
#define NACK_VAL        0x1    /*!< I2C nack value */

///////////////////////////////////////
/*
typedef struct {
    // fields
    uint8_t     addr;
    i2c_port_t  port;
    uint8_t     sda;
    uint8_t     scl;
    uint32_t    freq;
} i2c_device_t;

i2c_device_t* i2c_device_create(uint8_t addr, i2c_port_t port, uint8_t sda, uint8_t scl, uint32_t freq);
esp_err_t i2c_device_init(i2c_device_t* i2c);
void i2c_device_deinit(i2c_device_t* i2c);
esp_err_t i2c_device_read(i2c_device_t* i2c, void* out_data, size_t out_size, void* in_data, size_t in_size);
esp_err_t i2c_device_write(i2c_device_t* i2c, void* reg_data, size_t reg_size, void* out_data, size_t out_size);
*/

class I2C_Device {
    private:
        uint8_t     addr;
        i2c_port_t  port;
        uint8_t     sda;
        uint8_t     scl;
        uint32_t    freq;
    public:
        I2C_Device(uint8_t addr, i2c_port_t port, uint8_t sda, uint8_t scl, uint32_t freq);
        esp_err_t init();
        esp_err_t read(uint8_t* out_data, size_t out_size, uint8_t* rd_data, size_t rd_size);
        esp_err_t write(uint8_t* out_data, size_t out_size, uint8_t* wr_data, size_t wr_size);
};

#endif // _I2C_DEVICE_H