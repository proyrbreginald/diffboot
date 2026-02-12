#include <printf.h>
#include <usart.h>

void _putchar(char character)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)&character, 1, 0xFFFF);
}