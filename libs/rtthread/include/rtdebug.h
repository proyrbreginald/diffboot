/**
 * @file rtdebug.h
 * @author reginald.yang (proyrb@yeah.net)
 * @version 0.1
 * @date 2026-03-21
 * @copyright Copyright (c) 2026
 * @brief 提供调试日志功能宏定义。
 */

#ifndef _RT_DEBUG_H_
#define _RT_DEBUG_H_

#include <rtconfig.h>
#include "rtdef.h"

#ifdef RT_DEBUG

/**
 * @brief 定义日志级别。
 */
#define DBG_FATAL   0
#define DBG_ERROR   1
#define DBG_WARN    2
#define DBG_INFO    3
#define DBG_DEBUG   4
#define DBG_VERBOSE 5

/**
 * @brief 定义日志标签。
 */
#ifndef DBG_TAG
#define DBG_TAG "DBG"
#endif

#ifdef RT_LOG_ENABLE

/**
 * @brief 定义默认日志级别。
 */
#ifndef DBG_LVL
#define DBG_LVL DBG_WARN
#endif

#ifdef RT_DEBUG_COLOR
/**
 * @brief 定义前景色。
 */
#define DBG_FG_BLACK    "30"
#define DBG_FG_RED      "31"
#define DBG_FG_GREEN    "32"
#define DBG_FG_YELLOW   "33"
#define DBG_FG_BLUE     "34"
#define DBG_FG_PURPLE   "35"
#define DBG_FG_CYAN     "36"
#define DBG_FG_WHITE    "37"
#define DBG_FG_H_BLACK  "90"
#define DBG_FG_H_RED    "91"
#define DBG_FG_H_GREEN  "92"
#define DBG_FG_H_YELLOW "93"
#define DBG_FG_H_BLUE   "94"
#define DBG_FG_H_PURPLE "95"
#define DBG_FG_H_CYAN   "96"
#define DBG_FG_H_WHITE  "97"

/**
 * @brief 定义背景色。
 */
#define DBG_BG_BLACK  "40"
#define DBG_BG_RED    "41"
#define DBG_BG_GREEN  "42"
#define DBG_BG_YELLOW "43"
#define DBG_BG_BLUE   "44"
#define DBG_BG_PURPLE "45"
#define DBG_BG_CYAN   "46"
#define DBG_BG_WHITE  "47"

/**
 * @brief 定义输出颜色控制。
 */
#define _DBG_LOG_HDR(color_fg, color_bg) "\033[" color_fg ";" color_bg "m"
#define _DBG_LOG_X_END                   "\033[0m\n"
#else
#define _DBG_LOG_HDR(lvl_name, color_fg) "[" lvl_name "/" DBG_TAG "] "
#define _DBG_LOG_X_END                   "\n"
#endif

/**
 * @brief 输出定位日志。
 */
#define dbg_here                                                               \
    if ((DBG_LVL) <= DBG_DEBUG) {                                              \
        rt_kprintf(DBG_TAG " Here %s:%d\n", __FUNCTION__, __LINE__);           \
    }

/**
 * @brief 输出单行日志。
 */
#define dbg_log_line(lvl, color_fg, color_bg, fmt, ...)                        \
    do {                                                                       \
        rt_kprintf(_DBG_LOG_HDR(color_fg, color_bg) "[" lvl "/" DBG_TAG ":%d"  \
                                                    "] " fmt _DBG_LOG_X_END,   \
                   __LINE__, ##__VA_ARGS__);                                   \
    } while (0)

/**
 * @brief 输出原数据日志。
 */
#define dbg_raw(...) rt_kprintf(__VA_ARGS__);
#else
#define dbg_log(level, fmt, ...)
#define dbg_here
#define dbg_enter
#define dbg_exit
#define dbg_log_line(lvl, color_fg, fmt, ...)
#define dbg_raw(...)
#endif

#if (DBG_LVL >= DBG_VERBOSE)
#define LOG_V(fmt, ...)                                                        \
    dbg_log_line("V", DBG_FG_H_BLACK, DBG_BG_BLACK, fmt, ##__VA_ARGS__)
#else
#define LOG_V(...)
#endif

#if (DBG_LVL >= DBG_DEBUG)
#define LOG_D(fmt, ...)                                                        \
    dbg_log_line("D", DBG_FG_BLUE, DBG_BG_BLACK, fmt, ##__VA_ARGS__)
#else
#define LOG_D(...)
#endif

#if (DBG_LVL >= DBG_INFO)
#define LOG_I(fmt, ...)                                                        \
    dbg_log_line("I", DBG_FG_GREEN, DBG_BG_BLACK, fmt, ##__VA_ARGS__)
#else
#define LOG_I(...)
#endif

#if (DBG_LVL >= DBG_WARN)
#define LOG_W(fmt, ...)                                                        \
    dbg_log_line("W", DBG_FG_YELLOW, DBG_BG_BLACK, fmt, ##__VA_ARGS__)
#else
#define LOG_W(...)
#endif

#if (DBG_LVL >= DBG_ERROR)
#define LOG_E(fmt, ...)                                                        \
    dbg_log_line("E", DBG_FG_RED, DBG_BG_BLACK, fmt, ##__VA_ARGS__)
#else
#define LOG_E(...)
#endif

#if (DBG_LVL >= DBG_FATAL)
#define LOG_F(fmt, ...)                                                        \
    dbg_log_line("F", DBG_FG_WHITE, DBG_BG_RED, fmt, ##__VA_ARGS__)
#else
#define LOG_F(...)
#endif

#define LOG_RAW(...) dbg_raw(__VA_ARGS__)
#define LOG_HEX(name, width, buf, size)

/**
 * @brief 定义开启哪些调试。
 */
#ifndef RT_DEBUG_MEM
#define RT_DEBUG_MEM 0
#endif
#ifndef RT_DEBUG_MEMHEAP
#define RT_DEBUG_MEMHEAP 0
#endif
#ifndef RT_DEBUG_MODULE
#define RT_DEBUG_MODULE 0
#endif
#ifndef RT_DEBUG_SCHEDULER
#define RT_DEBUG_SCHEDULER 0
#endif
#ifndef RT_DEBUG_SLAB
#define RT_DEBUG_SLAB 0
#endif
#ifndef RT_DEBUG_THREAD
#define RT_DEBUG_THREAD 0
#endif
#ifndef RT_DEBUG_TIMER
#define RT_DEBUG_TIMER 0
#endif
#ifndef RT_DEBUG_IRQ
#define RT_DEBUG_IRQ 0
#endif
#ifndef RT_DEBUG_IPC
#define RT_DEBUG_IPC 0
#endif
#ifndef RT_DEBUG_DEVICE
#define RT_DEBUG_DEVICE 1
#endif
#ifndef RT_DEBUG_CONTEXT_CHECK
#define RT_DEBUG_CONTEXT_CHECK 1
#endif

/**
 * @brief 定义调试选项的日志输出。
 */
#define RT_DEBUG_LOG(type, message)                                            \
    do {                                                                       \
        if (type) { rt_kprintf message; }                                      \
    } while (0)

/**
 * @brief 定义断言接口与宏。
 */
extern void (*rt_assert_hook)(const char *ex, const char *func, size_t line);
void rt_assert_set_hook(void (*hook)(const char *ex, const char *func,
                                     size_t line));
void rt_assert_handler(const char *ex, const char *func, size_t line);
#define RT_ASSERT(EX)                                                          \
    if (!(EX)) { rt_assert_handler(#EX, __FUNCTION__, __LINE__); }

#if RT_DEBUG_CONTEXT_CHECK
/**
 * @brief 检查当前是否处于中断上下文中。
 */
#define RT_DEBUG_NOT_IN_INTERRUPT                                              \
    do {                                                                       \
        rt_base_t level;                                                       \
        level = rt_hw_interrupt_disable();                                     \
        if (rt_interrupt_get_nest() != 0) {                                    \
            rt_kprintf("Function[%s] shall not be used in ISR\n",              \
                       __FUNCTION__);                                          \
            RT_ASSERT(0)                                                       \
        }                                                                      \
        rt_hw_interrupt_enable(level);                                         \
    } while (0)

/**
 * @brief 检查当前是否处于线程上下文中。
 */
#define RT_DEBUG_IN_THREAD_CONTEXT                                             \
    do {                                                                       \
        rt_base_t level;                                                       \
        level = rt_hw_interrupt_disable();                                     \
        if (rt_thread_self() == NULL) {                                        \
            rt_kprintf(                                                        \
                "Function[%s] shall not be used before scheduler start\n",     \
                __FUNCTION__);                                                 \
            RT_ASSERT(0)                                                       \
        }                                                                      \
        RT_DEBUG_NOT_IN_INTERRUPT;                                             \
        rt_hw_interrupt_enable(level);                                         \
    } while (0)

/**
 * @brief 检查当前调度器是否处于解锁状态。
 */
#define RT_DEBUG_SCHEDULER_AVAILABLE(need_check)                               \
    do {                                                                       \
        if (need_check) {                                                      \
            rt_base_t level;                                                   \
            level = rt_hw_interrupt_disable();                                 \
            if (rt_critical_level() != 0) {                                    \
                rt_kprintf("Function[%s]: scheduler is not available\n",       \
                           __FUNCTION__);                                      \
                RT_ASSERT(0)                                                   \
            }                                                                  \
            RT_DEBUG_IN_THREAD_CONTEXT;                                        \
            rt_hw_interrupt_enable(level);                                     \
        }                                                                      \
    } while (0)
#else
#define RT_DEBUG_NOT_IN_INTERRUPT
#define RT_DEBUG_IN_THREAD_CONTEXT
#define RT_DEBUG_SCHEDULER_AVAILABLE(need_check)
#endif
#else
#define RT_ASSERT(EX)
#define RT_DEBUG_LOG(type, message)
#define RT_DEBUG_NOT_IN_INTERRUPT
#define RT_DEBUG_IN_THREAD_CONTEXT
#define RT_DEBUG_SCHEDULER_AVAILABLE(need_check)
#endif

#endif