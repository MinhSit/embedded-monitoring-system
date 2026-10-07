#include <stdio.h>
#include <stdint.h>
#include "find_blank.h"

/* Flash giả: page < first_blank là data, page >= first_blank là blank. */
typedef struct {
    uint32_t n_pages;
    uint32_t first_blank;
    uint32_t reads;          /* đếm số lần is_blank bị gọi */
    uint32_t out_of_range;   /* đếm lần hỏi page ngoài [0, n_pages) */
    int32_t  fail_at;        /* page nào trả lỗi (-1 = không lỗi) */
} fake_t;

static int fake_is_blank(void *ctx, uint32_t page){
    fake_t *f = (fake_t *)ctx;
    f->reads++;
    if(page >= f->n_pages){
        f->out_of_range++;
        return -1;
    }
    if((int32_t)page == f->fail_at){
        return -1;
    }
    return page >= f->first_blank ? 1 : 0;
}

static int failures = 0;

#define CHECK(cond, ...) do{ if(!(cond)){ failures++; printf("FAIL: "); printf(__VA_ARGS__); printf("\n"); } }while(0)

/* Chạy 1 case thường: kỳ vọng out == k, không đọc ngoài vùng, reads <= max_reads */
static void run_case(uint32_t n, uint32_t k, uint32_t max_reads){
    fake_t f = { n, k, 0, 0, -1 };
    uint32_t out = 0xDEADBEEF;
    int rc = find_first_blank(fake_is_blank, &f, n, &out);
    CHECK(rc == 0, "n=%lu k=%lu rc=%d", (unsigned long)n, (unsigned long)k, rc);
    CHECK(out == k, "n=%lu k=%lu out=%lu", (unsigned long)n, (unsigned long)k, (unsigned long)out);
    CHECK(f.out_of_range == 0, "n=%lu k=%lu doc ngoai vung %lu lan", (unsigned long)n, (unsigned long)k, (unsigned long)f.out_of_range);
    CHECK(f.reads <= max_reads, "n=%lu k=%lu reads=%lu > %lu", (unsigned long)n, (unsigned long)k, (unsigned long)f.reads, (unsigned long)max_reads);
}

int main(void){
    /* T1: vét cạn n = 0..40, mọi k = 0..n (k = n nghĩa là log đầy, không có page blank) */
    for(uint32_t n = 0; n <= 40; n++){
        for(uint32_t k = 0; k <= n; k++){
            run_case(n, k, 7);   /* ceil(log2(41)) = 6, cho dư 1 */
        }
    }

    /* T2: kích thước thật 32752 page, các k biên, tối đa 15 lần đọc */
    uint32_t ks[] = { 0, 1, 2, 100, 16375, 16376, 32750, 32751, 32752 };
    for(size_t i = 0; i < sizeof(ks) / sizeof(ks[0]); i++){
        run_case(32752, ks[i], 15);
    }

    /* T3: lỗi đọc giữa chừng phải trả -1 và không ghi *out */
    {
        fake_t f = { 100, 60, 0, 0, 50 };   /* page 50 là page đầu tiên bị hỏi, trả lỗi */
        uint32_t out = 0xDEADBEEF;
        int rc = find_first_blank(fake_is_blank, &f, 100, &out);
        CHECK(rc == -1, "T3 rc=%d, ky vong -1", rc);
        CHECK(out == 0xDEADBEEF, "T3 *out bi ghi khi loi: %lX", (unsigned long)out);
    }

    if(failures == 0){
        printf("ALL PASS\n");
        return 0;
    }
    printf("%d FAIL\n", failures);
    return 1;
}
