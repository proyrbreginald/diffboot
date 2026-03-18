/**
 * @file link.ld
 * @author reginald.yang (proyrb@yeah.net)
 * @version 0.1
 * @date 2026-03-18
 * @copyright Copyright (c) 2026
 * @brief 此文件为bootloader和app的链接脚本，
 * 通过在预处理阶段设置对应的宏来生成对应bootloader或者app的链接脚本。
 */

#include "partition.h"

/**
 * @brief 定义flash与ram。
 */
MEMORY
{
    /* flash */
    LOADER  (rwx) : ORIGIN = LOADER_START,      LENGTH = LOADER_SIZE
    USER    (rwx) : ORIGIN = USER_START,        LENGTH = USER_SIZE
    PATCH   (rw)  : ORIGIN = PATCH_START,       LENGTH = PATCH_SIZE
    OEM     (rwx) : ORIGIN = OEM_START,         LENGTH = OEM_SIZE

    /* tcm */
    ITCM    (rwx) : ORIGIN = MCU_ITCM_START,    LENGTH = MCU_ITCM_SIZE
    DTCM    (rwx) : ORIGIN = MCU_DTCM_START,    LENGTH = MCU_DTCM_SIZE

    /* sram */
    AXIRAM  (rwx) : ORIGIN = MCU_AXIRAM_START,  LENGTH = MCU_AXIRAM_SIZE
    AHBRAM  (rwx) : ORIGIN = MCU_AHBRAM_START,  LENGTH = MCU_AHBRAM_SIZE
    AHBRAM1 (rwx) : ORIGIN = MCU_AHBRAM1_START, LENGTH = MCU_AHBRAM1_SIZE
    BKPRAM  (rwx) : ORIGIN = MCU_BKPRAM_START,  LENGTH = MCU_BKPRAM_SIZE
}

/* 初始栈地址(从高地址向底地址生长) */
_stack_start = ORIGIN(DTCM) + LENGTH(DTCM);

/* 栈最小大小 */
_stack_min_size = 0x400;

/* 堆结束地址(紧靠栈且不覆盖最小栈空间) */
_heap_end = _stack_start - _stack_min_size;

/* 根据构建参数匹配不同的段分配布局 */
#ifdef BUILD_LOADER
#define CODE_PARTITION LOADER
#elif BUILD_USER
#define CODE_PARTITION USER
#elif BUILD_OEM
#define CODE_PARTITION OEM
#else
/* 无有效参数时报错 */
#error "BUILD_LOADER, BUILD_USER or BUILD_OEM must be defined"
#endif

/**
 * @brief 定义段如何分配。
 */
SECTIONS
{
    /* 普通代码段 */
    .code : ALIGN(4)
    {
        KEEP(*(.isr_vector))        /* 中断向量表 */
        *(.text*)                   /* 普通代码 */
        *(.glue_7*)                 /* ARM/Thumb胶水代码 */
        KEEP(*(SORT(.rti_fn*)))     /* RTOS启动时自动执行 */
    } > CODE_PARTITION

    /* 快速代码段 */
    .itcm : ALIGN(4)
    {
        _itcm_ram_start = .;        /* 起始地址 */
        KEEP(*(.itcm*))             /* 快速代码 */
        _itcm_ram_end = .;          /* 结束地址 */
    } > ITCM AT > CODE_PARTITION
    _itcm_section_addr = LOADADDR(.itcm);

    /* 常量数据段 */
    .const : ALIGN(4)
    {
        *(.rodata*)                 /* 常量数据 */
    } > CODE_PARTITION

    /* DTCM已初始化数据段 */
    .dtcm_init : ALIGN(4)
    {
        _dtcm_ram_init_start = .;   /* 起始地址 */
        *(.data*)                   /* 已初始化数据 */
        _dtcm_ram_init_end = .;     /* 结束地址 */
    } > DTCM AT > CODE_PARTITION
    _dtcm_ram_section_addr = LOADADDR(.dtcm_init);

    /* DTCM未初始化数据段 */
    .dtcm_uninit : ALIGN(4)
    {
        _dtcm_ram_uninit_start = .; /* 起始地址 */
        *(.bss*)                    /* 未初始化数据 */
        _dtcm_ram_uninit_end = .;   /* 结束地址 */
    } > DTCM

    /* 堆空间段 */
    .heap : ALIGN(8)
    {
        _heap_start = .;            /* 紧跟在数据段之后 */
    } > DTCM

    /* 栈空间段 */
    .stack : ALIGN(8)
    {
        . = . + _stack_min_size;    /* 不足以分配最小栈空间时报错 */
    } > DTCM

    /* AXIRAM已初始化数据段 */
    .axi_ram_init : ALIGN(4)
    {
        _axi_ram_init_start = .;    /* 起始地址 */
        _axi_ram_init_end = .;      /* 结束地址 */
    } > AXIRAM AT > LOADER
    _axi_ram_section_addr = LOADADDR(.axi_ram_init);

    /* AXIRAM未初始化数据段 */
    .axi_ram_uninit : ALIGN(4)
    {
        _axi_ram_uninit_start = .;  /* 起始地址 */
        _axi_ram_uninit_end = .;    /* 结束地址 */
    } > AXIRAM

    /* AHBRAM已初始化数据段 */
    .ahb_ram_init : ALIGN(4)
    {
        _ahb_ram_init_start = .;    /* 起始地址 */
        *(.share_init)              /* 已初始化数据 */
        _ahb_ram_init_end = .;      /* 结束地址 */
    } > AHBRAM AT > LOADER
    _ahb_ram_section_addr = LOADADDR(.ahb_ram_init);

    /* AHBRAM未初始化数据段 */
    .ahb_ram_uninit : ALIGN(4)
    {
        _ahb_ram_uninit_start = .;  /* 起始地址 */
        *(.share_uninit)            /* 未初始化数据 */
        _ahb_ram_uninit_end = .;    /* 结束地址 */
    } > AHBRAM

    /* share数据段 */
    .share_ram : ALIGN(4)
    {
        _share_ram_start = .;       /* 起始地址 */
        *(.share)                   /* 程序之间共享的数据 */
        _share_ram_end = .;         /* 结束地址 */
    } > AHBRAM1 AT > LOADER
    _share_ram_section_addr = LOADADDR(.share_ram);

    /* BKPRAM已初始化数据段 */
    .bkp_ram : ALIGN(4)
    {
        _bkp_ram_start = .;         /* 起始地址 */
        *(.backup)                  /* 备份数据 */
        _bkp_ram_end = .;           /* 结束地址 */
    } > BKPRAM

    /* 显式丢弃的内容 */
    /DISCARD/ :
    {
        /* 丢弃C++异常回溯表 */
        *(.ARM.extab* .gnu.linkonce.armextab.*)
        *(.ARM.exidx*)

        /* 丢弃编译器自带的TLS占位符 */
        *(.tdata*)
        *(.tbss*)

        /* 丢弃C标准库和数学库 */
        libc.a:* (*)
        libm.a:* (*)
    }
}