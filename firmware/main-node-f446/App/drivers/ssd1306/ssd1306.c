#include <string.h>
#include "drivers/ssd1306/ssd1306.h"
#include "drivers/ssd1306/font5x7.h"

HAL_StatusTypeDef ssd1306_write_cmd(I2C_HandleTypeDef *hi2c, uint8_t cmd){
    uint8_t buf[2] = { SSD1306_CTRL_CMD, cmd };
    return HAL_I2C_Master_Transmit(hi2c, SSD1306_ADDR, buf, 2, 100);
}

HAL_StatusTypeDef ssd1306_write_data(I2C_HandleTypeDef *hi2c, const uint8_t *data, uint16_t len){
    uint8_t buf[129] = {0};
    buf[0] = SSD1306_CTRL_DATA;
    if(len > 128){
        return HAL_ERROR;
    }
    memcpy(&buf[1], data, len);
    return HAL_I2C_Master_Transmit(hi2c, SSD1306_ADDR, buf, len + 1, 100);
}

HAL_StatusTypeDef ssd1306_init(I2C_HandleTypeDef *hi2c){
    static const uint8_t init_seq[] = {
        0xAE,        /* display OFF */
        0xD5, 0x80,  /* clock divide ratio / tần số dao động */
        0xA8, 0x3F,  /* multiplex ratio = 64 hàng (0x3F = 63) */
        0xD3, 0x00,  /* display offset = 0 */
        0x40,        /* start line = 0 */
        0x8D, 0x14,  /* charge pump ON */
        0x20, 0x00,  /* horizontal addressing mode */
        0xA1,        /* segment remap */
        0xC8,        /* COM scan đảo */
        0xDA, 0x12,  /* COM pins: alternative config (128x64) */
        0x81, 0xCF,  /* contrast */
        0xD9, 0xF1,  /* pre-charge period */
        0xDB, 0x40,  /* VCOMH deselect level */
        0xA4,        /* hiện nội dung RAM (không ép sáng toàn màn) */
        0xA6,        /* chế độ hiển thị bình thường (không đảo màu) */
        0xAF         /* display ON */
    };
    for(uint8_t i = 0; i < sizeof(init_seq); i++){
        HAL_StatusTypeDef st = ssd1306_write_cmd(hi2c, init_seq[i]);
        if(st != HAL_OK){
            return st;
        }
    }
    return HAL_OK;
}

HAL_StatusTypeDef ssd1306_clear(I2C_HandleTypeDef *hi2c){
    static const uint8_t clear[] = {
    0x21, 0x00, 0x7F,
    0x22, 0x00, 0x07}; 
    for(uint8_t i = 0; i < sizeof(clear); i++){
        HAL_StatusTypeDef st = ssd1306_write_cmd(hi2c, clear[i]);
        if(st != HAL_OK){
            return st;
        }
    }
    uint8_t zeros[128] = {0};
    for(int i = 0; i < 8; i++){
        HAL_StatusTypeDef st = ssd1306_write_data(hi2c, zeros, 128);
        if(st != HAL_OK){
            return st;
        }
    }
    return HAL_OK;
}
 
HAL_StatusTypeDef ssd1306_set_pos(I2C_HandleTypeDef *hi2c, uint8_t col, uint8_t page){
    if(col > 127 || page > 7){
        return HAL_ERROR;
    }
    uint8_t seq[6] = {0x21, col, 0x7F, 0x22, page, 0x07};
    for(uint8_t i = 0; i < sizeof(seq); i++){
        HAL_StatusTypeDef st = ssd1306_write_cmd(hi2c, seq[i]);
        if(st != HAL_OK){
            return st;
        }
    }
    return HAL_OK;
}

HAL_StatusTypeDef ssd1306_write_char(I2C_HandleTypeDef *hi2c, char c){
    if(c < FONT5X7_FIRST || c > FONT5X7_LAST){
        c = '?';
    }
    uint8_t buf[6];
    memcpy(buf, font5x7[c - FONT5X7_FIRST], 5);
    buf[5] = 0x00;
    return ssd1306_write_data(hi2c, buf, 6);
}

HAL_StatusTypeDef ssd1306_write_str(I2C_HandleTypeDef *hi2c, const char *s){
    while(*s){
        HAL_StatusTypeDef st = ssd1306_write_char(hi2c, *s);
        if(st != HAL_OK){
            return st;
        }
        s++;
    }
    return HAL_OK;
}

static uint16_t stretch_v(uint8_t b){
    uint16_t r = 0;
    for(int i = 0; i < 8; i++){
        if(b & (1u << i)){
            r |= (3u << (2 * i));
        }
    }
    return r;
}

HAL_StatusTypeDef ssd1306_write_char2x(I2C_HandleTypeDef *hi2c, char c, uint8_t col, uint8_t page){
    if(col > 116 || page > 6){
        return HAL_ERROR;
    }
    if(c < FONT5X7_FIRST || c > FONT5X7_LAST){
        c = '?';
    }
    uint8_t top[12] = {0}; 
    uint8_t bot[12] = {0};
    for(int i = 0; i < 5; i++){
        uint16_t v = stretch_v(font5x7[c - FONT5X7_FIRST][i]);
        top[2*i] = top[2*i+1] = v & 0xFF;     // nửa trên (page)
        bot[2*i] = bot[2*i+1] = v >> 8;       // nửa dưới (page+1)
    }
    HAL_StatusTypeDef st = ssd1306_set_pos(hi2c, col, page);
    if(st != HAL_OK){
        return st;
    }
    st = ssd1306_write_data(hi2c, top, 12);
    if(st != HAL_OK){
        return st;
    }
    st = ssd1306_set_pos(hi2c, col, page + 1);
    if(st != HAL_OK){
        return st;
    }
    st = ssd1306_write_data(hi2c, bot, 12);
    if(st != HAL_OK){
        return st;
    }
    return HAL_OK;
}

HAL_StatusTypeDef ssd1306_write_str2x(I2C_HandleTypeDef *hi2c, const char *s, uint8_t col, uint8_t page){
    while(*s && col <= 116){
        HAL_StatusTypeDef st = ssd1306_write_char2x(hi2c, *s, col, page);
        if(st != HAL_OK){
            return st;
        }
        col+=12;
        s++;
    }
    return HAL_OK;
}
