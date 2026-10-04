# UART RX (Block F)

## Luồng dữ liệu

1. PC gửi một byte qua UART2. Ngắt RXNE kích hoạt, `HAL_UART_RxCpltCallback` đẩy byte vào ring buffer (`ringbuf_put`), rồi bật nhận lại bằng `HAL_UART_Receive_IT`.
2. `rx_task` thức dậy mỗi 10 ms và lấy hết byte trong ring bằng `ringbuf_get`.
3. Mỗi byte được in ra qua `log_write` dưới dạng `rx: XX`.

## Thiết kế

- Một producer (ISR ghi `head`) và một consumer (`rx_task` ghi `tail`) nên không cần lock. `head`/`tail` là `volatile`.
- `RINGBUF_SIZE` là 64, sức chứa thực là 63 (chừa 1 ô trống để phân biệt đầy và rỗng).
- `rx_task` cùng mức ưu tiên với `storage_task` (BelowNormal) nên không chen lên trên storage.

## Giới hạn

- Khi ring đầy, byte mới bị bỏ và `rx_lost_cnt` tăng 1. Giá trị này in ra ở dòng heartbeat dưới dạng `lost=`.
- Nếu `rx_task` đọc quá chậm, bên gửi nhồi byte nhanh hơn tốc độ lấy ra nên mất byte.
- Đã kiểm chứng: gửi 112 byte trong lúc `rx_task` ngủ 3 s, nhận 63 byte, `lost=49`.
- Chưa xử lý lỗi UART overrun (ORE) và `HAL_UART_ErrorCallback`.