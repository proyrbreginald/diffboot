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
#define DBG_FATAL   0
#define DBG_ERROR   1
#define DBG_WARN    2
#define DBG_INFO    3
#define DBG_LOG     4
#define DBG_VERBOSE 5

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

/* 前景色 (Foreground) */
#define DBG_FG_BLACK  "30"
#define DBG_FG_RED    "31"
#define DBG_FG_GREEN  "32"
#define DBG_FG_YELLOW "33"
#define DBG_FG_BLUE   "34"
#define DBG_FG_PURPLE "35"
#define DBG_FG_CYAN   "36"
#define DBG_FG_WHITE  "37"

/* 高亮度前景色 (High Intensity Foreground) */
#define DBG_FG_H_BLACK  "90"
#define DBG_FG_H_RED    "91"
#define DBG_FG_H_GREEN  "92"
#define DBG_FG_H_YELLOW "93"
#define DBG_FG_H_BLUE   "94"
#define DBG_FG_H_PURPLE "95"
#define DBG_FG_H_CYAN   "96"
#define DBG_FG_H_WHITE  "97"

/* 背景色 (Background) */
#define DBG_BG_BLACK                     "40"
#define DBG_BG_RED                       "41"
#define DBG_BG_GREEN                     "42"
#define DBG_BG_YELLOW                    "43"
#define DBG_BG_BLUE                      "44"
#define DBG_BG_PURPLE                    "45"
#define DBG_BG_CYAN                      "46"
#define DBG_BG_WHITE                     "47"

#define _DBG_LOG_HDR(color_fg, color_bg) "\033[" color_fg ";" color_bg "m"
#define _DBG_LOG_X_END                   "\033[0m\n"

#else

#define _DBG_LOG_HDR(lvl_name, color_fg) "[" lvl_name "/" DBG_TAG "] "
#define _DBG_LOG_X_END                   "\n"

#endif

// 定义定位日志输出宏
#define dbg_here                                                               \
    if ((DBG_LVL) <= DBG_LOG) {                                                \
        rt_kprintf(DBG_TAG " Here %s:%d\n", __FUNCTION__, __LINE__);           \
    }

// 定义单行日志输出宏
#define dbg_log_line(lvl, color_fg, color_bg, fmt, ...)                        \
    do {                                                                       \
        rt_kprintf(_DBG_LOG_HDR(color_fg, color_bg) "[" lvl "/" DBG_TAG        \
                                                    "] " fmt _DBG_LOG_X_END,   \
                   ##__VA_ARGS__);                                             \
    } while (0)

// 定义源日志输出宏
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

#if (DBG_LVL >= DBG_LOG)
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

#endif