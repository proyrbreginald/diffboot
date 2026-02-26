#ifndef PRINTF_H
#define PRINTF_H

#include <stdarg.h>
#include <stddef.h>

/* 配置宏：按需开启，节省ROM */
#define ENABLE_VSNPRINTF_FLOAT     1 // 是否支持浮点数%f(占用更多ROM)
#define ENABLE_VSNPRINTF_LONG_LONG 1 // 是否支持64位整型%lld, %llu

int vsnprintf(char *buffer, size_t size, const char *format, va_list ap);
int sprintf(char *buffer, const char *format, ...);

#endif