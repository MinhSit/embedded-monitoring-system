# FreeRTOS design (Block E)

## 1. Tasks
| Task | Priority | Stack (word) | Period | Job |
|---|---|---|---|---|
| heartbeat | osPriorityLow | 256 * 4 = 1024 B | 1000 ms | In thông số mỗi 1 s, kiểm health flags, feed IWDG |
| acquisition | osPriorityBelowNormal | 256 * 4 = 1024 B | 100 ms | Đọc MPU6050 (giữ i2c1_mutex) và đặt sample vào queue |
| storage | osPriorityBelowNormal | 256 * 4 = 1024 B | event-driven (chờ queue, timeout 1 s) | Lấy sample, tạo record + CRC, erase/program/verify (giữ spi1_mutex) |
| rx | osPriorityBelowNormal | 512 * 4 = 2048 B | 10 ms | Lấy byte từ ring buffer, chạy CLI |
| oled | osPriorityBelowNormal | 512 * 4 = 2048 B | 500 ms | Vẽ 4 dòng trạng thái (giữ i2c1_mutex) |

## 2. Queue and mutex
- `sample_queue` (16 slot × 24 B): acquisition put, storage get. Tách 2 task để việc ghi flash chậm không làm trễ việc đọc cảm biến.
- `i2c1_mutex`: MPU6050 (acquisition) và OLED (oled_task) dùng chung bus I2C1.
- `spi1_mutex`: W25Q64 trên SPI1. storage_task giữ suốt erase + program + verify một page (~50 ms khi có erase); lệnh `dump` (rx_task) giữ khi đọc page. Không có mutex thì `dump` có thể kéo CS xuống giữa lúc storage đang ghi.
- `log_mutex`: UART gửi bằng polling (HAL_UART_Transmit chờ xong mới trả về). Nếu task priority cao chen vào lúc đang in, 2 dòng bị trộn hoặc mất (test 5.1). Mutex bắt mỗi lần chỉ một task được in.

## 3. Stack usage
Mỗi task có 1024 B stack cho biến local và các hàm nó gọi. Buffer lớn (page_buf, verify_buf) khai `static` để nằm ở RAM chung. Quy tắc: luôn còn ≥ 25% dư.

## 4. Counters
In trong dòng heartbeat mỗi giây:
- `drops`: sample bị bỏ vì queue đầy. Bình thường = 0.
- `pg`: số page ghi flash thành công và verify OK, tăng 1 mỗi giây.
- `ae`: số lần đọc MPU6050 lỗi. Bình thường = 0.
- `fe`: số page lỗi flash (ghi lỗi, đọc lại lỗi hoặc verify FAIL). Bình thường = 0.

## 5. Known weaknesses
- Task priority cao mà busy-wait (ví dụ HAL_I2C_Mem_Read timeout 100 ms) làm đói task priority thấp. Đã giảm timeout xuống 10 ms.
- Khi MPU6050 lỗi, acquisition in 1 dòng ERROR mỗi 100 ms (10 dòng/giây), chiếm UART, chưa có rate-limit.
- Sau lỗi I2C, bus kẹt ở trạng thái BUSY, mỗi lần đọc vẫn chờ ~25 ms (đo được dt=25). Chưa có reset I2C để hồi phục.
- `w25q64_wait_busy` poll status liên tục, không nhường CPU cho tới khi chip hết busy hoặc hết timeout. Storage và acquisition cùng priority (BelowNormal) nên khi erase (~45 ms) hai task chia đôi CPU, còn heartbeat (priority thấp hơn) bị đói. Hướng sửa: gọi `osDelay(1)` trong vòng poll để nhường CPU; đổi lại driver sẽ phụ thuộc RTOS.
- Khi erase/program trả lỗi, `write_addr` không tăng nên page sau thử lại đúng địa chỉ đó (không erase lại). Nếu lần trước đã ghi dở thì lần thử lại verify FAIL, lúc đó `write_addr` mới tăng. Không có đánh dấu sector hỏng.
- (Đã sửa ở G) Chạm 0x800000 thì dừng ghi, log WARN "flash full" một lần; không quay vòng.
- `log_write` không dùng được trong ISR. Mutex: ISR không được block, mà task đang giữ mutex không chạy được khi ISR đang chạy nên không bao giờ trả mutex (deadlock); `osMutexAcquire` trong ISR chỉ trả `osErrorISR`. printf: HAL_UART_Transmit polling làm ISR đứng chờ cả chục ms, chặn mọi thứ có priority thấp hơn. Cách đúng: ISR chỉ gắn cờ hoặc đẩy việc vào queue, task lo việc in.
- Mọi printf phải đi qua `log_write`: newlib chưa có `__malloc_lock`, nên mutex của log_write cũng là thứ bảo vệ malloc nội bộ của printf. Heap FreeRTOS (15360 B) chưa được in ra trong heartbeat, chưa biết còn dư bao nhiêu.
- (Đã sửa ở v0.9.1) `w25q64_wait_busy` từng kiểm timeout ngay sau lần đọc BUSY: nếu storage bị task khác chiếm CPU lâu hơn timeout thì báo TIMEOUT giả dù chip đã ghi xong. Giờ hàm chốt cờ timeout trước khi đọc SR1, nên luôn có một lần đọc sau mốc timeout rồi mới kết luận.
- Hook tràn stack in bằng `HAL_UART_Transmit`: nếu tràn đúng lúc một task đang in, HAL trả BUSY và không in gì (hệ thống vẫn dừng). Hướng sửa: ghi thẳng thanh ghi UART.