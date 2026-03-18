#include <main.h>
#include <mcu/mcu.h>
#include <reset/start.h>
#include <startup.h>
#include <load/load.h>

void load_app(void)
{
    // 待加载的app程序地址
    uint32_t app_bin_addr;

    // 校验跳转标志位
    switch (*(volatile uint32_t *)MCU_BKPRAM_START)
    {
    case LOAD_USER_CHECKSUM:
        // 赋值user程序分区的地址作为app程序地址
        app_bin_addr = USER_START;
        break;
    case LOAD_OEM_CHECKSUM:
        // 赋值oem程序分区的地址作为app程序地址
        app_bin_addr = OEM_START;
        break;
    default:
        // 无效参数时不加载app程序
        return;
    }

    // 清除跳转标志位
    *(volatile uint32_t *)MCU_BKPRAM_START = 0;

    // 获取app的栈指针和复位处理函数
    // app程序的第一个4字节是栈地址，第二个是复位处理函数地址
    const uint32_t new_msp = *(volatile uint32_t *)app_bin_addr;
    const void_fn_void_t new_reset_handler =
        (void_fn_void_t)(*(volatile uint32_t *)(app_bin_addr + 4));

    // 防止在跳转过程中被中断打断
    __disable_irq();

    // 设置向量表偏移
    SCB->VTOR = app_bin_addr;

    // 设置cpu的栈指针
    __set_MSP(new_msp);

    // 执行跳转
    new_reset_handler();
}

void load_boot(void)
{
    // 配置mcu与外设
    main();

    // 加载rtthread
    rtthread_startup();
}