#ifndef MPU6050_H
#define MPU6050_H

#include <stdint.h>
#include "stm32f4xx_hal.h"

typedef enum {
    MPU6050_OK = 0,
    MPU6050_ERR_BUS,
    MPU6050_ERR_ID,
    MPU6050_ERR_TIMEOUT
} mpu6050_status_t;

mpu6050_status_t mpu6050_read_who_am_i(I2C_HandleTypeDef *hi2c, uint8_t *who_am_i);

mpu6050_status_t mpu6050_init(I2C_HandleTypeDef *hi2c);

#endif /* MPU6050_H */
