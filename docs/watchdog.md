# Watchdog (Block I)

## Thiết kế

- `acquisition_task`, `rx_task` và `storage_task` mỗi vòng lặp gọi `osEventFlagsSet` để báo alive (bit `HEALTH_TASK_ACQ`, `HEALTH_TASK_STORAGE`, `HEALTH_TASK_RX`).
- `heartbeat_task` là monitor: mỗi chu kỳ đọc và xóa cờ bằng `osEventFlagsClear`, nếu cả 3 bit đều có (`health_ok`) thì mới ghi `0xAAAA` vào `IWDG->KR`. Thiếu bất kỳ task nào thì không feed và IWDG tự reset MCU.
- IWDG bật sau khi boot scan xong, ngay trong `heartbeat_task`.
- `storage_task` chờ queue với timeout 1 s (không chờ vô hạn) để khi queue rỗng vẫn lặp và báo alive.
- Khi boot, `print_reset_reason` đọc `RCC->CSR` và in lý do reset, xét `IWDGRST` trước vì `PINRST` luôn bật cùng `IWDGRST`.

## Giá trị IWDG

- `IWDG->PR = 4` (chia 64), `IWDG->RLR = 1999`, clock LSI danh định 32 kHz.
- Tần số đếm = 32000 / 64 = 500 Hz, timeout = (RLR + 1) / 500 = 4 s.

## Bằng chứng (test I5)

- Cho `acquisition_task` treo có chủ ý: log in `health fail alive=06` 3 lần (bit1 và bit2 có, bit0 thiếu, đúng task acquisition).
- Khoảng 4 s sau lần feed cuối, MCU reset. Boot lên in `reset: IWDGRST` và `reset: PINRST`.
- Code treo thử đã xóa, không còn trong repo.

## Điểm yếu đã biết

- Boot scan tìm `write_addr` mất khoảng 141 s (tăng mỗi boot theo độ đầy flash). IWDG chỉ bật sau scan nên cửa sổ này không được bảo vệ.
- Stack còn trống của `heartbeat_task` khoảng 308 B trên 1024 B, khá sát vì task vừa in log vừa là monitor.
- `heartbeat_task` vừa in log vừa là monitor: nếu `log_write` kẹt thì không feed và MCU reset. Chấp nhận được vì kẹt log cũng là lỗi cần reset.
- Chỉ phát hiện task không chạy, không phát hiện task vẫn chạy nhưng logic sai (ví dụ báo alive dù đọc cảm biến lỗi).
- LSI chỉ là 32 kHz danh định (datasheet cho khoảng 17 đến 47 kHz), nên timeout 4 s thực tế có thể lệch.
- `PINRST` luôn bật cùng `IWDGRST` nên phải xét `IWDGRST` trước khi kết luận lý do reset.
- Chưa lưu lý do reset qua các lần boot, sau reset chỉ có một dòng log.