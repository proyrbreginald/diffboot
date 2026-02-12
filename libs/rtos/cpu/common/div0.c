#include <rtthread.h>
#include <rtdbg.h>

void __div0(void)
{
    LOG_E("div0!");
    while (1)
        ;
}