/**
 * @file mcu.h
 * @author reginald.yang (proyrb@yeah.net)
 * @version 0.1
 * @date 2026-03-18
 * @copyright Copyright (c) 2026
 * @brief 作为diffboot的底层头文件，
 * 向上层提供硬件定义、软件适配接口和编译期符号与信息。
 */

#ifndef _MCU_H_
#define _MCU_H_

#include <stdint.h>
#include "macro.h"                                      //!< 导入mcu相关宏定义
#include DIFFBOOT_MCU_INCLUDE_LINK_SYMBOL(DIFFBOOT_MCU) //!< 导入链接脚本符号
#include "attribute.h" //!< 导入函数与变量属性分配宏定义
#include "reset.h"     //!< 导入复位处理函数

#endif