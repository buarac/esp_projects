#ifndef _I2C_DEVICE_H_
#define _I2C_DEVICE_H_

#include "driver/i2c.h"

#define ACK_CHECK_EN    0x1    /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS   0x0    /*!< I2C master will not check ack from slave */
#define ACK_VAL         0x0    /*!< I2C ack value */
#define NACK_VAL        0x1    /*!< I2C nack value */

typedef struct {
    uint8_t     addr;
    i2c_port_t  port;
    uint8_t     sda_pin;
    uint8_t     scl_pin;
    uint32_t    freq;
} i2cdev_params_t;


esp_err_t i2cdev_setup(i2cdev_params_t* dev);
esp_err_t i2cdev_read(i2cdev_params_t* dev, void* out_data, size_t out_size, void* in_data, size_t in_size);
esp_err_t i2cdev_write(i2cdev_params_t* dev, void* reg_data, size_t reg_size, void* out_data, size_t out_size);



#endif // _I2C_DEVICE_H