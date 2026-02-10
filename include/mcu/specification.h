/**
 * @brief MCU规格参数。
 * @warning 当MCU规格变化后需要在此处修改的对应的规格参数。
 * @note
 * 在GCC对链接脚本进行预处理时从此处获取MCU规格参数来指导最终链接脚本的生成。
 * @date 2026.2.10
 * @author proyrbreginald
 */

#ifndef _SPECIFICATION_H_
#define _SPECIFICATION_H_

/* Flash0规格参数 */
#define MCU_FLASH_SECTOR_SIZE   (128u * 1024u) // Flash扇区大小
#define MCU_FLASH0_START        0x08000000     // Flash0起始地址
#define MCU_FLASH0_SECTOR_COUNT 8u             // Flash0扇区数量

/* Flash1规格参数 */
#define MCU_FLASH1_EXIST 1
#if MCU_FLASH1_EXIST
#define MCU_FLASH1_START                                                       \
    (MCU_FLASH0_START +                                                        \
     (MCU_FLASH0_SECTOR_COUNT * MCU_FLASH_SECTOR_SIZE)) // Flash1起始地址
#define MCU_FLASH1_SECTOR_COUNT 8u                      // Flash1扇区数量
#endif

/* ITCM规格参数 */
#define MCU_ITCM_EXIST 1
#if MCU_ITCM_EXIST
#define MCU_ITCM_OFFSET 0x400 // ITCM地址偏移避免与复位中断向量表地址冲突
#define MCU_ITCM_START  (0x00000000 + MCU_ITCM_OFFSET) // ITCM起始地址
#define MCU_ITCM_SIZE   (64u * 1024u - 0x400)          // ITCM大小
#endif

/* DTCM规格参数 */
#define MCU_DTCM_EXIST 1
#if MCU_DTCM_EXIST
#define MCU_DTCM_START 0x20000000     // DTCM起始地址
#define MCU_DTCM_SIZE  (128u * 1024u) // DTCM大小
#endif

/* RAM0规格参数 */
#define MCU_RAM0_EXIST 1
#if MCU_RAM0_EXIST
#define MCU_RAM0_START 0x24000000     // RAM0起始地址
#define MCU_RAM0_SIZE  (512u * 1024u) // RAM0大小
#endif

/* RAM1规格参数 */
#define MCU_RAM1_EXIST 1
#if MCU_RAM1_EXIST
#define MCU_RAM1_START 0x30000000     // RAM1起始地址
#define MCU_RAM1_SIZE  (288u * 1024u) // RAM1大小
#endif

/* RAM2规格参数 */
#define MCU_RAM2_EXIST 1
#if MCU_RAM2_EXIST
#define MCU_RAM2_START 0x38000000    // RAM2起始地址
#define MCU_RAM2_SIZE  (64u * 1024u) // RAM2大小
#endif

#endif