# Storage log (Block G)

Layout record: xem docs/record_format.md.

## 1. Luồng ghi
- acquisition_task put sample vào sample_queue (timeout 0, đầy thì tăng sample_drop_cnt); storage_task get ra và đổi thành record_t 32 B kèm CRC-32.
- Đủ 8 record (RECORDS_PER_PAGE) thì ghi 1 page 256 B; nếu write_addr nằm đầu sector (chia hết SECTOR_SIZE) thì erase sector trước khi program.
- Sau program, đọc lại page vào verify_buf bằng read_data rồi memcmp với page_buf; lệch thì ghi log ERROR và tăng flash_err_cnt.
- Khi write_addr >= W25Q64_CAPACITY, storage_task không erase/program nữa: log WARN "flash full, stop logging" đúng 1 lần và bỏ các page sau. Không quay vòng.

## 2. Luồng đọc và nối log lúc boot
- storage_find_write_addr tìm page trống đầu tiên bằng binary search (find_first_blank, App/findblank). Log chỉ ghi nối tiếp nên các page luôn có dạng [data ... data blank ... blank]; chỉ có một điểm chuyển. Tối đa 15 lần đọc cho 32752 page, không tăng theo độ dài log. Kết quả log "resume addr=...".
- Log đầy (không có page blank) thì địa chỉ = W25Q64_CAPACITY. Lỗi đọc flash thì hàm trả -1: storage_task bật `log_disabled`, tăng `flash_err_cnt` (OLED hiện FLASH:ERR), log ERROR "boot scan failed, logging disabled" và không ghi page nào, thay vì đoán địa chỉ rồi ghi đè log cũ (trước v0.9.1 resume về LOG_START_ADDR).
- Boot scan và mọi thao tác ghi page giữ `spi1_mutex` để lệnh `dump` không chen vào.
- Giả định: không có "lỗ" (page blank nằm giữa vùng data). Mất điện giữa chừng khi ghi một page vẫn giữ giả định này vì page được ghi nối tiếp.
- storage_scan_page đọc 1 page vào record_t[8], gọi record_check (CRC-32 trên 28 B đầu) cho từng record và đếm record hỏng, log "scan addr=... bad=N".
- Record_check chỉ có ý nghĩa với dữ liệu đọc từ flash lúc boot, không dùng ở đường ghi (ở đó đã có verify bằng memcmp).

## 3. Điểm yếu đã biết
- (Đã sửa ở G7) Quét boot tuyến tính: từng tốn ~141 s với log tới ~0x531B00, tăng mỗi lần boot và làm mất ~1400 mẫu. Sau khi dùng binary search: resume addr=0x572100 tại t = 500 ms, drops=0.
- seq về 0 mỗi lần boot vì chưa có metadata (boot_count, seq cuối). Log cũ và mới có seq trùng nhau.
- Boot self-test (erase sector 0 + pattern test) chạy mỗi lần boot: test code nằm trong đường chạy thật và làm mòn flash.
- Flash đầy thì dừng ghi, không quay vòng (ghi đè log cũ cần metadata để biết đâu là record cũ nhất).
- Lỗi ghi (erase/program fail) không tăng write_addr: page sau thử lại cùng địa chỉ mà không erase lại; nếu page đã ghi dở thì lần thử lại verify FAIL rồi mới sang page kế.
- Boot chỉ gọi storage_scan_page cho page đầu tiên (LOG_START_ADDR), không kiểm CRC toàn bộ log.
- record_t được ghi thẳng bằng struct thô (little-endian của CPU), không phải định dạng độc lập kiến trúc.