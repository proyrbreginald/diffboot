/**
 * @file specification.h
 * @author reginald.yang (proyrb@yeah.net)
 * @version 0.1
 * @date 2026-03-16
 * @copyright Copyright (c) 2026
 * @brief 在不同MCU上需要适配以下的宏定义，
 * 以执行正确的内存划分与段分配。
 */

#ifndef _SPECIFICATION_H_
#define _SPECIFICATION_H_

/**
 * @brief 闪存规格参数。
 */
#define MCU_FLASH_SECTOR_SIZE  0x00020000 //!< 扇区大小
#define MCU_FLASH_START        0x08000000 //!< 起始地址
#define MCU_FLASH_SECTOR_COUNT 16         //!< 扇区数量

/**
 * @brief ITCM规格参数。
 */
#define MCU_ITCM_OFFSET 0x400 //!< 地址偏移避免与复位中断向量表地址冲突
#define MCU_ITCM_START  (0x00000000 + MCU_ITCM_OFFSET) //!< 起始地址
#define MCU_ITCM_SIZE   (64 * 1024 - MCU_ITCM_OFFSET)  //!< 字节大小

/**
 * @brief DTCM规格参数。
 */
#define MCU_DTCM_START 0x20000000   //!< 起始地址
#define MCU_DTCM_SIZE  (128 * 1024) //!< 字节大小

/**
 * @brief AXIRAM规格参数。
 */
#define MCU_AXIRAM_START 0x24000000   //!< 起始地址
#define MCU_AXIRAM_SIZE  (512 * 1024) //!< 字节大小

/**
 * @brief AHBRAM规格参数。
 */
#define MCU_AHBRAM_START 0x30000000   //!< 起始地址
#define MCU_AHBRAM_SIZE  (288 * 1024) //!< 字节大小

/**
 * @brief AHBRAM1规格参数。
 */
#define MCU_AHBRAM1_START 0x38000000  //!< 起始地址
#define MCU_AHBRAM1_SIZE  (64 * 1024) //!< 字节大小

/**
 * @brief BKPRAM规格参数。
 */
#define MCU_BKPRAM_START 0x38800000 //!< 起始地址
#define MCU_BKPRAM_SIZE  (4 * 1024) //!< 字节大小

#endif