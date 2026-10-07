#ifndef SSD1306_H
#define SSD1306_H

#include <stdint.h>
#include "stm32f4xx_hal.h"

#define SSD1306_ADDR   (0x3C << 1)   /* địa chỉ 7-bit dịch trái 1 bit cho HAL */
#define SSD1306_CTRL_CMD   0x00      /* byte đầu = 0x00: các byte sau là lệnh */
#define SSD1306_CTRL_DATA  0x40      /* byte đầu = 0x40: các byte sau là dữ liệu pixel */

HAL_StatusTypeDef ssd1306_write_cmd(I2C_HandleTypeDef *hi2c, uint8_t cmd);

HAL_StatusTypeDef ssd1306_write_data(I2C_HandleTypeDef *hi2c, const uint8_t *data, uint16_t len);

HAL_StatusTypeDef ssd1306_init(I2C_HandleTypeDef *hi2c);

HAL_StatusTypeDef ssd1306_clear(I2C_HandleTypeDef *hi2c);

HAL_StatusTypeDef ssd1306_set_pos(I2C_HandleTypeDef *hi2c, uint8_t col, uint8_t page);

HAL_StatusTypeDef ssd1306_write_char(I2C_HandleTypeDef *hi2c, char c);

HAL_StatusTypeDef ssd1306_write_str(I2C_HandleTypeDef *hi2c, const char *s);

HAL_StatusTypeDef ssd1306_write_char2x(I2C_HandleTypeDef *hi2c, char c, uint8_t col, uint8_t page);

HAL_StatusTypeDef ssd1306_write_str2x(I2C_HandleTypeDef *hi2c, const char *s, uint8_t col, uint8_t page);

#endif