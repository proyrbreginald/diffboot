#ifndef _RT_DBG_H_
#define _RT_DBG_H_

#include <rtconfig.h>

// 定义RT_DEBUG宏来开启日志调试功能
#if defined(RT_DEBUG) && !defined(DBG_ENABLE)
#define DBG_ENABLE
#endif

// 定义RT_DEBUG_COLOR宏来开启附带颜色的日志
#if defined(RT_DEBUG_COLOR) && !defined(DBG_COLOR)
#define DBG_COLOR
#endif

// 定义日志级别
#define DBG_ERROR 0
#define DBG_WARN  1
#define DBG_NEWS  2
#define DBG_INFO  3
#define DBG_LOG   4

// 定义日志标签
#ifndef DBG_TAG
#define DBG_TAG "DBG"
#endif

// 定义DBG_ENABLE宏来开启日志输出宏
#ifdef DBG_ENABLE

// 默认日志级别
#ifndef DBG_LVL
#define DBG_LVL DBG_WARN
#endif

// 定义DBG_COLOR宏来开启日志颜色输出宏
#ifdef DBG_COLOR

/*
 * 前景色（符号颜色）
 * BLACK    30
 * RED      31
 * GREEN    32
 * YELLOW   33
 * BLUE     34
 * PURPLE   35
 * CYAN     36
 * WHITE    37
 */
#define _DBG_COLOR(n) rt_kprintf("\033[" #n "m")
#define _DBG_LOG_HDR(lvl_name, color_n)                                        \
    rt_kprintf("\033[" #color_n "m[" lvl_name "/" DBG_TAG "] ")
#define _DBG_LOG_X_END rt_kprintf("\033[0m\n")

#else

#define _DBG_COLOR(n)
#define _DBG_LOG_HDR(lvl_name, color_n)                                        \
    rt_kprintf("[" lvl_name "/" DBG_TAG "] ")
#define _DBG_LOG_X_END rt_kprintf("\n")

#endif

// 定义普通日志输出宏
#define dbg_log(level, fmt, ...)                                               \
    if ((level) <= DBG_LVL) {                                                  \
        switch (level) {                                                       \
            case DBG_ERROR:                                                    \
                _DBG_LOG_HDR("E", 31);                                         \
                break;                                                         \
            case DBG_WARN:                                                     \
                _DBG_LOG_HDR("W", 33);                                         \
                break;                                                         \
            case DBG_NEWS:                                                     \
                _DBG_LOG_HDR("N", 34);                                         \
                break;                                                         \
            case DBG_INFO:                                                     \
                _DBG_LOG_HDR("I", 32);                                         \
                break;                                                         \
            case DBG_LOG:                                                      \
                _DBG_LOG_HDR("D", 0);                                          \
                break;                                                         \
            default:                                                           \
                break;                                                         \
        }                                                                      \
        rt_kprintf(fmt, ##__VA_ARGS__);                                        \
        _DBG_COLOR(0);                                                         \
    }

// 定义定位日志输出宏
#define dbg_here                                                               \
    if ((DBG_LVL) <= DBG_LOG) {                                                \
        rt_kprintf(DBG_TAG " Here %s:%d\n", __FUNCTION__, __LINE__);           \
    }

// 定义单行日志输出宏
#define dbg_log_line(lvl, color_n, fmt, ...)                                   \
    do {                                                                       \
        _DBG_LOG_HDR(lvl, color_n);                                            \
        rt_kprintf(fmt, ##__VA_ARGS__);                                        \
        _DBG_LOG_X_END;                                                        \
    } while (0)

// 定义源日志输出宏
#define dbg_raw(...) rt_kprintf(__VA_ARGS__);

#else

#define dbg_log(level, fmt, ...)
#define dbg_here
#define dbg_enter
#define dbg_exit
#define dbg_log_line(lvl, color_n, fmt, ...)
#define dbg_raw(...)

#endif

#if (DBG_LVL >= DBG_LOG)
#define LOG_D(fmt, ...) dbg_log_line("D", 0, fmt, ##__VA_ARGS__)
#else
#define LOG_D(...)
#endif

#if (DBG_LVL >= DBG_INFO)
#define LOG_I(fmt, ...) dbg_log_line("I", 32, fmt, ##__VA_ARGS__)
#else
#define LOG_I(...)
#endif

#if (DBG_LVL >= DBG_NEWS)
#define LOG_N(fmt, ...) dbg_log_line("N", 34, fmt, ##__VA_ARGS__)
#else
#define LOG_N(...)
#endif

#if (DBG_LVL >= DBG_WARN)
#define LOG_W(fmt, ...) dbg_log_line("W", 33, fmt, ##__VA_ARGS__)
#else
#define LOG_W(...)
#endif

#if (DBG_LVL >= DBG_ERROR)
#define LOG_E(fmt, ...) dbg_log_line("E", 31, fmt, ##__VA_ARGS__)
#else
#define LOG_E(...)
#endif

#define LOG_RAW(...) dbg_raw(__VA_ARGS__)

#define LOG_HEX(name, width, buf, size)

#endif