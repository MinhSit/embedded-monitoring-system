# MPU6050 Serial Validation Test

## Test objective

Verify MPU6050 behavior in three conditions:

1. Sensor connected
2. Sensor disconnected
3. Sensor reconnected

The firmware must report sensor communication errors correctly and continue running without hanging.

---

## Case 1 - Sensor connected

```text
WHO_AM_I status: 0
WHO_AM_I: 0x68
Accel status: 0
AX: -348 AY: -96 AZ: 16696
AX: -0.021 g AY: -0.006 g AZ: 1.019 g
Gyro status: 0
GX: -262 GY: 217 GZ: 74
GX: -2.000 dps GY: 1.656 dps GZ: 0.565 dps
==========================================

Embedded Monitoring System
Board    : NUCLEO-F446RE
Firmware : 0.1.0
Build    : Debug
================

Boot OK
```

### Result

**PASS**

- `WHO_AM_I` returned `0x68`
- Accelerometer data is reasonable for a stationary board
- Z-axis acceleration is approximately `1 g`
- Gyroscope readings are close to `0 dps`
- Firmware completed startup successfully

---

## Case 2 - Sensor disconnected

```text
MPU6050 wake failed
WHO_AM_I status: 1
WHO_AM_I read failed
Accel status: 1
Accel read failed
Gyro status: 1
Gyro read failed
================

Embedded Monitoring System
Board    : NUCLEO-F446RE
Firmware : 0.1.0
Build    : Debug
================

Boot OK
```

### Result

**PASS**

- MPU6050 communication failure was detected
- Invalid sensor data was not processed as valid measurements
- Firmware did not hang
- Firmware continued execution and reached `Boot OK`

---

## Case 3 - Sensor reconnected

```text
WHO_AM_I status: 0
WHO_AM_I: 0x68
Accel status: 0
AX: -460 AY: -288 AZ: 16608
AX: -0.028 g AY: -0.018 g AZ: 1.014 g
Gyro status: 0
GX: -287 GY: 152 GZ: 55
GX: -2.191 dps GY: 1.160 dps GZ: 0.420 dps
==========================================

Embedded Monitoring System
Board    : NUCLEO-F446RE
Firmware : 0.1.0
Build    : Debug
================

Boot OK
```

### Result

**PASS**

- Communication recovered after reconnecting the sensor
- `WHO_AM_I` again returned `0x68`
- Accelerometer readings returned to reasonable values
- Gyroscope readings returned to reasonable values
- Firmware completed startup successfully

---

## Final result

**PASS**

The MPU6050 driver successfully handles normal operation, sensor disconnection, and reconnection without causing the firmware to hang.

A short startup delay after waking the MPU6050 is used before the first sensor measurement to allow the device to become ready.