/**
 * @file reset.h
 * @author reginald.yang (proyrb@yeah.net)
 * @version 0.1
 * @date 2026-03-18
 * @copyright Copyright (c) 2026
 * @brief 系统复位后，配置程序运行所需的环境，
 * 包括设置栈初始指针、开启mcu内核相关配置、初始化ram数据、
 * 根据条件跳转到指定程序开始执行。
 */

#ifndef _RESET_H_
#define _RESET_H_

#include <stdint.h>
#include <string.h>
#include <stddef.h>
#include "attribute.h"

/**
 * @brief 开启fpu权限。
 */
extern void fpu_init(void);

/**
 * @brief 配置默认的系统时钟树。
 */
extern void rcc_init(void);

/**
 * @brief 开启所有ram区域。
 */
extern void ram_init(void);

/**
 * @brief 默认处理函数。
 */
extern void default_handler(void);

/**
 * @brief 将数据从flash拷贝到ram。
 * @param dest 拷贝数据到该地址。
 * @param src 从该地址拷贝数据。
 * @param len 拷贝字节数.
 */
INLINE static inline void reset_copy_ram_init(const char *dest, const char *src,
                                              size_t len)
{
    memcpy((void *)dest, (void *)src, len);
}

/**
 * @brief 将ram指定区域清零。
 * @param start 起始地址。
 * @param end   结束地址。
 */
INLINE static inline void reset_clear_ram_uninit(const char *start,
                                                 const char *end)
{
    memset((void *)start, 0, (size_t)end - (size_t)start);
}

#endif