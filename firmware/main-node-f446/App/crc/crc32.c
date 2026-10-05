#include "crc32.h"

#define CRC32_POLY  0xEDB88320UL  /* đa thức đã đảo bit (reflected) */

uint32_t crc32_calc(const uint8_t *data, size_t len)
{
    uint32_t crc = 0xFFFFFFFFUL;
    for(size_t i = 0; i < len; i++){
        crc ^= data[i];
        for(size_t bit = 0; bit < 8; bit++){
            if(crc & 1UL){
                  crc = (crc >> 1) ^ CRC32_POLY;
              }
              else{
                  crc = crc >> 1;
              }
        }
    }
    return crc ^ 0xFFFFFFFFUL;
}
