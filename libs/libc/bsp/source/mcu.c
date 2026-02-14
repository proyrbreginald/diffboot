#include <printf.h>
#include <usart.h>

void _putchar(char character)
{
    /* 等待发送数据寄存器为空 (TXE) */
    while (!LL_USART_IsActiveFlag_TXE_TXFNF(USART1))
        ;

    /* 写入字符到发送数据寄存器 */
    LL_USART_TransmitData8(USART1, character);
}