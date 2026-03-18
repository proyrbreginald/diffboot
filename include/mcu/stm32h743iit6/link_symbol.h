/**
 * @file link_symbol.h
 * @author reginald.yang (proyrb@yeah.net)
 * @version 0.1
 * @date 2026-03-18
 * @copyright Copyright (c) 2026
 * @brief 导出链接脚本中的符号。
 */

#ifndef _LINK_SYMBOL_H_
#define _LINK_SYMBOL_H_

#include <stdint.h>

/**
 * @brief 导出堆栈符号。
 */
extern const char _stack_start[];    //!< 初始栈地址(从高地址向底地址生长)
extern const char _stack_min_size[]; //!< 栈最小大小
extern const char _heap_start[];     //!< 紧跟在数据段之后
extern const char _heap_end[];       //!< 堆结束地址(紧靠栈且不覆盖最小栈空间)

/**
 * @brief 导入itcm初始化符号定义。
 */
extern const char _itcm_section_addr[]; //!< flash中快速代码段的起始地址
extern const char _itcm_ram_start[];    //!< itcm中快速代码段的起始地址
extern const char _itcm_ram_end[];      //!< itcm中快速代码段的结束地址

/**
 * @brief 导入dtcm初始化符号定义。
 */
extern const char _dtcm_ram_section_addr[]; //!< flash中dtcm有值数据的起始地址
extern const char _dtcm_ram_init_start[];   //!< dtcm中有值数据的起始地址
extern const char _dtcm_ram_init_end[];     //!< dtcm中有值数据的结束地址
extern const char _dtcm_ram_uninit_start[]; //!< dtcm中无值数据的起始地址
extern const char _dtcm_ram_uninit_end[];   //!< dtcm中无值数据的结束地址

/**
 * @brief 导入axiram初始化符号定义。
 */
extern const char _axi_ram_section_addr[]; //!< flash中axiram有值数据的起始地址
extern const char _axi_ram_init_start[];   //!< axiram中有值数据的起始地址
extern const char _axi_ram_init_end[];     //!< axiram中有值数据的结束地址
extern const char _axi_ram_uninit_start[]; //!< axiram中无值数据的起始地址
extern const char _axi_ram_uninit_end[];   //!< axiram中无值数据的结束地址

/**
 * @brief 导入ahbram初始化符号定义。
 */
extern const char _ahb_ram_section_addr[]; //!< flash中ahb有值数据的起始地址
extern const char _ahb_ram_init_start[];   //!< ahb中有值数据的起始地址
extern const char _ahb_ram_init_end[];     //!< ahb中有值数据的结束地址
extern const char _ahb_ram_uninit_start[]; //!< ahb中无值数据的起始地址
extern const char _ahb_ram_uninit_end[];   //!< ahb中无值数据的结束地址

/**
 * @brief 导入ahbram1初始化符号定义。
 */
extern const char _ahb_ram1_section_addr[]; //!< flash中ahb1有值数据的起始地址
extern const char _ahb_ram1_init_start[];   //!< ahb1中有值数据的起始地址
extern const char _ahb_ram1_init_end[];     //!< ahb1中有值数据的结束地址
extern const char _ahb_ram1_uninit_start[]; //!< ahb1中无值数据的起始地址
extern const char _ahb_ram1_uninit_end[];   //!< ahb1中无值数据的结束地址

/**
 * @brief 导入bkpram符号定义。
 */
extern const char _bkp_ram_start[]; //!< bkpram数据的起始地址
extern const char _bkp_ram_end[];   //!< bkpram数据的结束地址

#endif