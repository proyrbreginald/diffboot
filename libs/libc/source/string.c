#include <mcu.h>
#include <stddef.h>
#include <stdint.h>

ITCM int memcmp(const void *s1, const void *s2, size_t n)
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

/**
 * @brief 自定义memcpy实现
 * @param dest 目标地址
 * @param src  源地址
 * @param n    要拷贝的字节数
 * @return     返回目标地址 dest
 */
ITCM void *memcpy(void *dest, const void *src, size_t n)
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

        // 循环拷贝（也可以用 Duff's Device
        // 进一步展开循环，但现代编译器优化已足够）
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

ITCM void *memmove(void *dest, const void *src, size_t n)
{
    unsigned char *d = (unsigned char *)dest;
    const unsigned char *s = (const unsigned char *)src;

    if (d == s || n == 0)
        return dest;

    // 1. 判断方向：如果 dest 在 src 后面且有重叠，必须从后往前拷贝
    if (d > s && d < s + n)
    {
        d += n;
        s += n;

        // --- 从后向前拷贝 ---

        // 先拷贝末尾不满足一个字的部分（对齐尾部）
        while (n > 0 && ((uintptr_t)d & 0x7))
        {
            *--d = *--s;
            n--;
        }

        // 以 8 字节（Word）为单位反向拷贝
        while (n >= 8)
        {
            n -= 8;
            d -= 8;
            s -= 8;
            *(uint64_t *)d = *(uint64_t *)s;
        }

        // 拷贝剩余字节
        while (n > 0)
        {
            *--d = *--s;
            n--;
        }
    }
    else
    {
        // --- 从前向后拷贝（同 memcpy 逻辑） ---

        // 先拷贝开头不满足一个字的部分（对齐首部）
        while (n > 0 && ((uintptr_t)d & 0x7))
        {
            *d++ = *s++;
            n--;
        }

        // 以 8 字节（Word）为单位正向拷贝
        while (n >= 8)
        {
            *(uint64_t *)d = *(uint64_t *)s;
            d += 8;
            s += 8;
            n -= 8;
        }

        // 拷贝剩余字节
        while (n > 0)
        {
            *d++ = *s++;
            n--;
        }
    }

    return dest;
}

/**
 * @brief Fill a memory block with a specific value
 *
 * Sets the first 'n' bytes of the memory block pointed to by 's' to the
 * specified value 'v'. Each byte in the memory block is set to the same value.
 *
 * @param s Pointer to the memory block to be filled
 * @param v Value to be set (converted to unsigned char)
 * @param n Number of bytes to be set to the value
 * @return Returns a pointer to the memory block 's'
 */
ITCM void *memset(void *s, int v, size_t n)
{
    uint8_t *p = (uint8_t *)s; // Cast input pointer to uint8_t pointer
    while (n--)                // Loop n times, decrementing counter
    {
        *p++ = (uint8_t)
            v; // Assign value v to current position and advance pointer
    }
    return s; // Return original pointer to memory block
}

ITCM char *strcat(char *dest, const char *src)
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

ITCM char *strchr(const char *s, int c)
{
    unsigned char target = (unsigned char)c;

    // 1. 对齐内存地址：先逐字节查找，直到指针 8 字节对齐
    while (((uintptr_t)s & 7) != 0)
    {
        if (*(unsigned char *)s == target)
        {
            return (char *)s;
        }
        if (*s == '\0')
        {
            return NULL;
        }
        s++;
    }

    // 2. 准备掩码
    // 创建一个包含 8 个目标字符的 64 位整数
    uint64_t char_mask = target;
    char_mask |= char_mask << 8;
    char_mask |= char_mask << 16;
    char_mask |= char_mask << 32;

    // 常数掩码：用于快速零字节检测
    const uint64_t magic_bits = 0x0101010101010101ULL;
    const uint64_t high_bits = 0x8080808080808080ULL;

    const uint64_t *word_ptr = (const uint64_t *)s;

    // 3. 主循环：一次检查 8 个字节
    while (1)
    {
        uint64_t word = *word_ptr;

        // 技巧 A: 检测当前字中是否包含 \0 (Null Byte)
        // 原理：(word - 0x01...) & ~word & 0x80... 如果非零，表示存在 0x00
        uint64_t has_zero = (word - magic_bits) & ~word & high_bits;

        // 技巧 B: 检测当前字中是否包含目标字符 target
        // 原理：将 word 与 char_mask 异或，如果某个字节相同，异或结果该字节为
        // 0x00 然后再次使用零字节检测算法
        uint64_t xor_res = word ^ char_mask;
        uint64_t has_char = (xor_res - magic_bits) & ~xor_res & high_bits;

        // 如果发现了零字节或目标字节，跳出循环进行细致检查
        if (has_zero || has_char)
        {
            break;
        }
        word_ptr++;
    }

    // 4. 细致检查：确定具体是哪个位置
    s = (const char *)word_ptr;
    while (1)
    {
        if (*(unsigned char *)s == target)
        {
            return (char *)s;
        }
        if (*s == '\0')
        {
            // 特殊情况：如果寻找的就是 '\0'，标准 strchr 要求返回指向它的指针
            return (target == '\0') ? (char *)s : NULL;
        }
        s++;
    }
}

ITCM int strcmp(const char *s1, const char *s2)
{
    while (*s1 && (*s1 == *s2))
    {
        s1++;
        s2++;
    }
    // 返回差值：必须转换为 unsigned char 以防止符号扩展导致的问题
    return *(unsigned char *)s1 - *(unsigned char *)s2;
}

ITCM size_t strlen(const char *s)
{
    const char *p = s;
    while (*p)
    {
        p++;
    }
    return (size_t)(p - s);
}

ITCM int strncmp(const char *s1, const char *s2, size_t n)
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

ITCM char *strncpy(char *dest, const char *src, size_t n)
{
    size_t i = 0;

    // 对齐检查：尝试按机器字（8字节）拷贝
    if (n >= sizeof(uint64_t))
    {
        // 只有当指针对齐时才使用长字拷贝（防止非对齐访问性能损失）
        if (((uintptr_t)src & 7) == ((uintptr_t)dest & 7))
        {
            while (i + sizeof(uint64_t) <= n)
            {
                uint64_t word = *(uint64_t *)(src + i);
                // 检查这 8 个字节中是否包含 '\0'
                // 位运算技巧：检查字中是否存在零字节
                if ((word - 0x0101010101010101ULL) & ~word &
                    0x8080808080808080ULL)
                {
                    break;
                }
                *(uint64_t *)(dest + i) = word;
                i += sizeof(uint64_t);
            }
        }
    }

    // 处理剩余字节或 src 结束后的逻辑
    for (; i < n && src[i] != '\0'; i++)
    {
        dest[i] = src[i];
    }
    // 标准 strncpy 规定：若 src 结束，剩余位置补 '\0'
    for (; i < n; i++)
    {
        dest[i] = '\0';
    }

    return dest;
}