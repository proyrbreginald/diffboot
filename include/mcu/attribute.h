/**
 * @file attribute.h
 * @author reginald.yang (proyrb@yeah.net)
 * @version 0.1
 * @date 2026-03-18
 * @copyright Copyright (c) 2026
 * @brief 提供为函数或变量分配属性的宏定义，
 * 以提供高级程序特性。
 */

#ifndef _ATTRIBUTE_H_
#define _ATTRIBUTE_H_

/**
 * @brief 将函数放在itcm运行，提高代码执行速度或者避免与flash操作冲突。
 */
#define ITCM __attribute__((section(".itcm"), noinline))

/**
 * @brief 将数据放在boot与app共享的ram，
 * boot与app之间的切换不会影响这些数据。
 */
#define SHARE __attribute__((section(".share")))

/**
 * @brief 将有值数据放在外设可访问的ram，
 * boot与app之间的切换会覆盖这些数据。
 */
#define SHARE_INIT __attribute__((section(".share_init")))

/**
 * @brief 将无值数据放在外设可访问的ram，
 * boot与app之间的切换可能会覆盖这些数据。
 */
#define SHARE_UNINIT __attribute__((section(".share_uninit")))

/**
 * @brief 将数据放在备份ram，电池供电时始终保持数据存在，
 * 除非程序的显示修改。
 */
#define BACKUP __attribute__((section(".backup")))

/**
 * @brief 指定函数为弱符号，实现可被覆盖。
 */
#define WEAK __attribute__((weak))

#endif