#ifndef RINGBUF_H
#define RINGBUF_H

#include <stdint.h>
#include <stdbool.h>

#define RINGBUF_SIZE 64u   // so o cua mang, dung duoc 63 byte

typedef struct{
    uint8_t buf[RINGBUF_SIZE];
    volatile uint16_t head;  // ISR ghi vao day
    volatile uint16_t tail;  // task doc tu day
}ringbuf_t;

void ringbuf_init(ringbuf_t *rb);
bool ringbuf_put(ringbuf_t *rb, uint8_t byte);   // false neu day (byte bi bo)
bool ringbuf_get(ringbuf_t *rb, uint8_t *byte);  // false neu rong
bool ringbuf_is_empty(const ringbuf_t *rb);
bool ringbuf_is_full(const ringbuf_t *rb);

#endif
