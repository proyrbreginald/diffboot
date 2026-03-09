#ifndef PRINTF_H
#define PRINTF_H

#include <stdarg.h>
#include <stddef.h>

/* 配置宏：按需开启，节省ROM */
#define ENABLE_VSNPRINTF_FLOAT     1 // 是否支持浮点数%f(占用更多ROM)
#define ENABLE_VSNPRINTF_LONG_LONG 1 // 是否支持64位整型%lld, %llu

/**
 * @brief 将格式化数据写入字符串缓冲区（带长度限制，使用 va_list）
 * 
 * @param buffer 指向用于存储结果字符串的字符数组的指针。
 * @param size   缓冲区的最大容量（字节数），包括最后的空终止符 '\0'。
 *               即使发生截断，写入的数据也不会超过这个长度。
 * @param format 格式化字符串（包含占位符，如 %d, %s 等）。
 * @param ap     一个 va_list 类型的变量，代表已经初始化的可变参数列表。
 * 
 * @return 成功：返回本应写入缓冲区的总字符数（不包括 '\0'）。
 *               如果返回值 >= size，说明输出被截断了。
 *         失败：返回一个负数。
 */
int vsnprintf(char *buffer, size_t size, const char *format, va_list ap);

/**
 * @brief 将格式化数据写入字符串缓冲区
 * 
 * @note  警告：该函数不检查目标缓冲区 buffer 的大小，存在缓冲区溢出风险。
 *        在现代开发中，建议优先使用 snprintf 以确保安全。
 * 
 * @param buffer 指向用于存储结果字符串的字符数组的指针。
 *               调用者必须确保该空间足够容纳生成的所有数据。
 * @param format 格式化字符串（包含占位符，如 %x, %f 等）。
 * @param ...    可变参数，对应格式化字符串中的占位符。
 * 
 * @return 成功：返回实际写入缓冲区的字符总数（不包括末尾的 '\0'）。
 *         失败：返回一个负数。
 */
int sprintf(char *buffer, const char *format, ...);

#endif