#include "drivers/mpu6050/mpu6050.h"

#define MPU6050_I2C_ADDR      0x68U
#define MPU6050_REG_WHO_AM_I  0x75U
#define MPU6050_I2C_TIMEOUT_MS        100U

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

mpu6050_status_t mpu6050_read_who_am_i(I2C_HandleTypeDef *hi2c, uint8_t *who_am_i){
    return read_regs(hi2c,  MPU6050_REG_WHO_AM_I, who_am_i, 1);
}
