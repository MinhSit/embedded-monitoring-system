# Định dạng record trên flash

- Kích thước: 32 byte, 8 record / page 256 byte, 128 record / sector 4 KB
- Byte order: little-endian (STM32 và PC x86 đều vậy)
- Field `reserved` phải ghi 0 trước khi tính CRC (nếu không CRC phụ thuộc rác trên stack)

| Offset | Size | Field    |
|-------:|-----:|----------|
| 0      | 4    | seq      |
| 4      | 4    | ts_ms    |
| 8      | 6    | accel[3] |
| 14     | 6    | gyro[3]  |
| 20     | 2    | temp     |
| 22     | 6    | reserved |
| 28     | 4    | crc      |

CRC: crc32_calc trên 28 byte đầu (offset 0 đến 27).

Giới hạn: ghi raw struct bằng memcpy, chỉ đọc lại đúng trên máy cùng endianness và cùng version struct.