#include <mcu.h>
#include <ringbuffer.h>
#include <rtthread.h>
#include <stdint.h>
#include <string.h>

void rt_ringbuffer_init(struct rt_ringbuffer *rb, uint8_t *pool, uint16_t size)
{
    rb->buffer_ptr = pool;
    rb->buffer_size = size;
    rt_ringbuffer_reset(rb);
}

void rt_ringbuffer_reset(struct rt_ringbuffer *rb)
{
    rb->read_mirror = 0;
    rb->read_index = 0;
    rb->write_mirror = 0;
    rb->write_index = 0;
}

ITCM size_t rt_ringbuffer_data_len(struct rt_ringbuffer *rb)
{
    if (rb->read_index == rb->write_index)
    {
        return (rb->read_mirror == rb->write_mirror) ? 0 : rb->buffer_size;
    }
    if (rb->write_index > rb->read_index)
        return rb->write_index - rb->read_index;
    else
        return rb->buffer_size - (rb->read_index - rb->write_index);
}

ITCM size_t rt_ringbuffer_put(struct rt_ringbuffer *rb, const uint8_t *ptr,
                              uint16_t length)
{
    uint16_t size;
    uint16_t free_len = rb->buffer_size - rt_ringbuffer_data_len(rb);

    if (free_len == 0)
        return 0;
    if (length > free_len)
        length = free_len;

    size = rb->buffer_size - rb->write_index;
    if (size > length)
    {
        memcpy(&rb->buffer_ptr[rb->write_index], ptr, length);
        rb->write_index += length;
    }
    else
    {
        memcpy(&rb->buffer_ptr[rb->write_index], ptr, size);
        memcpy(&rb->buffer_ptr[0], &ptr[size], length - size);
        rb->write_index = length - size;
        rb->write_mirror = ~rb->write_mirror;
    }
    return length;
}

ITCM size_t rt_ringbuffer_get(struct rt_ringbuffer *rb, uint8_t *ptr,
                              uint16_t length)
{
    uint16_t size;
    uint16_t data_len = rt_ringbuffer_data_len(rb);

    if (data_len == 0)
        return 0;
    if (length > data_len)
        length = data_len;

    size = rb->buffer_size - rb->read_index;
    if (size > length)
    {
        memcpy(ptr, &rb->buffer_ptr[rb->read_index], length);
        rb->read_index += length;
    }
    else
    {
        memcpy(ptr, &rb->buffer_ptr[rb->read_index], size);
        memcpy(&ptr[size], &rb->buffer_ptr[0], length - size);
        rb->read_index = length - size;
        rb->read_mirror = ~rb->read_mirror;
    }
    return length;
}