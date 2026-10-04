#include <assert.h>
#include <stdio.h>
#include "ringbuf.h"

static void test_init_empty(void){
    ringbuf_t rb;
    uint8_t out;
    ringbuf_init(&rb);
    assert(ringbuf_is_empty(&rb) == true);
    assert(ringbuf_is_full(&rb) == false);
    assert(ringbuf_get(&rb, &out) == false);
    printf("test_init_empty OK\n");
}

static void test_fill_until_full(void){
    ringbuf_t rb;
    ringbuf_init(&rb);
    for(uint8_t i = 0; i < RINGBUF_SIZE - 1; i++){
        assert(ringbuf_put(&rb, i) == true);
    }
    assert(ringbuf_is_full(&rb) == true);
    assert(ringbuf_put(&rb, 0xFF) == false);
    printf("test_fill_until_full OK\n");
}

static void test_fifo_order(void){
    ringbuf_t rb;
    uint8_t out;
    ringbuf_init(&rb);
    for(uint8_t i = 0; i < 10; i++){
        assert(ringbuf_put(&rb, (uint8_t)(i + 100)) == true);
    }
    for(uint8_t i = 0; i < 10; i++){
        assert(ringbuf_get(&rb, &out) == true);
        assert(out == (uint8_t)(i + 100));
    }
    assert(ringbuf_is_empty(&rb) == true);
    printf("test_fifo_order OK\n");
}

// 3 vong x 40 byte = 120 byte > 64 o, nen head/tail chac chan quay qua cuoi mang
static void test_wrap_around(void){
    ringbuf_t rb;
    uint8_t out;
    ringbuf_init(&rb);
    for(uint8_t round = 0; round < 3; round++){
        for(uint8_t i = 0; i < 40; i++){
            assert(ringbuf_put(&rb, (uint8_t)(round * 40 + i)) == true);
        }
        for(uint8_t i = 0; i < 40; i++){
            assert(ringbuf_get(&rb, &out) == true);
            assert(out == (uint8_t)(round * 40 + i));
        }
        assert(ringbuf_is_empty(&rb) == true);
    }
    printf("test_wrap_around OK\n");
}

int main(void){
    test_init_empty();
    test_fill_until_full();
    test_fifo_order();
    test_wrap_around();
    return 0;
}
