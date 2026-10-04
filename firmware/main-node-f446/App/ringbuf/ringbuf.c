#include "ringbuf.h"

void ringbuf_init(ringbuf_t *rb){
    rb->head = 0;
    rb->tail = 0;
}

bool ringbuf_is_empty(const ringbuf_t *rb){
    return rb->head == rb->tail;
}

bool ringbuf_is_full(const ringbuf_t *rb){
    return (rb->head + 1) % RINGBUF_SIZE == rb->tail;
}

bool ringbuf_put(ringbuf_t *rb, uint8_t byte){
    if(ringbuf_is_full(rb)){
        return false;
    }
    rb->buf[rb->head] = byte;
    rb->head = (rb->head + 1) % RINGBUF_SIZE;
    return true;
}

bool ringbuf_get(ringbuf_t *rb, uint8_t *byte){
    if(ringbuf_is_empty(rb)){
        return false;
    }
    *byte = rb->buf[rb->tail];
    rb->tail = (rb->tail + 1)% RINGBUF_SIZE;
    return true;
}
