/**
 * @file mcu_port.ld
 * @author reginald.yang (proyrb@yeah.net)
 * @version 0.1
 * @date 2026-03-18
 * @copyright Copyright (c) 2026
 * @brief 导入特定芯片的移植文件。
 */

#include "macro.h" //!< 导入mcu相关宏定义
#include DIFFBOOT_MCU_INCLUDE_LINK_LD(DIFFBOOT_MCU) //!< 导入特定mcu的链接脚本