#ifndef FIND_BLANK_H
#define FIND_BLANK_H

#include <stdint.h>

/* Hỏi page số page_index: trả 1 nếu blank, 0 nếu có data, -1 nếu lỗi đọc.
 * ctx = con trỏ tuỳ ý của người gọi (trên MCU là w25q64_t*, trên PC là flash giả). */
typedef int (*page_blank_fn)(void *ctx, uint32_t page_index);

/* Tìm page blank đầu tiên trong [0, n_pages) bằng binary search.
 * Giả định: các page có dạng [data ... data blank ... blank].
 * Ghi kết quả vào *out (n_pages nếu không có page blank, tức log đầy).
 * Trả 0 nếu OK, -1 nếu is_blank báo lỗi. */
int find_first_blank(page_blank_fn is_blank, void *ctx, uint32_t n_pages, uint32_t *out);

#endif