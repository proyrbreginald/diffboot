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

#include <stdint.h>
#include <mcu.h>

/**
 * @brief 定义启动选项。
 */
typedef enum load_which_t {
    LOAD_BOOT     = 0x00,
    LOAD_APP_USER = 0x01,
    LOAD_APP_OEM  = 0x02,
    LOAD_NULL     = 0xff,
} load_which_t;

/**
 * @brief 定义如何启动程序。
 */
typedef struct load_config_info_t {
    volatile load_which_t which; //!< 启动哪个程序
} load_config_info_t;

/**
 * @brief 定义如何校验配置。
 */
typedef struct load_config_t {
    load_config_info_t info; //!< 配置信息
    volatile uint16_t crc;   //!< crc校验码
} load_config_t;

/**
 * @brief 读取如何启动。
 * @return uint8_t 从哪个程序启动。
 */
uint8_t load_read_config_which(void);

/**
 * @brief 读取启动配置的校验值。
 * @return uint16_t 读取的crc校验值。
 */
uint16_t load_read_config_crc(void);

/**
 * @brief 写入新配置。
 * @param which 从哪个程序启动。
 */
void load_write_config_which(uint8_t which);

/**
 * @brief 更新启动配置的校验值。
 * @return uint16_t 更新后的校验值。
 */
uint16_t load_update_config_crc(void);

/**
 * @brief 尝试启动app程序。
 */
extern void load_app(void);

/**
 * @brief 尝试启动boot程序。
 */
extern void load_boot(void);

#endif