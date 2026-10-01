#ifndef W25Q64_H
#define W25Q64_H

#include <stdint.h>
#include "stm32f4xx_hal.h"

typedef enum{
    W25Q64_OK = 0,
    W25Q64_ERR_BUS,
    W25Q64_ERR_TIMEOUT
} w25q64_status_t;

typedef struct {
    SPI_HandleTypeDef *hspi;
    GPIO_TypeDef      *cs_port;
    uint16_t           cs_pin;
} w25q64_t;

/* Reads 3 bytes: manufacturer, memory type, capacity. */
w25q64_status_t w25q64_read_jedec_id(const w25q64_t *dev, uint8_t id[3]);

/* Reads Status Register-1. */
w25q64_status_t w25q64_read_status(const w25q64_t *dev, uint8_t *sr1);

/* Polls BUSY until it clears or timeout_ms elapses. */
w25q64_status_t w25q64_wait_busy(const w25q64_t *dev, uint32_t timeout_ms);

w25q64_status_t w25q64_write_enable(const w25q64_t *dev);

#endif /* W25Q64 */

