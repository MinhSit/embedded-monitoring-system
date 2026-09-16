# MPU6050 Fault / Recovery Test

## Normal operation
- WHO_AM_I = 0x68
- Accel ≈ 1 g on Z when board is flat
- Gyro near 0 dps at rest
- Status = MPU6050_OK

## Sensor disconnected
- Wake/read functions return MPU6050_ERR_BUS
- No invalid accel/gyro data is processed
- Firmware continues to Boot OK
- Main loop / LED remains alive

## Sensor reconnected
Initial issue:
- WHO_AM_I succeeded
- accel/gyro data could be invalid immediately after wake

Fix:
- Added 100 ms delay after successful wake before first measurement

Retest:
- WHO_AM_I = 0x68
- Accel ≈ 1 g on Z
- Gyro near 0 dps
- Firmware recovers normally