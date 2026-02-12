#include <boot/section.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Fill a memory block with a specific value
 *
 * Sets the first 'n' bytes of the memory block pointed to by 's' to the specified value 'v'.
 * Each byte in the memory block is set to the same value.
 *
 * @param s Pointer to the memory block to be filled
 * @param v Value to be set (converted to unsigned char)
 * @param n Number of bytes to be set to the value
 * @return Returns a pointer to the memory block 's'
 */
FAST void *memset(void *s, int v, size_t n)
{
    uint8_t *p = (uint8_t *)s; // Cast input pointer to uint8_t pointer
    while (n--)                // Loop n times, decrementing counter
    {
        *p++ = (uint8_t)v; // Assign value v to current position and advance pointer
    }
    return s; // Return original pointer to memory block
}

/**
 * @brief 自定义memcpy实现
 * @param dest 目标地址
 * @param src  源地址
 * @param n    要拷贝的字节数
 * @return     返回目标地址 dest
 */
FAST void *memcpy(void *dest, const void *src, size_t n)
{
    // 1. 保存返回用的原始 dest 指针
    void *ret = dest;

    // 2. 转换为字节指针，方便算术运算
    unsigned char *d = (unsigned char *)dest;
    const unsigned char *s = (const unsigned char *)src;

    // =========================================================
    // 优化策略：按字（Word）拷贝
    // 大多数 CPU（如 STM32H7, x86）一次读写 4/8 字节比读写 1 字节快得多
    // =========================================================

    // 定义“字”的类型，32位系统通常是 uint32_t，64位是 uint64_t
    // 这里使用 unsigned long，在 STM32 上通常是 32位 (4字节)
    typedef unsigned long word;
    const size_t word_size = sizeof(word);
    const size_t word_mask = word_size - 1;

    // 只有当数据量足够大（比如大于 16 字节）才尝试优化
    // 且源地址和目标地址的“对齐偏移量”必须一致，才能进行字对齐拷贝
    if (n >= 16 && ((uintptr_t)d & word_mask) == ((uintptr_t)s & word_mask))
    {
        // 步骤 A: 先按字节拷贝，直到 dest 地址对齐到 word 边界
        while ((uintptr_t)d & word_mask)
        {
            *d++ = *s++;
            n--;
        }

        // 步骤 B: 按字（Word）批量拷贝
        // 此时 d 和 s 都是对齐的，可以强制转换为 word 指针
        word *wd = (word *)d;
        const word *ws = (const word *)s;

        // 计算可以按字拷贝的数量
        size_t loops = n / word_size;

        // 循环拷贝（也可以用 Duff's Device 进一步展开循环，但现代编译器优化已足够）
        while (loops--)
        {
            *wd++ = *ws++;
        }

        // 步骤 C: 更新指针和剩余字节数
        d = (unsigned char *)wd;
        s = (const unsigned char *)ws;
        n = n % word_size;
    }

    // =========================================================
    // 兜底策略：按字节拷贝
    // 处理剩余的字节，或者处理无法对齐的情况
    // =========================================================
    while (n--)
    {
        *d++ = *s++;
    }

    return ret;
}

FAST int memcmp(const void *s1, const void *s2, size_t n)
{
    const unsigned char *p1 = (const unsigned char *)s1;
    const unsigned char *p2 = (const unsigned char *)s2;

    for (size_t i = 0; i < n; i++)
    {
        if (p1[i] != p2[i])
        {
            // 返回第一个不同字节的差值
            return p1[i] - p2[i];
        }
    }

    // 如果前 n 个字节都相同，返回 0
    return 0;
}

FAST size_t strlen(const char *s)
{
    const char *p = s;
    while (*p)
    {
        p++;
    }
    return (size_t)(p - s);
}

FAST char *strcpy(char *dest, const char *src)
{
    char *d = dest;
    while ((*d++ = *src++))
    {
        // 循环体为空，赋值操作已在判断条件中完成
    }
    return dest;
}

FAST char *strcat(char *dest, const char *src)
{
    char *d = dest;
    // 1. 找到 dest 的末尾 ('\0' 的位置)
    while (*d)
    {
        d++;
    }
    // 2. 从该位置开始复制 src
    while ((*d++ = *src++))
    {
        // 复制过程包含 '\0'
    }
    return dest;
}

FAST int strcmp(const char *s1, const char *s2)
{
    while (*s1 && (*s1 == *s2))
    {
        s1++;
        s2++;
    }
    // 返回差值：必须转换为 unsigned char 以防止符号扩展导致的问题
    return *(unsigned char *)s1 - *(unsigned char *)s2;
}

FAST int strncmp(const char *s1, const char *s2, size_t n)
{
    if (n == 0)
        return 0;

    while (n-- > 0 && *s1 && (*s1 == *s2))
    {
        if (n == 0)
            break; // 已经比较了 n 个字符
        s1++;
        s2++;
    }
    return *(unsigned char *)s1 - *(unsigned char *)s2;
}