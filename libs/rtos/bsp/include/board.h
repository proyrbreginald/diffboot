#ifndef _BOARD_H_
#define _BOARD_H_

#include <stm32h7xx_hal.h>

extern void rt_hw_board_init(void);
extern void rt_hw_mcu_init(void);
uint64_t total_sleep_get(void);
void total_sleep_clear(void);

#endif