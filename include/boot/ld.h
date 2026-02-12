#ifndef _LD_H_
#define _LD_H_

#include "../mcu/partition.h"
#include <stdint.h>

// 栈底起始地址
extern const uint8_t stack_start;

// 栈最小大小
extern const uint8_t stack_size;

// 堆起始地址
extern const uint8_t _heap_start;

// 堆结束地址
extern const uint8_t heap_end;

#endif