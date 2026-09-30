#include "drivers/mpu6050/mpu6050.h"

#define MPU6050_I2C_ADDR      0x68U
#define MPU6050_REG_WHO_AM_I  0x75U
#define MPU6050_I2C_TIMEOUT_MS        100U
#define MPU6050_REG_PWR_MGMT_1    0x6BU
#define MPU6050_WHO_AM_I_VALUE    0x68U
#define MPU6050_PWR_MGMT_1_WAKE   0x00U   /* SLEEP bit = 0, internal 8 MHz clock */

static mpu6050_status_t read_regs(I2C_HandleTypeDef *hi2c, uint8_t reg,
                                  uint8_t *buf, uint16_t len){
    HAL_StatusTypeDef st = HAL_I2C_Mem_Read(
        hi2c,
        MPU6050_I2C_ADDR << 1,
        reg,
        I2C_MEMADD_SIZE_8BIT,
        buf,
        len,
        MPU6050_I2C_TIMEOUT_MS);

    switch (st){
        case HAL_OK:
            return MPU6050_OK;
        case HAL_TIMEOUT:
            return MPU6050_ERR_TIMEOUT;
        default:
            return MPU6050_ERR_BUS;
    }
}

static mpu6050_status_t write_regs(I2C_HandleTypeDef *hi2c, uint8_t reg,
                                   const uint8_t *data, uint16_t len){
    HAL_StatusTypeDef st = HAL_I2C_Mem_Write(
        hi2c,
        MPU6050_I2C_ADDR << 1,
        reg,
        I2C_MEMADD_SIZE_8BIT,
        (uint8_t *)data,
        len,
        MPU6050_I2C_TIMEOUT_MS);

    switch (st){
        case HAL_OK:
            return MPU6050_OK;
        case HAL_TIMEOUT:
            return MPU6050_ERR_TIMEOUT;
        default:
            return MPU6050_ERR_BUS;
    }
}

mpu6050_status_t mpu6050_read_who_am_i(I2C_HandleTypeDef *hi2c, uint8_t *who_am_i){
    return read_regs(hi2c,  MPU6050_REG_WHO_AM_I, who_am_i, 1);
}

mpu6050_status_t mpu6050_init(I2C_HandleTypeDef *hi2c){
    uint8_t id = 0;
    mpu6050_status_t status;

    status = mpu6050_read_who_am_i(hi2c, &id);
    if(status != MPU6050_OK){
        return status;
    }
    if(id != MPU6050_WHO_AM_I_VALUE){
        return MPU6050_ERR_ID;
    }

    const uint8_t wake = MPU6050_PWR_MGMT_1_WAKE;
    return write_regs(hi2c, MPU6050_REG_PWR_MGMT_1, &wake, 1);
}
