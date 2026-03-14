#include <main.h>
#include <mcu/partition.h>
#include <reset/start.h>
#include <startup.h>
#include <stm32h7xx.h>

/**
 * @brief 开启FPU权限。
 *
 */
void fpu_init(void)
{
    SCB->CPACR |= ((3UL << (10 * 2)) | (3UL << (11 * 2)));
}

/**
 * @brief 复位时钟系统。
 *
 */
void rcc_init(void)
{
    if (FLASH_LATENCY_DEFAULT > (READ_BIT((FLASH->ACR), FLASH_ACR_LATENCY)))
    {
        MODIFY_REG(FLASH->ACR, FLASH_ACR_LATENCY,
                   (uint32_t)(FLASH_LATENCY_DEFAULT));
    }

    RCC->CR |= RCC_CR_HSION;
    RCC->CFGR = 0x00000000;
    RCC->CR &= 0xEAF6ED7FU;

    if (FLASH_LATENCY_DEFAULT < (READ_BIT((FLASH->ACR), FLASH_ACR_LATENCY)))
    {
        MODIFY_REG(FLASH->ACR, FLASH_ACR_LATENCY,
                   (uint32_t)(FLASH_LATENCY_DEFAULT));
    }

    RCC->D1CFGR = 0x00000000;
    RCC->D2CFGR = 0x00000000;
    RCC->D3CFGR = 0x00000000;

    RCC->PLLCKSELR = 0x02020200;
    RCC->PLLCFGR = 0x01FF0000;
    RCC->PLL1DIVR = 0x01010280;
    RCC->PLL1FRACR = 0x00000000;

    RCC->PLL2DIVR = 0x01010280;
    RCC->PLL2FRACR = 0x00000000;
    RCC->PLL3DIVR = 0x01010280;

    RCC->PLL3FRACR = 0x00000000;

    RCC->CR &= 0xFFFBFFFFU;

    RCC->CIER = 0x00000000;

    if ((DBGMCU->IDCODE & 0xFFFF0000U) < 0x20000000U)
    {
        *((__IO uint32_t *)0x51008108) = 0x000000001U;
    }

    if (READ_BIT(RCC->AHB3ENR, RCC_AHB3ENR_FMCEN) == 0U)
    {
        SET_BIT(RCC->AHB3ENR, RCC_AHB3ENR_FMCEN);
        FMC_Bank1_R->BTCR[0] = 0x000030D2;
        CLEAR_BIT(RCC->AHB3ENR, RCC_AHB3ENR_FMCEN);
    }
}

/**
 * @brief 开启RAM时钟与备份RAM。
 *
 */
void ram_init(void)
{
    RCC->AHB2ENR |=
        (RCC_AHB2ENR_D2SRAM1EN | RCC_AHB2ENR_D2SRAM2EN | RCC_AHB2ENR_D2SRAM3EN);
    volatile uint32_t tmpreg = RCC->AHB2ENR;
    (void)tmpreg;

    __HAL_RCC_BKPRAM_CLK_ENABLE();
    PWR->CR1 |= PWR_CR1_DBP;
}

/**
 * @brief 尝试加载app程序。
 *
 */
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

/**
 * @brief 加载boot程序。
 *
 */
void load_boot(void)
{
    // 配置内核与外设
    main();

    // 加载rtthread
    rtthread_startup();
}