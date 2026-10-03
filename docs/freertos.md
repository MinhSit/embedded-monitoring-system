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