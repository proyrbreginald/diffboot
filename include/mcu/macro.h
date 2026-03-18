/**
 * @file macro.h
 * @author reginald.yang (proyrb@yeah.net)
 * @version 0.1
 * @date 2026-03-17
 * @copyright Copyright (c) 2026
 * @brief 集成boot和app需要使用的宏定义。
 */

#ifndef _MACRO_H_
#define _MACRO_H_

/**
 * @brief 将输入标识符转换成字符串。
 */
#define CONVERT_TO_STR(x) #x

/**
 * @brief 用来导入特定芯片的flash分区定义。
 */
// clang-format off
#define DIFFBOOT_MCU_INCLUDE_PARTITION_H(model) CONVERT_TO_STR(model/partition.h)
// clang-format on

/**
 * @brief 用来导入特定芯片的链接脚本。
 */
// clang-format off
#define DIFFBOOT_MCU_INCLUDE_LINK_LD(model) CONVERT_TO_STR(model/link.ld)
// clang-format on

/**
 * @brief 用来导入链接脚本的符号。
 */
// clang-format off
#define DIFFBOOT_MCU_INCLUDE_LINK_SYMBOL(model) CONVERT_TO_STR(model/link_symbol.h)
// clang-format on

/**
 * @brief 定义使用哪款芯片。
 */
#define DIFFBOOT_MCU stm32h743iit6

#include DIFFBOOT_MCU_INCLUDE_PARTITION_H(DIFFBOOT_MCU) //!< 导入特定芯片的flash分区定义。

#endif