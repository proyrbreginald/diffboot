#ifndef _RINGBUFFER_H_
#define _RINGBUFFER_H_

#include <rtthread.h>

/* 环形缓冲区结构体 */
struct rt_ringbuffer {
    rt_uint8_t *buffer_ptr;
    rt_uint16_t read_mirror : 1;
    rt_uint16_t read_index : 15;
    rt_uint16_t write_mirror : 1;
    rt_uint16_t write_index : 15;
    rt_uint16_t buffer_size;
};

void rt_ringbuffer_init(struct rt_ringbuffer *rb, rt_uint8_t *pool,
                        rt_uint16_t size);
rt_size_t rt_ringbuffer_put(struct rt_ringbuffer *rb, const rt_uint8_t *ptr,
                            rt_uint16_t length);
rt_size_t rt_ringbuffer_get(struct rt_ringbuffer *rb, rt_uint8_t *ptr,
                            rt_uint16_t length);
rt_size_t rt_ringbuffer_data_len(struct rt_ringbuffer *rb);
void rt_ringbuffer_reset(struct rt_ringbuffer *rb);

#endif