/**
 * @file partition.h
 * @author reginald.yang (proyrb@yeah.net)
 * @version 0.1
 * @date 2026-03-18
 * @copyright Copyright (c) 2026
 * @brief 划分flash的逻辑分区。
 */

#ifndef _PARTITION_H_
#define _PARTITION_H_

#include "specification.h" //!< 导入内存划分与段分配

/**
 * @brief 各分区占用扇区数量。
 */
#define LOADER_SECTOR_COUNT 2 //!< 引导程序占用扇区数量
#define USER_SECTOR_COUNT   6 //!< 用户程序占用扇区数量
#define PATCH_SECTOR_COUNT  2 //!< 差分补丁占用扇区数量
#define OEM_SECTOR_COUNT    6 //!< 厂商程序占用扇区数量

/**
 * @brief 内存各分区占用大小。
 */
#define LOADER_SIZE                                                            \
    (LOADER_SECTOR_COUNT * MCU_FLASH_SECTOR_SIZE) //!< 引导程序占用大小
#define USER_SIZE                                                              \
    (USER_SECTOR_COUNT * MCU_FLASH_SECTOR_SIZE) //!< 用户程序占用大小
#define PATCH_SIZE                                                             \
    (PATCH_SECTOR_COUNT * MCU_FLASH_SECTOR_SIZE) //!< 差分补丁占用大小
#define OEM_SIZE                                                               \
    (OEM_SECTOR_COUNT * MCU_FLASH_SECTOR_SIZE) //!< 厂商程序占用大小

/**
 * @brief 各分区起始地址。
 */
#define LOADER_START MCU_FLASH_START              //!< 引导程序起始地址
#define USER_START   (LOADER_START + LOADER_SIZE) //!< 用户程序起始地址
#define PATCH_START  (USER_START + USER_SIZE)     //!< 差分补丁起始地址
#define OEM_START    (PATCH_START + PATCH_SIZE)   //!< 厂商程序起始地址

#endif