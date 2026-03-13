/**
  * @brief 包含预处理指令的通用链接脚本。
  * @warning 请使用GCC进行预处理后生成的版本。
  */

#include <mcu/partition.h>

/* MCU内存分区 */
MEMORY
{
    LOADER  (rwx) : ORIGIN = LOADER_START,      LENGTH = LOADER_SIZE
    CONFIG  (rw)  : ORIGIN = CONFIG_START,      LENGTH = CONFIG_SIZE
    USER    (rwx) : ORIGIN = USER_START,        LENGTH = USER_SIZE
    PATCH   (rw)  : ORIGIN = PATCH_START,       LENGTH = PATCH_SIZE
    OEM     (rwx) : ORIGIN = OEM_START,         LENGTH = OEM_SIZE

    ITCM    (rwx) : ORIGIN = MCU_ITCM_START,    LENGTH = MCU_ITCM_SIZE
    DTCM    (rwx) : ORIGIN = MCU_DTCM_START,    LENGTH = MCU_DTCM_SIZE
    RAM0    (rwx) : ORIGIN = MCU_RAM0_START,    LENGTH = MCU_RAM0_SIZE
    RAM1    (rwx) : ORIGIN = MCU_RAM1_START,    LENGTH = MCU_RAM1_SIZE
    RAM2    (rwx) : ORIGIN = MCU_RAM2_START,    LENGTH = MCU_RAM2_SIZE
}

/* 栈底地址 */
stack_start = ORIGIN(DTCM) + LENGTH(DTCM);

/* 栈大小 */
stack_size = 0x400;

/* 堆结束地址 */
heap_end = stack_start - stack_size;

/* 根据构建参数匹配不同的段分配布局 */
#ifdef BUILD_LOADER
SECTIONS
{
    /* 普通代码段 */
    .code : ALIGN(4)
    {
        KEEP(*(.isr_vector))    /* 中断向量表 */
        *(.text*)               /* 普通代码 */
        *(.glue_7*)             /* ARM/Thumb胶水代码 */
        KEEP(*(SORT(.rti_fn*))) /* RTOS启动时自动执行 */
    } > LOADER

    /* 快速代码段 */
    .fast : ALIGN(4)
    {
        _fast_ram_start = .;    /* 起始地址 */
        KEEP(*(.fast*))         /* 快速代码 */
        _fast_ram_end = .;      /* 结束地址 */
    } > ITCM AT > LOADER
    _fast_section_addr = LOADADDR(.fast);

    /* 常量数据段 */
    .const : ALIGN(4)
    {
        *(.rodata*) /* 常量数据 */
    } > LOADER

    /* DTCM已初始化数据段 */
    .dtcm_init : ALIGN(4)
    {
        _dtcm_init_start = .;   /* 起始地址 */
        *(.data*)               /* 已初始化数据 */
        _dtcm_init_end = .;     /* 结束地址 */
    } > DTCM AT > LOADER
    _assigned_section_addr = LOADADDR(.dtcm_init);

    /* DTCM未初始化数据段 */
    .dtcm_uninit : ALIGN(4)
    {
        _dtcm_uninit_start = .; /* 起始地址 */
        *(.bss*)                /* 未初始化数据 */
        _dtcm_uninit_end = .;   /* 结束地址 */
    } > DTCM

    /* 堆空间段 */
    .heap : ALIGN(8)
    {
        _heap_start = .;
    } > DTCM

    /* 栈空间段 */
    .stack : ALIGN(8)
    {
        . = . + stack_size; /* 如果RAM不足会报错 */
    } > DTCM

    /* RAM0已初始化数据段 */
    .ram0_init : ALIGN(4)
    {
        _ram0_init_start = .;   /* 起始地址 */
        *(.share*)              /* 已初始化数据 */
        _ram0_init_end = .;     /* 结束地址 */
    } > RAM0 AT > LOADER
    _ram0_section_addr = LOADADDR(.ram0_init);

    /* RAM0未初始化数据段 */
    .ram0_uninit : ALIGN(4)
    {
        _ram0_uninit_start = .; /* 起始地址 */
        *(.bss*)                /* 未初始化数据 */
        _ram0_uninit_end = .;   /* RAM中的结束地址 */
    } > RAM0

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
#elif BUILD_USER
SECTIONS
{
    /* 普通代码段 */
    .code : ALIGN(4)
    {
        KEEP(*(.isr_vector))    /* 中断向量表 */
        *(.text*)               /* 普通代码 */
        *(.glue_7*)             /* ARM/Thumb胶水代码 */
        KEEP(*(SORT(.rti_fn*))) /* RTOS启动时自动执行 */
    } > USER

    /* 快速代码段 */
    .fast : ALIGN(4)
    {
        _fast_ram_start = .;    /* 起始地址 */
        KEEP(*(.fast*))         /* 快速代码 */
        _fast_ram_end = .;      /* 结束地址 */
    } > ITCM AT > USER
    _fast_section_addr = LOADADDR(.fast);

    /* 常量数据段 */
    .const : ALIGN(4)
    {
        *(.rodata*) /* 常量数据 */
    } > USER

    /* DTCM已初始化数据段 */
    .dtcm_init : ALIGN(4)
    {
        _dtcm_init_start = .;   /* 起始地址 */
        *(.data*)               /* 已初始化数据 */
        _dtcm_init_end = .;     /* 结束地址 */
    } > DTCM AT > USER
    _assigned_section_addr = LOADADDR(.dtcm_init);

    /* DTCM未初始化数据段 */
    .dtcm_uninit : ALIGN(4)
    {
        _dtcm_uninit_start = .; /* 起始地址 */
        *(.bss*)                /* 未初始化数据 */
        _dtcm_uninit_end = .;   /* 结束地址 */
    } > DTCM

    /* 堆空间段 */
    .heap : ALIGN(8)
    {
        _heap_start = .;
    } > DTCM

    /* 栈空间段 */
    .stack : ALIGN(8)
    {
        . = . + stack_size; /* 如果RAM不足会报错 */
    } > DTCM

    /* RAM0已初始化数据段 */
    .ram0_init : ALIGN(4)
    {
        _ram0_init_start = .;   /* 起始地址 */
        *(.share*)              /* 已初始化数据 */
        _ram0_init_end = .;     /* 结束地址 */
    } > RAM0 AT > USER
    _ram0_section_addr = LOADADDR(.ram0_init);

    /* RAM0未初始化数据段 */
    .ram0_uninit : ALIGN(4)
    {
        _ram0_uninit_start = .; /* 起始地址 */
        *(.bss*)                /* 未初始化数据 */
        _ram0_uninit_end = .;   /* RAM中的结束地址 */
    } > RAM0

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
#elif BUILD_OEM
#else
/* 无有效参数时报错 */
#error "BUILD_LOADER, BUILD_USER or BUILD_OEM must be defined"
#endif