#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "crc32.h"

static void test_check_value(void)
{
    /* giá trị chuẩn của CRC-32 cho chuỗi "123456789" */
    const char *s = "123456789";
    assert(crc32_calc((const uint8_t *)s, strlen(s)) == 0xCBF43926UL);
    printf("test_check_value OK\n");
}

static void test_empty(void)
{
    /* không có dữ liệu: init XOR xorout = 0 */
    assert(crc32_calc((const uint8_t *)"", 0) == 0x00000000UL);
    printf("test_empty OK\n");
}

static void test_single_byte(void)
{
    const uint8_t a = 'a';
    assert(crc32_calc(&a, 1) == 0xE8B7BE43UL);
    printf("test_single_byte OK\n");
}

static void test_detect_bit_flip(void)
{
    uint8_t buf[16];
    for(int i = 0; i < 16; i++){ buf[i] = (uint8_t)i; }
    uint32_t good = crc32_calc(buf, 16);
    buf[7] ^= 0x01;
    assert(crc32_calc(buf, 16) != good);
    printf("test_detect_bit_flip OK\n");
}

int main(void)
{
    test_check_value();
    test_empty();
    test_single_byte();
    test_detect_bit_flip();
    printf("ALL PASS\n");
    return 0;
}