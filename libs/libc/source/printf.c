#include <mcu/mcu.h>
#include <printf.h>
#include <stdbool.h>
#include <stdint.h>

// 格式化标记位
#define FLAG_LEFT 0x01
#define FLAG_PLUS 0x02
#define FLAG_SPACE 0x04
#define FLAG_ZERO 0x08
#define FLAG_HASH 0x10

// 长度修饰符
enum
{
    LEN_DEFAULT,
    LEN_HH,
    LEN_H,
    LEN_L,
    LEN_LL,
    LEN_Z
};

// 输出上下文记录器，保证线程安全
typedef struct
{
    char *buf;
    size_t size;
    size_t count;
} out_ctx_t;

// 内联单个字符输出，速度最快
static inline void out_char(out_ctx_t *ctx, char c)
{
    if (ctx->buf && ctx->count < ctx->size - 1)
    {
        ctx->buf[ctx->count] = c;
    }
    ctx->count++;
}

// 快速整数转字符串并输出
ITCM static void print_int(out_ctx_t *ctx, uint64_t value, int base,
                           bool is_negative, int width, int precision,
                           int flags, bool uppercase)
{
    char buf[32]; // 足够容纳 64位八进制/十进制的长度
    int len = 0;
    const char *digits = uppercase ? "0123456789ABCDEF" : "0123456789abcdef";

    // 核心优化：如果在32位单片机上，64位除法极慢。
    // 如果值在32位范围内，我们强转为32位变量，让编译器使用高效的硬件除法指令。
    if (value == 0)
    {
        if (precision != 0)
            buf[len++] = '0'; // 精度为0且值为0时不打印
    }
    else
    {
        if (value <= 0xFFFFFFFFull)
        {
            uint32_t val32 = (uint32_t)value;
            while (val32 != 0)
            {
                buf[len++] = digits[val32 % base];
                val32 /= base;
            }
        }
        else
        {
            while (value != 0)
            {
                buf[len++] = digits[value % base];
                value /= base;
            }
        }
    }

    // 处理符号
    char sign = 0;
    if (is_negative)
    {
        sign = '-';
    }
    else if (flags & FLAG_PLUS)
    {
        sign = '+';
    }
    else if (flags & FLAG_SPACE)
    {
        sign = ' ';
    }

    // 计算填充
    int num_digits = len;
    int zeros_to_add = (precision > num_digits) ? (precision - num_digits) : 0;
    int total_len = num_digits + zeros_to_add + (sign ? 1 : 0);

    // 如果有 # 前缀 (针对十六进制和八进制)
    int hash_len = 0;
    if ((flags & FLAG_HASH) && value != 0)
    {
        if (base == 16)
            hash_len = 2;
        else if (base == 8)
            hash_len = 1;
    }
    total_len += hash_len;

    int spaces_to_add = (width > total_len) ? (width - total_len) : 0;

    // 当使用前导0且没有指定精度，也没有左对齐时，将空格转化为前导0
    if ((flags & FLAG_ZERO) && precision < 0 && !(flags & FLAG_LEFT))
    {
        zeros_to_add += spaces_to_add;
        spaces_to_add = 0;
    }

    // 1. 打印右对齐的空格
    if (!(flags & FLAG_LEFT))
    {
        while (spaces_to_add-- > 0)
            out_char(ctx, ' ');
    }

    // 2. 打印符号
    if (sign)
        out_char(ctx, sign);

    // 3. 打印 # 前缀
    if (hash_len == 2)
    {
        out_char(ctx, '0');
        out_char(ctx, uppercase ? 'X' : 'x');
    }
    else if (hash_len == 1)
    {
        out_char(ctx, '0');
    }

    // 4. 打印前导 0
    while (zeros_to_add-- > 0)
        out_char(ctx, '0');

    // 5. 打印数字 (反向)
    while (len > 0)
        out_char(ctx, buf[--len]);

    // 6. 打印左对齐的空格
    if (flags & FLAG_LEFT)
    {
        while (spaces_to_add-- > 0)
            out_char(ctx, ' ');
    }
}

// 可选：浮点数打印 (针对嵌入式的轻量级实现)
#if ENABLE_VSNPRINTF_FLOAT
ITCM static void print_float(out_ctx_t *ctx, double value, int width,
                             int precision, int flags)
{
    if (precision < 0)
        precision = 6; // 默认6位小数

    bool is_negative = false;
    if (value < 0.0)
    {
        is_negative = true;
        value = -value;
    }

    // 四舍五入处理
    double rounding = 0.5;
    for (int i = 0; i < precision; ++i)
        rounding /= 10.0;
    value += rounding;

    // 整数和小数部分拆分 (注意：这里用 uint32_t 限制了最大整数只能到 42亿)
    // 嵌入式中通常足够，如需更大范围需用浮点算法，但会引入巨大性能开销。
    uint32_t int_part = (uint32_t)value;
    double remainder = value - (double)int_part;

    // 打印整数部分 (复用 print_int，宽度和精度置1处理)
    print_int(ctx, int_part, 10, is_negative, 0, 1, flags & ~FLAG_ZERO, false);

    // 打印小数部分
    if (precision > 0 || (flags & FLAG_HASH))
    {
        out_char(ctx, '.');
        while (precision-- > 0)
        {
            remainder *= 10.0;
            uint32_t digit = (uint32_t)remainder;
            out_char(ctx, '0' + digit);
            remainder -= (double)digit;
        }
    }
}
#endif

ITCM int vsnprintf(char *buffer, size_t size, const char *format, va_list ap)
{
    out_ctx_t ctx = {buffer, size, 0};

    while (*format)
    {
        if (*format != '%')
        {
            out_char(&ctx, *format++);
            continue;
        }
        format++; // skip '%'

        if (*format == '%')
        {
            out_char(&ctx, *format++);
            continue;
        }

        // 1. 解析 Flags
        int flags = 0;
        while (1)
        {
            if (*format == '-')
                flags |= FLAG_LEFT;
            else if (*format == '+')
                flags |= FLAG_PLUS;
            else if (*format == ' ')
                flags |= FLAG_SPACE;
            else if (*format == '0')
                flags |= FLAG_ZERO;
            else if (*format == '#')
                flags |= FLAG_HASH;
            else
                break;
            format++;
        }

        // 2. 解析 Width
        int width = 0;
        if (*format == '*')
        {
            width = va_arg(ap, int);
            if (width < 0)
            {
                flags |= FLAG_LEFT;
                width = -width;
            }
            format++;
        }
        else
        {
            while (*format >= '0' && *format <= '9')
            {
                width = width * 10 + (*format++ - '0');
            }
        }

        // 3. 解析 Precision
        int precision = -1;
        if (*format == '.')
        {
            format++;
            precision = 0;
            if (*format == '*')
            {
                precision = va_arg(ap, int);
                if (precision < 0)
                    precision = -1;
                format++;
            }
            else
            {
                while (*format >= '0' && *format <= '9')
                {
                    precision = precision * 10 + (*format++ - '0');
                }
            }
        }

        // 4. 解析 Length 修饰符
        int length = LEN_DEFAULT;
        if (*format == 'l')
        {
            length = LEN_L;
            format++;
            if (*format == 'l')
            {
                length = LEN_LL;
                format++;
            }
        }
        else if (*format == 'h')
        {
            length = LEN_H;
            format++;
            if (*format == 'h')
            {
                length = LEN_HH;
                format++;
            }
        }
        else if (*format == 'z')
        {
            length = LEN_Z;
            format++;
        }

        // 5. 解析 Specifier
        char spec = *format++;
        switch (spec)
        {
        case 'd':
        case 'i': {
            int64_t val = 0;
#if ENABLE_VSNPRINTF_LONG_LONG
            if (length == LEN_LL)
                val = va_arg(ap, long long);
            else
#endif
                if (length == LEN_L)
                val = va_arg(ap, long);
            else if (length == LEN_Z)
                val = va_arg(ap, size_t);
            else
                val = va_arg(ap, int); // h/hh promotions

            bool is_neg = false;
            if (val < 0)
            {
                is_neg = true;
                val = -val;
            }
            print_int(&ctx, (uint64_t)val, 10, is_neg, width, precision, flags,
                      false);
            break;
        }
        case 'u':
        case 'o':
        case 'x':
        case 'X': {
            uint64_t val = 0;
#if ENABLE_VSNPRINTF_LONG_LONG
            if (length == LEN_LL)
                val = va_arg(ap, unsigned long long);
            else
#endif
                if (length == LEN_L)
                val = va_arg(ap, unsigned long);
            else if (length == LEN_Z)
                val = va_arg(ap, size_t);
            else
                val = va_arg(ap, unsigned int);

            int base =
                (spec == 'o') ? 8 : ((spec == 'x' || spec == 'X') ? 16 : 10);
            print_int(&ctx, val, base, false, width, precision, flags,
                      (spec == 'X'));
            break;
        }
        case 'c': {
            char c = (char)va_arg(ap, int);
            if (!(flags & FLAG_LEFT))
            {
                while (--width > 0)
                    out_char(&ctx, ' ');
            }
            out_char(&ctx, c);
            if (flags & FLAG_LEFT)
            {
                while (--width > 0)
                    out_char(&ctx, ' ');
            }
            break;
        }
        case 's': {
            const char *s = va_arg(ap, const char *);
            if (!s)
                s = "(null)";
            int len = 0;
            while (s[len] && (precision < 0 || len < precision))
                len++;

            int spaces = width > len ? width - len : 0;
            if (!(flags & FLAG_LEFT))
            {
                while (spaces-- > 0)
                    out_char(&ctx, ' ');
            }
            for (int i = 0; i < len; i++)
                out_char(&ctx, s[i]);
            if (flags & FLAG_LEFT)
            {
                while (spaces-- > 0)
                    out_char(&ctx, ' ');
            }
            break;
        }
        case 'p': {
            void *p = va_arg(ap, void *);
            flags |= FLAG_HASH; // 强制加上 0x
            print_int(&ctx, (uintptr_t)p, 16, false, width, precision, flags,
                      false);
            break;
        }
#if ENABLE_VSNPRINTF_FLOAT
        case 'f':
        case 'F': {
            double dval = va_arg(ap, double);
            print_float(&ctx, dval, width, precision, flags);
            break;
        }
#endif
        default:
            // 未知字符，直接输出
            out_char(&ctx, spec);
            break;
        }
    }

    // 处理 \0 结尾符
    if (ctx.buf && ctx.size > 0)
    {
        if (ctx.count < ctx.size)
        {
            ctx.buf[ctx.count] = '\0';
        }
        else
        {
            ctx.buf[ctx.size - 1] = '\0';
        }
    }

    // C99 标准要求：返回如果空间无限大时本应该写入的字符数
    return (int)ctx.count;
}

ITCM int sprintf(char *buffer, const char *format, ...)
{
    va_list va;
    va_start(va, format);
    const int ret = vsnprintf(buffer, (size_t)-1, format, va);
    va_end(va);
    return ret;
}