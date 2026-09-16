#include "mpu6050.h"

static mpu6050_status_t mpu6050_map_hal_status(HAL_StatusTypeDef status)
{
    if (status == HAL_OK)
    {
        return MPU6050_OK;
    }

    if (status == HAL_TIMEOUT)
    {
        return MPU6050_ERR_TIMEOUT;
    }

    return MPU6050_ERR_BUS;
}

mpu6050_status_t mpu6050_read_who_am_i(
    I2C_HandleTypeDef *hi2c,
    uint8_t *who_am_i
)
{
    HAL_StatusTypeDef status = HAL_I2C_Mem_Read(
        hi2c,
        MPU6050_ADDR << 1,
        MPU6050_REG_WHO_AM_I,
        I2C_MEMADD_SIZE_8BIT,
        who_am_i,
        1,
        100
    );

    if (status != HAL_OK)
    {
        return mpu6050_map_hal_status(status);
    }

    if (*who_am_i != MPU6050_ADDR)
    {
        return MPU6050_ERR_ID;
    }

    return MPU6050_OK;
}

mpu6050_status_t mpu6050_wake(
    I2C_HandleTypeDef *hi2c
)
{
    uint8_t power_config = 0x00;

    HAL_StatusTypeDef status = HAL_I2C_Mem_Write(
        hi2c,
        MPU6050_ADDR << 1,
        MPU6050_REG_PWR_MGMT_1,
        I2C_MEMADD_SIZE_8BIT,
        &power_config,
        1,
        100
    );

    return mpu6050_map_hal_status(status);
}

mpu6050_status_t mpu6050_read_accel_raw(
    I2C_HandleTypeDef *hi2c,
    int16_t *ax,
    int16_t *ay,
    int16_t *az
)
{
	uint8_t accel_buf[6] = {0};
	HAL_StatusTypeDef status = HAL_I2C_Mem_Read(
			hi2c,
			MPU6050_ADDR << 1,
			MPU6050_REG_ACCEL_XOUT_H,
			I2C_MEMADD_SIZE_8BIT,
			accel_buf,
			6,
			100
	);
	if(status != HAL_OK){
		return mpu6050_map_hal_status(status);
	}
	*ax = (int16_t)((accel_buf[0] << 8) | accel_buf[1]);
	*ay = (int16_t)((accel_buf[2] << 8) | accel_buf[3]);
	*az = (int16_t)((accel_buf[4] << 8) | accel_buf[5]);
	return MPU6050_OK;
}

mpu6050_status_t mpu6050_read_gyro_raw(
    I2C_HandleTypeDef *hi2c,
    int16_t *gx,
    int16_t *gy,
    int16_t *gz
)
{
	uint8_t gyro_buf[6] = {0};
	HAL_StatusTypeDef status = HAL_I2C_Mem_Read(
			hi2c,
			MPU6050_ADDR << 1,
			MPU6050_REG_GYRO_XOUT_H,
			I2C_MEMADD_SIZE_8BIT,
			gyro_buf,
			6,
			100
	);
	if(status != HAL_OK){
		return mpu6050_map_hal_status(status);
	}
	*gx = (int16_t)((gyro_buf[0] << 8) | gyro_buf[1]);
	*gy = (int16_t)((gyro_buf[2] << 8) | gyro_buf[3]);
	*gz = (int16_t)((gyro_buf[4] << 8) | gyro_buf[5]);
	return MPU6050_OK;
}
