#include <mcu/mcu.h>
#include <stdlib.h>

ITCM int atoi(const char *p)
{
    long res = 0;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
    {
        p++; // 跳过空白符
    }
    while (*p >= '0' && *p <= '9')
    {
        res = res * 10 + (*p - '0');
        p++;
    }
    return res;
}