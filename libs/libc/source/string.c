#include <boot/section.h>
#include <stddef.h>
#include <stdint.h>

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

FAST void *memmove(void *dest, const void *src, size_t n)
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
FAST void *memset(void *s, int v, size_t n)
{
    uint8_t *p = (uint8_t *)s; // Cast input pointer to uint8_t pointer
    while (n--)                // Loop n times, decrementing counter
    {
        *p++ = (uint8_t)
            v; // Assign value v to current position and advance pointer
    }
    return s; // Return original pointer to memory block
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

FAST char *strchr(const char *s, int c)
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

FAST char *strcpy(char *dest, const char *src)
{
    char *d = dest;
    while ((*d++ = *src++))
    {
        // 循环体为空，赋值操作已在判断条件中完成
    }
    return dest;
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

FAST char *strncpy(char *dest, const char *src, size_t n)
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

FAST char *strstr(const char *haystack, const char *needle)
{
    if (!*needle)
        return (char *)haystack;

    const char *h = haystack;
    const char *n = needle;
    size_t needle_len = strlen(n);
    char first = *n;

    // 预处理：跳过不匹配的首字符
    while ((h = strchr(h, first)) != NULL)
    {
        // 使用 memcmp 快速检查后续内容，memcmp 通常是高度优化的
        if (memcmp(h, n, needle_len) == 0)
        {
            return (char *)h;
        }
        h++;
    }
    return NULL;
}

FAST int strcasecmp(const char *s1, const char *s2)
{
    static const unsigned char ascii_tolower_map[256] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b,
        0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
        0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23,
        0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b,
        0x3c, 0x3d, 0x3e, 0x3f, 0x40, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
        0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, 0x70, 0x71, 0x72, 0x73,
        0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
        0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b,
        0x6c, 0x6d, 0x6e, 0x6f, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
        0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f, 0x80, 0x81, 0x82, 0x83,
        0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
        0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0x9b,
        0x9c, 0x9d, 0x9e, 0x9f, 0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7,
        0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf, 0xb0, 0xb1, 0xb2, 0xb3,
        0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
        0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb,
        0xcc, 0xcd, 0xce, 0xcf, 0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7,
        0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf, 0xe0, 0xe1, 0xe2, 0xe3,
        0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,
        0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb,
        0xfc, 0xfd, 0xfe, 0xff};

    const unsigned char *u1 = (const unsigned char *)s1;
    const unsigned char *u2 = (const unsigned char *)s2;

    // 当 u1 指向非空且映射后的两个字符相等时，继续循环
    // 这里的查表操作非常快，且能够被 CPU 的缓存预取
    while (*u1 && (ascii_tolower_map[*u1] == ascii_tolower_map[*u2]))
    {
        u1++;
        u2++;
    }

    // 返回差值。如果完全相等，结果为 0
    return (int)ascii_tolower_map[*u1] - (int)ascii_tolower_map[*u2];
}