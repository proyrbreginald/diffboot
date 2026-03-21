#ifndef _RINGBUFFER_H_
#define _RINGBUFFER_H_

#include <stddef.h>

/* 环形缓冲区结构体 */
struct rt_ringbuffer {
    uint8_t *buffer_ptr;
    uint16_t read_mirror : 1;
    uint16_t read_index : 15;
    uint16_t write_mirror : 1;
    uint16_t write_index : 15;
    uint16_t buffer_size;
};

void rt_ringbuffer_init(struct rt_ringbuffer *rb, uint8_t *pool, uint16_t size);
size_t rt_ringbuffer_put(struct rt_ringbuffer *rb, const uint8_t *ptr,
                         uint16_t length);
size_t rt_ringbuffer_get(struct rt_ringbuffer *rb, uint8_t *ptr,
                         uint16_t length);
size_t rt_ringbuffer_data_len(struct rt_ringbuffer *rb);
void rt_ringbuffer_reset(struct rt_ringbuffer *rb);

#endif