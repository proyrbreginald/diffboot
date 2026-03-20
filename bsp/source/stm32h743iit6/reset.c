#include <load/load.h>
#include <main.h>
#include <mcu.h>

NONE void fpu_init(void)
{
    SCB->CPACR |= ((3UL << (10 * 2)) | (3UL << (11 * 2)));
}

NONE void rcc_init(void)
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

NONE void ram_init(void)
{
    RCC->AHB2ENR |=
        (RCC_AHB2ENR_D2SRAM1EN | RCC_AHB2ENR_D2SRAM2EN | RCC_AHB2ENR_D2SRAM3EN);
    volatile uint32_t tmpreg = RCC->AHB2ENR;
    (void)tmpreg;

    __HAL_RCC_BKPRAM_CLK_ENABLE();
    PWR->CR1 |= PWR_CR1_DBP;
}

/**
 * @brief 复位处理函数，配置程序运行所需的环境，
 * 包括设置栈初始指针、开启mcu内核相关配置、初始化ram数据、
 * 根据条件跳转到指定程序开始执行。
 */
NAKED void reset_handler(void)
{
    // 设置初始栈指针
    __set_MSP((uint32_t)_stack_start); //!< 执行后可以正常使用栈空间

    // 调用初始化函数
    fpu_init(); //!< 开启fpu执行权限
    rcc_init(); //!< 配置默认的系统时钟树
    ram_init(); //!< 开启所有ram区域

    /* 加载itcm的数据 */
    reset_copy_ram_init((char *)_itcm_ram_start, _itcm_section_addr,
                        (size_t)_itcm_ram_end -
                            (size_t)_itcm_ram_start); //!< 拷贝itcm的数据

#if defined(BUILD_LOADER)
    // 尝试启动app程序
    load_app(); //!< 所有app不满足启动要求则返回并开始启动boot
#endif

    /* 加载dtcm的数据 */
    reset_copy_ram_init(
        (char *)_dtcm_ram_init_start, _dtcm_ram_section_addr,
        (size_t)_dtcm_ram_init_end -
            (size_t)_dtcm_ram_init_start); //!< 拷贝dtcm的有值数据
    reset_clear_ram_uninit((char *)_dtcm_ram_uninit_start,
                           (char *)_dtcm_ram_uninit_end); //!< 清空dtcm的无值数据

    /* 加载axiram的数据 */
    reset_copy_ram_init(
        (char *)_axi_ram_init_start, _axi_ram_section_addr,
        (size_t)_axi_ram_init_end -
            (size_t)_axi_ram_init_start); //!< 拷贝axiram的有值数据
    reset_clear_ram_uninit((char *)_axi_ram_uninit_start,
                           (char *)_axi_ram_uninit_end); //!< 清空axiram的无值数据

    /* 加载ahbram的数据 */
    reset_copy_ram_init(
        (char *)_ahb_ram_init_start, _ahb_ram_section_addr,
        (size_t)_ahb_ram_init_end -
            (size_t)_ahb_ram_init_start); //!< 拷贝ahbram的有值数据
    reset_clear_ram_uninit((char *)_ahb_ram_uninit_start,
                           (char *)_ahb_ram_uninit_end); //!< 清空ahbram的无值数据

    // 程序数据已经加载完成，开始启动boot程序
    load_boot();

    // 正常不会执行到这里
    default_handler(); //!< 进入默认处理函数，执行死循环
}