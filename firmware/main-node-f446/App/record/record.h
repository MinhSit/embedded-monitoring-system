#ifndef RECORD_H
#define RECORD_H

#include <stdint.h>

#define RECORD_SIZE 32U

typedef struct {
    uint32_t seq;      /* số thứ tự record, tăng dần */
    uint32_t ts_ms;    /* thời điểm lấy mẫu */
    int16_t  accel[3]; /* ax, ay, az */
    int16_t  gyro[3];  /* gx, gy, gz */
    int16_t  temp;
    int16_t  reserved[3];
    uint32_t crc;      /* CRC-32 của 28 byte đứng trước */
} record_t;

_Static_assert(sizeof(record_t) == RECORD_SIZE, "record_t must be 32 bytes");

#endif
