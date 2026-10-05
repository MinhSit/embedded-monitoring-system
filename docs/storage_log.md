# Storage log (Block G)

Layout record: xem docs/record_format.md.

## 1. Luồng ghi
- acquisition_task put sample vào sample_queue (timeout 0, đầy thì tăng sample_drop_cnt); storage_task get ra và đổi thành record_t 32 B kèm CRC-32.
- Đủ 8 record (RECORDS_PER_PAGE) thì ghi 1 page 256 B; nếu write_addr nằm đầu sector (chia hết SECTOR_SIZE) thì erase sector trước khi program.
- Sau program, đọc lại page vào verify_buf bằng read_data rồi memcmp với page_buf; lệch thì ghi log ERROR và tăng flash_err_cnt.
- Khi write_addr >= W25Q64_CAPACITY, storage_task không erase/program nữa: log WARN "flash full, stop logging" đúng 1 lần và bỏ các page sau. Không quay vòng.

## 2. Luồng đọc và nối log lúc boot
- storage_find_write_addr quét từ LOG_START_ADDR (0x001000), mỗi page đọc 256 B bằng storage_page_is_blank; page đầu tiên toàn 0xFF là chỗ ghi tiếp theo, log "resume addr=...".
- Không có page trống thì trả W25Q64_CAPACITY (log đầy); lỗi đọc flash thì trả LOG_START_ADDR.
- storage_scan_page đọc 1 page vào record_t[8], gọi record_check (CRC-32 trên 28 B đầu) cho từng record và đếm record hỏng, log "scan addr=... bad=N".
- Record_check chỉ có ý nghĩa với dữ liệu đọc từ flash lúc boot, không dùng ở đường ghi (ở đó đã có verify bằng memcmp).

## 3. Điểm yếu đã biết
- Quét boot chậm: storage_find_write_addr đọc từng page nên tốn ~2.8 s với 107 page và ~4-5 s với ~620 page, tăng theo độ dài log. Trong lúc quét sample_queue tràn (drops tăng ~30). Hướng sửa: lưu write_index ở metadata sector 0 hoặc tìm nhị phân.
- seq về 0 mỗi lần boot vì chưa có metadata (boot_count, seq cuối). Log cũ và mới có seq trùng nhau.
- Boot self-test (erase sector 0 + pattern test) chạy mỗi lần boot: test code nằm trong đường chạy thật và làm mòn flash.
- Flash đầy thì dừng ghi, không quay vòng (ghi đè log cũ cần metadata để biết đâu là record cũ nhất).
- Lỗi ghi (erase/program fail) không tăng write_addr: page sau thử lại cùng địa chỉ mà không erase lại, nên dễ thành verify FAIL liên tiếp.
- Boot chỉ gọi storage_scan_page cho page đầu tiên (LOG_START_ADDR), không kiểm CRC toàn bộ log.
- record_t được ghi thẳng bằng struct thô (little-endian của CPU), không phải định dạng độc lập kiến trúc.