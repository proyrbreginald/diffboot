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
#include <mcu/mcu.h>

/**
 * @brief 定义如何启动程序。
 */
typedef struct load_config_info_t {
    uint8_t which; //!< 启动哪个程序
    uint8_t _gap1; //!< 手动填充间隙
    uint8_t _gap2; //!< 手动填充间隙
    uint8_t _gap3; //!< 手动填充间隙
    uint8_t _gap4; //!< 手动填充间隙
    uint8_t _gap5; //!< 手动填充间隙
    uint8_t _gap6; //!< 手动填充间隙
    uint8_t _gap7; //!< 手动填充间隙
} load_config_info_t;

/**
 * @brief 定义crc校验值类型。
 */
typedef uint16_t crc_t;

/**
 * @brief 定义如何校验配置。
 */
typedef struct load_config_t {
    load_config_info_t info; //!< 配置信息
    crc_t crc;               //!< crc校验码
} load_config_t;

load_config_t *load_get_config(void);

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