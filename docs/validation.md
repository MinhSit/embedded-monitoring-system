# Validation

## I2C — MPU6050 WHO_AM_I

![I2C WHO_AM_I](images/i2c_mpu6050_who_am_i.png)

- Tool: 24 MHz logic analyzer + PulseView, sample rate 2 MHz
- Bus: I2C1, SCL = PB8 (CH0), SDA = PB9 (CH1)
- I2C clock: ~100 kHz
- Address: 0x68 (write, then read)
- Register: 0x75 (WHO_AM_I)
- Returned value: 0x68
- Sequence: S, addr W, ACK, reg, ACK, Sr, addr R, ACK, data, NACK, P

## I2C — MPU6050 init (WHO_AM_I check + wake-up)

![I2C init](images/i2c_mpu6050_init.png)

- Transaction 1: read WHO_AM_I (0x75) → 0x68
- Transaction 2: write PWR_MGMT_1 (0x6B) = 0x00 → clear SLEEP bit
- All bytes ACKed by sensor

## SPI1 — W25Q64 JEDEC ID (0x9F)

- Setup: SPI1 mode 0, prescaler 256 (≈328 kHz), CS = PB6 (software)
- Capture: PulseView, 2 MHz — D0 = CS, D1 = SCK, D2 = MOSI, D3 = MISO

![SPI JEDEC ID](images/spi_w25q64_jedec_id.png)

| Line | Bytes |
|------|-------|
| MOSI | 9F 00 00 00 |
| MISO | FF EF 40 17 |

- Result: EF = Winbond, 40 17 = W25Q64 (64 Mbit) — matches datasheet
- SCK measured ≈330 kHz; idle LOW (mode 0)
- ~1 µs gap between bytes: HAL polling overhead (DMA planned in Block F)
- Negative test: MISO disconnected → `00 00 00` with HAL_OK
  → SPI has no ACK; driver must validate the ID itself