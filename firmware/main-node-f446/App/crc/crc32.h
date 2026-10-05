#ifndef CRC32_H
#define CRC32_H

#include <stdint.h>
#include <stddef.h>

/* CRC-32 (Ethernet/zlib): poly 0xEDB88320 reflected, init 0xFFFFFFFF, xorout 0xFFFFFFFF */
uint32_t crc32_calc(const uint8_t *data, size_t len);

#endif
