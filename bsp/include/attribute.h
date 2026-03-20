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
 * @brief 指定函数为弱符号，实现可被覆盖。
 */
#define WEAK __attribute__((weak))

/**
 * @brief 避免函数因优化而剔除。
 */
#define USED __attribute__((used))

/**
 * @brief 明确指定不使用该函数。
 */
#define UNUSE_FUNC __attribute__((unused))

/**
 * @brief 明确指定不使用该变量。
 */
#define UNUSE_VAR(x) ((void)x)

/**
 * @brief 禁止编译器自动给函数添加任何汇编代码。
 */
#define NAKED __attribute__((naked))

/**
 * @brief 强制内联函数。
 */
#define INLINE __attribute__((always_inline))

/**
 * @brief 消除结构体成员之间的填充，实现紧凑存储。
 */
#define PACKED __attribute__((packed))

/**
 * @brief 起始地址进行n字节对齐。
 */
#define ALIGN(n) __attribute__((aligned(n)))

/**
 * @brief 添加段分配属性。
 */
#define SECTION(name) __attribute__((section(name)))

/**
 * @brief 不添加任何属性。
 */
#define NONE

/**
 * @brief 将函数放在itcm运行，提高代码执行速度或者避免与flash操作冲突。
 */
#define ITCM SECTION(".itcm")

/**
 * @brief 将数据放在boot与app共享的ram，
 * boot与app之间的切换不会影响这些数据。
 */
#define SHARE SECTION(".share")

/**
 * @brief 将有值数据放在外设可访问的ram，
 * boot与app之间的切换会覆盖这些数据。
 */
#define SHARE_INIT SECTION(".share_init")

/**
 * @brief 将无值数据放在外设可访问的ram，
 * boot与app之间的切换可能会覆盖这些数据。
 */
#define SHARE_UNINIT SECTION(".share_uninit")

/**
 * @brief 将数据放在备份ram，电池供电时始终保持数据存在，
 * 除非程序的显示修改。
 */
#define BACKUP SECTION(".backup")

#endif