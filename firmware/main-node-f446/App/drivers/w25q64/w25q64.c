#include "drivers/w25q64/w25q64.h"

#define W25Q64_CMD_JEDEC_ID     0x9F
#define W25Q64_SPI_TIMEOUT_MS   100
#define W25Q64_CMD_READ_SR1     0x05
#define W25Q64_CMD_WRITE_ENABLE 0x06
#define W25Q64_SR1_BUSY         (1u << 0)
#define W25Q64_SR1_WEL          (1u << 1)
#define W25Q64_CMD_SECTOR_ERASE         0x20
#define W25Q64_SECTOR_ERASE_TIMEOUT_MS  400   /* tSE max, datasheet */
#define W25Q64_CMD_PAGE_PROGRAM         0x02
#define W25Q64_PAGE_SIZE                256U
#define W25Q64_PAGE_PROGRAM_TIMEOUT_MS  5     /* tPP max 3 ms + tick margin */
#define W25Q64_CAPACITY          0x800000UL   /* 8 MB = 64 Mbit */
#define W25Q64_CMD_READ_DATA     0x03
#define W25Q64_CHIP_ERASE_TIMEOUT_MS 120000UL

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

w25q64_status_t w25q64_read_status(const w25q64_t *dev, uint8_t *sr1)
{
    uint8_t tx[2] = {W25Q64_CMD_READ_SR1, 0x00};
    uint8_t rx[2] = {0};

    cs_select(dev);
    HAL_StatusTypeDef st = HAL_SPI_TransmitReceive(dev->hspi, tx, rx, sizeof(tx), W25Q64_SPI_TIMEOUT_MS);
    cs_deselect(dev);
    if(st != HAL_OK){
        return to_status(st);
    }
    *sr1 = rx[1];
    return W25Q64_OK;
}

w25q64_status_t w25q64_wait_busy(const w25q64_t *dev, uint32_t timeout_ms)
{
    uint32_t start_tick = HAL_GetTick();
    uint8_t sr1;
    while(1){
        /* Chốt timeout TRƯỚC khi đọc SR1: nếu task bị task khác chiếm CPU quá timeout,
         * vẫn còn một lần đọc sau mốc timeout, chip xong rồi thì trả OK, không báo TIMEOUT giả. */
        int timed_out = (HAL_GetTick() - start_tick > timeout_ms);
        w25q64_status_t st = w25q64_read_status(dev, &sr1);
        if(st != W25Q64_OK){
            return st;
        }
        if((sr1 & (uint8_t)W25Q64_SR1_BUSY) == 0){
            return W25Q64_OK;
        }
        if(timed_out){
            return W25Q64_ERR_TIMEOUT;
        }
    }
}

w25q64_status_t w25q64_write_enable(const w25q64_t *dev)
{
    uint8_t tx[1] = {W25Q64_CMD_WRITE_ENABLE};
    cs_select(dev);
    HAL_StatusTypeDef st = HAL_SPI_Transmit(dev->hspi, tx, sizeof(tx), W25Q64_SPI_TIMEOUT_MS);
    cs_deselect(dev);
    return to_status(st);
}

w25q64_status_t w25q64_sector_erase(const w25q64_t *dev, uint32_t addr)
{
    w25q64_status_t st = w25q64_write_enable(dev);
    if(st != W25Q64_OK){
        return st;
    }
    uint8_t tx[4] = {W25Q64_CMD_SECTOR_ERASE, 0x00, 0x00, 0x00};
    tx[1] = (addr >> 16) & 0xFF;
    tx[2] = (addr >> 8) & 0xFF;
    tx[3] = addr & 0xFF;
    cs_select(dev);
    HAL_StatusTypeDef hal_st = HAL_SPI_Transmit(dev->hspi, tx, sizeof(tx), W25Q64_SPI_TIMEOUT_MS);
    cs_deselect(dev);
    if(hal_st != HAL_OK){
        return to_status(hal_st);
    }
    st = w25q64_wait_busy(dev, W25Q64_SECTOR_ERASE_TIMEOUT_MS);
    return st;
}

w25q64_status_t w25q64_page_program(const w25q64_t *dev, uint32_t addr,
                                    const uint8_t *data, uint16_t len)
{
    uint32_t offset = addr & 0xFF;
    uint32_t remaining = W25Q64_PAGE_SIZE - offset;
    if(len == 0 || data == NULL || addr >= W25Q64_CAPACITY || len > remaining){
        return W25Q64_ERR_PARAM;
    }
    w25q64_status_t st = w25q64_write_enable(dev);
    if(st != W25Q64_OK){
        return st;
    }
    uint8_t tx[4] = {W25Q64_CMD_PAGE_PROGRAM, 0x00, 0x00, 0x00};
    tx[1] = (addr >> 16) & 0xFF;
    tx[2] = (addr >> 8) & 0xFF;
    tx[3] = addr & 0xFF;
    cs_select(dev);
    HAL_StatusTypeDef hal_st = HAL_SPI_Transmit(dev->hspi, tx, sizeof(tx), W25Q64_SPI_TIMEOUT_MS);
    if(hal_st != HAL_OK){
        cs_deselect(dev);
        return to_status(hal_st);
    }
    hal_st = HAL_SPI_Transmit(dev->hspi, data, len, W25Q64_SPI_TIMEOUT_MS);
    cs_deselect(dev);
    if(hal_st != HAL_OK){
        return to_status(hal_st);
    }
    st = w25q64_wait_busy(dev, W25Q64_PAGE_PROGRAM_TIMEOUT_MS);
    return st;
}

w25q64_status_t w25q64_read_data(const w25q64_t *dev, uint32_t addr, uint8_t *data, uint16_t len){
    if(len == 0 || data == NULL || addr >= W25Q64_CAPACITY || len > W25Q64_CAPACITY - addr){
        return W25Q64_ERR_PARAM;
    }
    uint8_t tx[4] = {W25Q64_CMD_READ_DATA, 0x00, 0x00, 0x00};
    tx[1] = (addr >> 16) & 0xFF;
    tx[2] = (addr >> 8) & 0xFF;
    tx[3] = addr & 0xFF;
    cs_select(dev);
    HAL_StatusTypeDef hal_st = HAL_SPI_Transmit(dev->hspi, tx, sizeof(tx), W25Q64_SPI_TIMEOUT_MS);
    if(hal_st != HAL_OK){
        cs_deselect(dev);
        return to_status(hal_st);
    }
    hal_st = HAL_SPI_Receive(dev->hspi, data, len, W25Q64_SPI_TIMEOUT_MS);
    cs_deselect(dev);
    if(hal_st != HAL_OK){
        return to_status(hal_st);
    }
    return W25Q64_OK;
}

w25q64_status_t w25q64_chip_erase(const w25q64_t *dev){
    w25q64_status_t st = w25q64_write_enable(dev);
    if(st != W25Q64_OK){
        return st;
    }
    uint8_t tx = 0xC7;
    cs_select(dev);
    HAL_StatusTypeDef hal_st = HAL_SPI_Transmit(dev->hspi, &tx, 1, W25Q64_SPI_TIMEOUT_MS);
    cs_deselect(dev);
    if(hal_st != HAL_OK){
        return to_status(hal_st);
    }
    st = w25q64_wait_busy(dev, W25Q64_CHIP_ERASE_TIMEOUT_MS);
    return st;
}
