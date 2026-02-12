/**
 * @brief MCU内存分区。
 * @note
 * 在GCC对链接脚本进行预处理时从此处获取MCU内存分区来指导最终链接脚本的生成。
 * @date 2026.2.10
 * @author proyrbreginald
 */

#ifndef _PARTITION_H_
#define _PARTITION_H_

#include "specification.h"

// 内存各分区占用扇区数量
#define LOADER_SECTOR_COUNT 2 // 引导程序占用扇区数量
#define CONFIG_SECTOR_COUNT 1 // 配置参数占用扇区数量
#define USER_SECTOR_COUNT   5 // 用户程序占用扇区数量
#define PATCH_SECTOR_COUNT  3 // 差分数据占用扇区数量
#define OEM_SECTOR_COUNT    5 // 厂商程序占用扇区数量

// 内存各分区占用大小
#define LOADER_SIZE                                                            \
    (LOADER_SECTOR_COUNT * MCU_FLASH_SECTOR_SIZE) // 引导程序占用大小
#define CONFIG_SIZE                                                            \
    (CONFIG_SECTOR_COUNT * MCU_FLASH_SECTOR_SIZE) // 配置参数占用大小
#define USER_SIZE                                                              \
    (USER_SECTOR_COUNT * MCU_FLASH_SECTOR_SIZE) // 用户程序占用大小
#define PATCH_SIZE                                                             \
    (PATCH_SECTOR_COUNT * MCU_FLASH_SECTOR_SIZE)            // 差分数据占用大小
#define OEM_SIZE (OEM_SECTOR_COUNT * MCU_FLASH_SECTOR_SIZE) // 厂商程序占用大小

// 内存各分区起始地址
#define LOADER_START MCU_FLASH0_START             // 引导程序起始地址
#define CONFIG_START (LOADER_START + LOADER_SIZE) // 配置参数起始地址
#define USER_START   (CONFIG_START + CONFIG_SIZE) // 用户程序起始地址
#define PATCH_START  MCU_FLASH1_START             // 差分数据起始地址
#define OEM_START    (PATCH_START + PATCH_SIZE)   // 厂商程序起始地址

#endif