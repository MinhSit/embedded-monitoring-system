#include "record.h"
#include "crc/crc32.h"
#include <stddef.h>

int record_check(const record_t *r){
    return r->crc == crc32_calc((const uint8_t *)r, offsetof(record_t, crc));
}
