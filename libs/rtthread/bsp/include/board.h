/**
 * @file board.h
 * @author reginald.yang (proyrb@yeah.net)
 * @version 0.1
 * @date 2026-03-23
 * @copyright Copyright (c) 2026
 * @brief 导出板级支持包接口。
 */

#ifndef _BOARD_H_
#define _BOARD_H_

#include <main.h>

/**
 * @brief 启动阶段执行mcu相关的配置以支持rtthread特性与功能。
 */
extern void rt_hw_mcu_init(void);

/**
 * @brief 启动阶段执行board相关的配置以支持相关库以及顶层业务特性与功能。
 */
extern void rt_hw_board_init(void);

#endif