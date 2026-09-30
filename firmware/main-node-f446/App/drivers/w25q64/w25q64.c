#include "drivers/w25q64/w25q64.h"

#define W25Q64_CMD_JEDEC_ID     0x9F
#define W25Q64_SPI_TIMEOUT_MS   100

static w25q64_status_t to_status(HAL_StatusTypeDef st)
{
    switch(st){
        case HAL_OK:
            return W25Q64_OK;
        case HAL_TIMEOUT:
            return W25Q64_ERR_TIMEOUT;
        default:
            return W25Q64_ERR_BUS;
    }
}

static void cs_select(const w25q64_t *dev)
{
    HAL_GPIO_WritePin(dev->cs_port, dev->cs_pin, GPIO_PIN_RESET);
}

static void cs_deselect(const w25q64_t *dev)
{
    HAL_GPIO_WritePin(dev->cs_port, dev->cs_pin, GPIO_PIN_SET);
}

w25q64_status_t w25q64_read_jedec_id(const w25q64_t *dev, uint8_t id[3])
{
    uint8_t tx[4] = { W25Q64_CMD_JEDEC_ID, 0x00, 0x00, 0x00 };
    uint8_t rx[4] = { 0 };

    cs_select(dev);
    HAL_StatusTypeDef st = HAL_SPI_TransmitReceive(dev->hspi, tx, rx, sizeof(tx), W25Q64_SPI_TIMEOUT_MS);
    cs_deselect(dev);
    if(st != HAL_OK){
        return to_status(st);
    }
    for(int i = 0; i < 3; i++){
        id[i] = rx[i + 1];
    }
    return W25Q64_OK;
}
