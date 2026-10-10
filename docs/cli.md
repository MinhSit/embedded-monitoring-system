# CLI (Block H)

## Luồng dữ liệu

1. `rx_task` gọi `ringbuf_get` để lấy từng byte ra khỏi ring buffer.
2. Mỗi byte đưa vào `cli_line_feed`, hàm ghép các byte thành một dòng cho tới khi gặp CR/LF.
3. Dòng hoàn chỉnh đi qua `cli_tokenize`, tách thành `argc`/`argv`.
4. `cli_dispatch` so `argv[0]` với `cli_table`. Không khớp thì trả `false` và `rx_task` in WARN "unknown cmd".
5. Khớp thì `cli_dispatch` gọi handler tương ứng (`cmd_help`, `cmd_status`, `cmd_dump`).

## Danh sách lệnh

- `help`: in danh sách lệnh và mô tả.
- `status`: in một dòng log gồm `up`, `drops`, `pg`, `ae`, `fe`, `lost`.
- `dump`: đọc page đầu tiên tại `LOG_START_ADDR` (0x001000) và in 8 record, mỗi record kèm `crc=OK` hoặc `crc=BAD`. Lệnh chưa nhận tham số địa chỉ.

## Điểm yếu đã biết

- (Đã sửa ở v0.9.1) `help` từng dùng `printf` trực tiếp (`cli_print_help`), không qua `log_mutex`. Giờ `cmd_help` in từng dòng bằng `log_write`; `cli_print_help` chỉ còn dùng trong test PC.
- Khi đủ `CLI_MAX_ARGS`, token cuối không được gán NUL, nên `argv` cuối có thể chứa cả phần còn lại của dòng.
- Dòng dài quá 63 ký tự bị cắt, ký tự thừa bị bỏ, lệnh có thể chạy với nội dung thiếu.
- Không echo, không history, không kiểm tra số tham số của lệnh.
- `dump` chỉ đọc page đầu, chưa nhận địa chỉ.
- Boot scan tìm `write_addr` mất khoảng 55 đến 60 s và tăng theo độ đầy flash. Trong lúc quét, queue tràn nên `drops` cao.
- Self-test ghi sector 0 và chạy pattern test ở mỗi lần boot (mòn flash, code test nằm trong đường chạy thật).
- `rx_task` thăm dò mỗi 10 ms, chưa chờ bằng semaphore.
- Bài học H3: ghi đè `cli.c` làm mất `cli_line_feed`. Sau mỗi lần sửa file đã PASS phải build và chạy lại test PC của file đó trước khi commit.