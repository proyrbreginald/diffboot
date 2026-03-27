#ifndef _DETOOLS_PORT_H_
#define _DETOOLS_PORT_H_

#include <stdint.h>
#include <detools.h>

/*
 * 移植层上下文结构体
 * 用于在回调函数中记录旧固件、差分包和新固件的 Flash 地址偏移
 */
typedef struct {
    uint32_t old_app_base;   // 旧固件在 Flash 中的首地址
    uint32_t old_app_offset; // 旧固件当前读取偏移量

    uint32_t patch_base;   // 差分包在 Flash 中的首地址
    uint32_t patch_offset; // 差分包当前读取偏移量

    uint32_t new_app_base;   // 新固件在 Flash 中的首地址
    uint32_t new_app_offset; // 新固件当前写入偏移量

    /* 新增：32 字节对齐的缓存区，用于凑齐 Flash Word */
    __attribute__((aligned(32))) uint8_t write_buf[32];
    uint32_t write_buf_len; // 缓存区当前已有字节数
} detools_ctx_t;

/**
 * @brief 执行差分还原
 *
 * @param old_app_addr  旧固件首地址 (例如 0x08010000)
 * @param patch_addr    差分包首地址 (例如 0x08040000)
 * @param patch_size    差分包总大小
 * @param new_app_addr  新固件写入首地址 (例如 0x08080000)
 * @return int          成功返回生成的新固件大小，失败返回负数错误码
 */
int detools_apply_patch(uint32_t old_app_addr, uint32_t patch_addr,
                        uint32_t patch_size, uint32_t new_app_addr);

#endif