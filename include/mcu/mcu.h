/**
 * @file mcu.h
 * @author reginald.yang (proyrb@yeah.net)
 * @version 0.1
 * @date 2026-03-17
 * @copyright Copyright (c) 2026
 * @brief 作为diffboot的底层头文件，向上层提供硬件与软件定义。
 */

#ifndef _MCU_H_
#define _MCU_H_

#include "macro.h" //!< 导入mcu相关宏定义
#include <stdint.h>

extern const uint8_t _stack_start;    //!< 初始栈地址(从高地址向底地址生长)
extern const uint8_t _stack_min_size; //!< 栈最小大小
extern const uint8_t _heap_start;     //!< 紧跟在数据段之后
extern const uint8_t _heap_end;       //!< 堆结束地址(紧靠栈且不覆盖最小栈空间)

#endif