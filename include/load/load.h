/**
 * @file load.h
 * @author reginald.yang (proyrb@yeah.net)
 * @version 0.1
 * @date 2026-03-18
 * @copyright Copyright (c) 2026
 * @brief 定义如何启动程序。
 */

#ifndef _LOAD_H_
#define _LOAD_H_

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief 定义错误选项。
 */
typedef enum load_error_t {
    LOAD_ERROR_VERIFY  = 0x00, //!< 数据验证无效
    LOAD_ERROR_WHICH   = 0x01, //!< 启动参数错误
    LOAD_ERROR_INVALID = 0xff, //!< 无效参数
} load_error_t;

/**
 * @brief 定义复位选项。
 */
typedef enum load_reset_t {
    LOAD_RESET         = 0x00, //!< 需要执行复位
    LOAD_RESET_INVALID = 0xff, //!< 无效参数
} load_reset_t;

/**
 * @brief 定义启动选项。
 */
typedef enum load_which_t {
    LOAD_APP_LOADER  = 0x00, //!< 从loader启动
    LOAD_APP_USER    = 0x01, //!< 从user启动
    LOAD_APP_OEM     = 0x02, //!< 从oem启动
    LOAD_APP_INVALID = 0xff, //!< 无效参数
} load_which_t;

/**
 * @brief 定义打补丁选项。
 */
typedef enum load_apply_t {
    LOAD_APPLY_USER    = 0x00, //!< 应用补丁生成新固件到user
    LOAD_APPLY_OEM     = 0x01, //!< 应用补丁生成新固件到oem
    LOAD_APPLY_INVALID = 0xff, //!< 无效参数
} load_apply_t;

/**
 * @brief 定义固件接收选项。
 */
typedef enum load_patch_t {
    LOAD_PATCH_USER    = 0x00, //!< 接收到user的补丁
    LOAD_PATCH_OEM     = 0x01, //!< 接收到oem的补丁
    LOAD_PATCH_INVALID = 0xff, //!< 无效参数
} load_patch_t;

/**
 * @brief 定义如何启动程序。
 */
typedef struct load_config_info_t {
    volatile load_error_t error;  //!< 错误信息
    volatile load_reset_t reset;  //!< 是否需要执行复位
    volatile load_which_t which;  //!< 从哪个程序启动
    volatile load_apply_t apply;  //!< 生成新固件到哪个分区
    volatile load_patch_t patch;  //!< 接收到哪个补丁
    volatile uint32_t patch_size; //!< 补丁大小
} load_config_info_t;

/**
 * @brief 定义如何校验配置。
 */
typedef struct load_config_t {
    load_config_info_t info; //!< 配置信息
    volatile uint16_t crc;   //!< crc校验码
} load_config_t;

/**
 * @brief 定义函数指针类型。
 */
typedef void (*void_fn_void_t)(void);

/**
 * @brief 进行数据校验，检查数据有效性。
 * @return true 数据有效。
 * @return false 数据无效。
 */
extern bool load_verify_config(void);

/**
 * @brief 更新启动配置的校验值。
 * @return uint16_t 更新后的校验值。
 */
extern uint16_t load_update_config_crc(void);

/**
 * @brief 读取启动配置的校验值。
 * @return uint16_t 读取的crc校验值。
 */
extern uint16_t load_read_config_crc(void);

/**
 * @brief 设置错误代码。
 * @param error 新的错误代码。
 */
extern void load_set_error(load_error_t error);

/**
 * @brief 清除错误代码。
 */
extern void load_clear_error(void);

/**
 * @brief 返回错误的代码。
 * @return load_error_t 当前错误代码。
 */
extern load_error_t load_get_error(void);

/**
 * @brief 设置复位需求。
 */
extern void load_set_reset(void);

/**
 * @brief 清除复位需求。
 */
extern void load_clear_reset(void);

/**
 * @brief 返回复位需求。
 * @return load_reset_t 是否需要复位。
 */
extern load_reset_t load_get_reset(void);

/**
 * @brief 写入新配置。
 * @param which 从哪个程序启动。
 */
extern void load_write_config_which(load_which_t which);

/**
 * @brief 读取启动配置。
 * @param which 指向读取变量的指针。
 * @return true 读取成功。
 * @return false 读取失败。
 */
extern bool load_read_config_which(load_which_t *which);

/**
 * @brief 设置生成新固件到哪个分区。
 * @param apply 新的生成新固件到哪个分区。
 */
extern void load_set_apply(load_apply_t apply);

/**
 * @brief 清除生成新固件到哪个分区。
 */
extern void load_clear_apply(void);

/**
 * @brief 返回生成新固件到哪个分区。
 * @return load_apply_t 当前生成新固件到哪个分区。
 */
extern load_apply_t load_get_apply(void);

/**
 * @brief 设置接收到哪个补丁。
 * @param patch 新的接收到哪个补丁。
 */
extern void load_set_patch(load_patch_t patch);

/**
 * @brief 清除接收到哪个补丁。
 */
extern void load_clear_patch(void);

/**
 * @brief 返回接收到哪个补丁。
 * @return load_patch_t 当前接收到哪个补丁。
 */
extern load_patch_t load_get_patch(void);

/**
 * @brief 设置补丁大小。
 * @param patch_size 新的补丁大小。
 */
extern void load_set_patch_size(uint32_t patch_size);

/**
 * @brief 返回补丁大小。
 * @return uint32_t 当前补丁大小。
 */
extern uint32_t load_get_patch_size(void);

/**
 * @brief 尝试启动app程序。
 */
extern void load_app(void);

/**
 * @brief 启动boot程序。
 */
extern void load_boot(void);

#endif