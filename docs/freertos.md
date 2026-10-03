# FreeRTOS design (Block E)

## 1. Tasks
| Task | Priority | Stack (word) | Period | Job |
|---|---|---|---|---|
| heartbeat | osPriorityLow | 256 * 4 = 1024 B | 1000 ms | In ra các thông số mỗi 1s |
| acquisition | osPriorityBelowNormal | 256 * 4 = 1024 B | 100 ms | Đọc data từ mpu6050 và đặt vào queue |
| storage | osPriorityBelowNormal | 256 * 4 = 1024 B | event-driven (chờ queue) | Lấy data từ queue |

## 2. Queue and mutex
- `sample_queue` (16 slot × 24 B): acquisition put, storage get. Tách 2 task để việc ghi flash chậm không làm trễ việc đọc cảm biến.
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
- Khi ghi flash lỗi, `write_addr` không tăng nên storage ghi lại đúng page đó. Flash chỉ đổi bit từ 1 xuống 0 khi program, page đã ghi dở mà không erase thì program lại sẽ verify FAIL mãi (erase chỉ chạy ở đầu sector). Hướng sửa: lỗi thì erase lại sector chứa page đó rồi thử lại, nếu vẫn lỗi thì đánh dấu sector hỏng và nhảy sang sector kế.
- `write_addr` không wrap: chạm 0x800000 (~9 giờ ở 1 page/s) thì `w25q64_page_program` trả ERR_PARAM mãi. Hướng sửa: quay về `LOG_START_ADDR` khi hết dung lượng (đầu sector sẽ tự erase vì đã có điều kiện `% SECTOR_SIZE == 0`).
- `log_write` không dùng được trong ISR. Mutex: ISR không được block, mà task đang giữ mutex không chạy được khi ISR đang chạy nên không bao giờ trả mutex (deadlock); `osMutexAcquire` trong ISR chỉ trả `osErrorISR`. printf: HAL_UART_Transmit polling làm ISR đứng chờ cả chục ms, chặn mọi thứ có priority thấp hơn. Cách đúng: ISR chỉ gắn cờ hoặc đẩy việc vào queue, task lo việc in.
- Mọi printf phải đi qua `log_write`: newlib chưa có `__malloc_lock`, nên mutex của log_write cũng là thứ bảo vệ malloc nội bộ của printf. Heap FreeRTOS (15360 B) chưa được in ra trong heartbeat, chưa biết còn dư bao nhiêu.
- `w25q64_wait_busy` kiểm timeout ngay sau lần đọc BUSY mà không đọc lại. Nếu storage bị task priority cao hơn chiếm CPU lâu hơn timeout (page program 5 ms), hàm báo TIMEOUT giả dù chip đã ghi xong. Hiện chưa xảy ra vì không có task nào cao hơn storage. Hướng sửa: hết timeout thì đọc SR1 thêm một lần rồi mới kết luận.
- Hook tràn stack in bằng `HAL_UART_Transmit`: nếu tràn đúng lúc một task đang in, HAL trả BUSY và không in gì (hệ thống vẫn dừng). Hướng sửa: ghi thẳng thanh ghi UART.