#include "find_blank.h"

int find_first_blank(page_blank_fn is_blank, void *ctx, uint32_t n_pages, uint32_t *out){
    uint32_t lo = 0, hi = n_pages;
    while(lo < hi){
        uint32_t mid = (hi - lo) / 2 + lo;
        int r = is_blank(ctx, mid);
        if(r < 0){
            return -1;
        }
        if(r == 1){
            hi = mid;
        }
        else{
            lo = mid + 1;
        }
    }
    *out = lo;
    return 0;
}