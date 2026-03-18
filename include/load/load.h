/**
 * @file load.h
 * @author reginald.yang (proyrb@yeah.net)
 * @version 0.1
 * @date 2026-03-18
 * @copyright Copyright (c) 2026
 * @brief 导出app与boot的启动函数。
 */

#ifndef _LOAD_H_
#define _LOAD_H_

/**
 * @brief 尝试启动app程序。
 */
extern void load_app(void);

/**
 * @brief 尝试启动boot程序。
 */
extern void load_boot(void);

#endif