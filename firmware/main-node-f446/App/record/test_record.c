#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include "record.h"
#include "crc32.h"

int main(void){
    record_t r;
    memset(&r, 0, sizeof(r));
    r.seq = 1;
    r.ts_ms = 100;
    r.accel[2] = 16384;
    r.crc = crc32_calc((const uint8_t *)&r, offsetof(record_t, crc));

    /* 1: record nguyên vẹn -> 1 */
    printf("T1 %s\n", record_check(&r) == 1 ? "PASS" : "FAIL");

    /* 2: lật 1 bit trong dữ liệu -> 0 */
    r.accel[2] ^= 1;
    printf("T2 %s\n", record_check(&r) == 0 ? "PASS" : "FAIL");
    r.accel[2] ^= 1;

    /* 3: lật 1 bit trong crc -> 0 */
    r.crc ^= 0x80000000UL;
    printf("T3 %s\n", record_check(&r) == 0 ? "PASS" : "FAIL");

    /* 4: sector xoá chưa ghi (toàn 0xFF) -> 0 */
    memset(&r, 0xFF, sizeof(r));
    printf("T4 %s\n", record_check(&r) == 0 ? "PASS" : "FAIL");
    return 0;
}