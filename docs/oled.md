## 1. Mục tiêu và phần cứng

OLED là tính năng hỗ trợ: hiển thị nhanh trạng thái hệ thống (chạy, tần số lấy
mẫu, flash, số sample bị rơi, gia tốc trục X) để theo dõi mà không cần mở
terminal. Đây không phải trọng tâm của dự án.

- Module: GM009605 v4.3, 0.96", 128x64, chip SSD1306.
- Giao tiếp: I2C1 (PB8 = SCL, PB9 = SDA), địa chỉ 7-bit 0x3C, tốc độ 100 kHz.
- Dùng chung bus I2C1 với MPU6050 (địa chỉ 0x68), nên cần mutex (xem mục 3).

## 2. Driver

File: `firmware/main-node-f446/App/drivers/ssd1306/ssd1306.[ch]`,
font: `firmware/main-node-f446/App/drivers/ssd1306/font5x7.h`.

| Hàm | Tác dụng |
|---|---|
| `ssd1306_write_cmd` | Gửi 1 lệnh tới SSD1306 (byte điều khiển 0x00 + lệnh). |
| `ssd1306_write_data` | Gửi tối đa 128 byte dữ liệu pixel (byte điều khiển 0x40). |
| `ssd1306_init` | Gửi chuỗi khởi tạo 26 byte (128x64, charge pump bật, COM pins 0xDA = 0x12). |
| `ssd1306_clear` | Xóa toàn màn hình: đặt cửa sổ 128 cột x 8 page, ghi 8 lần 128 byte 0. |
| `ssd1306_set_pos` | Đặt con trỏ ghi tại (cột, page). |
| `ssd1306_write_char` | Vẽ 1 ký tự 5x7 (5 cột + 1 cột trống). |
| `ssd1306_write_str` | Vẽ chuỗi ký tự 5x7. |
| `ssd1306_write_char2x` | Vẽ 1 ký tự phóng đại 2x (12 cột x 2 page); `stretch_v` nhân đôi các bit theo chiều dọc. |
| `ssd1306_write_str2x` | Vẽ chuỗi chữ 2x, tối đa 10 ký tự mỗi dòng (cột 0..116). |

Font: `font5x7.h` (ASCII 32..126, 5 byte mỗi ký tự, bit0 = hàng trên cùng) là bảng
font 5x7 cổ điển, do AI (Claude) tạo ra làm dữ liệu, không sao chép từ thư viện nào.
Phần driver (ssd1306.c/.h) do tôi tự viết, có hướng dẫn các bước và review từ AI
(Claude) trong lúc học, tra cứu syntax; không dùng thư viện OLED bên ngoài.

## 3. oled_task và mutex I2C1

OLED và MPU6050 dùng chung bus I2C1, nên mọi truy cập bus đều qua `i2c1_mutex`
(CMSIS-RTOS2 mutex, tạo trong `USER CODE BEGIN RTOS_MUTEX`):

- `acquisition_task` giữ mutex chỉ quanh `mpu6050_read_raw`.
- `oled_task` (priority BelowNormal, stack 512 word, chu kỳ 500 ms) chuẩn bị 4
  chuỗi, rồi giữ mutex một lần để ghi 4 dòng.
- Mutex của FreeRTOS có priority inheritance, nên acquisition (Normal) không bị
  oled_task (BelowNormal) chặn lâu.

Bố cục 4 dòng, chữ 2x, đúng 10 ký tự mỗi dòng để ghi đè hết chữ cũ:

| Page | Nội dung | Nguồn |
|---|---|---|
| 0 | `RUN 10HZ` | `1000 / ACQ_PERIOD_MS` (SYS + SAMPLE gộp một dòng) |
| 2 | `FLASH:OK` / `FLASH:ERR` / `FLASH:FULL` | `flash_err_cnt`, `flash_full` |
| 4 | `DROP:n` | `sample_drop_cnt` |
| 6 | `AX:n` | `last_ax` (accel_x thô, int16) |

## 4. Test

Test trên board NUCLEO-F446RE, firmware chạy bình thường:

- OLED hiện đủ 4 dòng (`RUN 10HZ`, `FLASH:OK`, `DROP:0`, `AX:n`), AX đổi theo thời gian thực khi nghiêng board.
- Tần số lấy mẫu thực tế là 10 Hz (`ACQ_PERIOD_MS` = 100 ms), không phải 100 Hz như ví dụ trong spec; OLED hiển thị đúng giá trị thực.
- `drops=0`, `acq` ổn định, không có `MPU6050 read failed`, IWDG không reset.
- Chưa test trên board hai trạng thái `FLASH:FULL` và `FLASH:ERR`
  (chỉ review code).

## 5. Hạn chế đã biết

- Chữ 1x (5x7) không đọc được trên panel này; nghi nguyên nhân là hàng trên và
  dưới bị cắt (chưa kiểm chứng). Vì vậy toàn bộ chữ dùng 2x, tối đa 10 ký tự
  mỗi dòng. `ssd1306_write_str` (1x) vẫn còn trong driver nhưng không dùng.
- Lệnh `0xDA` giữ 0x12; thử 0x02 làm màn hình hỏng.
- Trước khi thêm bus recovery, sau khi nạp code (debugger reset MCU giữa lúc đang
  truyền I2C) đôi khi `MPU6050 init FAIL` lặp mãi, rút nguồn thì hết. Nghi bus I2C
  bị kẹt (slave giữ SDA thấp). Đã thêm `i2c1_bus_recovery()` (DeInit I2C, đập SCL
  tối đa 9 xung cho tới khi SDA lên cao, rồi `MX_I2C1_Init()`), gọi đầu
  `USER CODE BEGIN 2`. Sau đó 7 lần boot/nạp liên tiếp không còn lỗi; mẫu nhỏ, chưa
  chứng minh hết hẳn.
- `FLASH:ERR` chỉ phản ánh `flash_err_cnt > 0`, `FLASH:FULL` ưu tiên hơn `ERR`
  khi có cả hai.
