#include "stm32f4xx_hal.h"

#ifndef MPU6050_H
#define MPU6050_H

#define MPU6050_ADDR                0x68
#define MPU6050_REG_PWR_MGMT_1      0x6B
#define MPU6050_REG_WHO_AM_I        0x75
#define MPU6050_REG_ACCEL_XOUT_H    0x3B
#define MPU6050_REG_GYRO_XOUT_H     0x43

#endif

typedef enum
{
    MPU6050_OK = 0,
    MPU6050_ERR_BUS,
    MPU6050_ERR_TIMEOUT,
    MPU6050_ERR_ID
} mpu6050_status_t;

mpu6050_status_t mpu6050_read_who_am_i(
    I2C_HandleTypeDef *hi2c,
    uint8_t *who_am_i
);

mpu6050_status_t mpu6050_wake(
    I2C_HandleTypeDef *hi2c
);

mpu6050_status_t mpu6050_read_accel_raw(
    I2C_HandleTypeDef *hi2c,
    int16_t *ax,
    int16_t *ay,
    int16_t *az
);

mpu6050_status_t mpu6050_read_gyro_raw(
    I2C_HandleTypeDef *hi2c,
    int16_t *gx,
    int16_t *gy,
    int16_t *gz
);
