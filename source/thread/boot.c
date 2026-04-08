#include <detools_port.h>
#include <load/load.h>
#include <main.h>
#include <rthw.h>
#include <rtthread.h>
#include <string.h>
#include <ymodem.h>

// 配置调试日志
#define DBG_TAG __FILE_NAME__
#define DBG_LVL DBG_DEBUG
#include <rtdebug.h>

ITCM void erase_user(uint32_t patch_size)
{
    int result = 0;
    FLASH_EraseInitTypeDef flash_erase_configuration = {
        .TypeErase = FLASH_TYPEERASE_SECTORS,
        .Banks = FLASH_BANK_1,
        .Sector = (USER_START - MCU_FLASH_START) / MCU_FLASH_SECTOR_SIZE,
        .NbSectors = (1 + patch_size / MCU_FLASH_SECTOR_SIZE),
        .VoltageRange = FLASH_VOLTAGE_RANGE_3,
    };

    LOG_D("flash erase sector index: %u, number: %u",
          flash_erase_configuration.Sector,
          flash_erase_configuration.NbSectors);

    // 在擦写前关闭全局中断
    __disable_irq();

    // 解锁Flash控制寄存器
    result = HAL_FLASH_Unlock();
    if (0 != result)
    {
        LOG_E("flash unlock fail");
        return;
    }

    // 清除ECC标志
    __HAL_FLASH_CLEAR_FLAG_BANK1(FLASH_FLAG_ALL_ERRORS_BANK1);

    // 执行擦除
    uint32_t sector_error;
    result = HAL_FLASHEx_Erase(&flash_erase_configuration, &sector_error);
    if (0 != result)
    {
        LOG_E("flash erase fail with %u", sector_error);
        return;
    }

    // 上锁Flash控制寄存器
    result = HAL_FLASH_Lock();
    if (0 != result)
    {
        LOG_E("flash lock fail");
    }

    // 使能全局中断
    __enable_irq();

    LOG_E("flash user success");
}

ITCM void erase_oem(uint32_t patch_size)
{
    int result = 0;
    FLASH_EraseInitTypeDef flash_erase_configuration = {
        .TypeErase = FLASH_TYPEERASE_SECTORS,
        .Banks = FLASH_BANK_2,
        .Sector = (OEM_START - MCU_FLASH_START) / MCU_FLASH_SECTOR_SIZE - 8,
        .NbSectors = (1 + patch_size / MCU_FLASH_SECTOR_SIZE),
        .VoltageRange = FLASH_VOLTAGE_RANGE_3,
    };

    LOG_D("flash erase sector index: %u, number: %u",
          flash_erase_configuration.Sector,
          flash_erase_configuration.NbSectors);

    // 在擦写前关闭全局中断
    __disable_irq();

    // 解锁Flash控制寄存器
    result = HAL_FLASH_Unlock();
    if (0 != result)
    {
        LOG_E("flash unlock fail");
        return;
    }

    // 清除ECC标志
    __HAL_FLASH_CLEAR_FLAG_BANK2(FLASH_FLAG_ALL_ERRORS_BANK2);

    // 执行擦除
    uint32_t sector_error;
    result = HAL_FLASHEx_Erase(&flash_erase_configuration, &sector_error);
    if (0 != result)
    {
        LOG_E("flash erase fail with %u", sector_error);
        return;
    }

    // 上锁Flash控制寄存器
    result = HAL_FLASH_Lock();
    if (0 != result)
    {
        LOG_E("flash lock fail");
    }

    // 使能全局中断
    __enable_irq();

    LOG_I("flash oem success");
}

ITCM void detect_apply(void)
{
    uint32_t old_app_addr;
    uint32_t patch_addr;
    uint32_t patch_size;
    uint32_t new_app_addr;
    const load_apply_t apply = load_get_apply();
    switch (apply)
    {
    case LOAD_APPLY_USER:
        LOG_I("LOAD_APPLY_OEM erase");
        old_app_addr = OEM_START;
        patch_addr = PATCH_START;
        patch_size = load_get_patch_size();
        new_app_addr = USER_START;
        erase_user(patch_size);
        break;
    case LOAD_APPLY_OEM:
        LOG_I("LOAD_APPLY_OEM erase");
        old_app_addr = USER_START;
        patch_addr = PATCH_START;
        patch_size = load_get_patch_size();
        new_app_addr = OEM_START;
        erase_oem(patch_size);
        break;
    default:
        LOG_I("LOAD_APPLY_INVALID");
        return;
    }

    const int result =
        detools_apply_patch(old_app_addr, patch_addr, patch_size, new_app_addr);
    if (result == DETOOLS_OK)
    {
        switch (apply)
        {
        case LOAD_APPLY_USER:
            load_write_config_which(LOAD_APP_USER);
            LOG_I("LOAD_APPLY_USER apply");
            break;
        case LOAD_APPLY_OEM:
            load_write_config_which(LOAD_APP_OEM);
            LOG_I("LOAD_APPLY_OEM apply");
            break;
        default:
            LOG_I("LOAD_APPLY_INVALID apply");
            return;
        }
        load_clear_apply();
        load_set_reset();
    }
}

ITCM void detect_reset(void)
{
    if (load_get_reset() != LOAD_RESET)
    {
        return;
    }

    load_which_t which;
    if (!load_read_config_which(&which))
    {
        return;
    }

    // 待加载的app程序地址
    uint32_t app_bin_addr;

    // 校验跳转标志位
    switch (which)
    {
    case LOAD_APP_USER:
        // 赋值user程序分区的地址作为app程序地址
        app_bin_addr = USER_START;
        LOG_I("detect user app");
        break;
    case LOAD_APP_OEM:
        // 赋值oem程序分区的地址作为app程序地址
        app_bin_addr = OEM_START;
        LOG_I("detect oem app");
        break;
    default:
        // 无效参数时不加载app程序
        return;
    }
    LOG_F("reset by software");

    // 获取app的栈指针和复位处理函数
    // app程序的第一个4字节是栈地址，第二个是复位处理函数地址
    const uint32_t new_msp = *((volatile uint32_t *)app_bin_addr);
    const void_fn_void_t new_reset_handler =
        (void_fn_void_t)(*((volatile uint32_t *)(app_bin_addr + 4)));
    LOG_I("VTOR: 0x%08x, new_msp: 0x%08x, new_reset_handler: 0x%08x",
          app_bin_addr, new_msp, new_reset_handler);
    UNUSE_VAR(new_msp);
    UNUSE_VAR(new_reset_handler);

    // 执行mcu软件复位
    rt_thread_mdelay(500);
    NVIC_SystemReset();
}

/**
 * @brief bootloader线程。
 * @param parameter 线程名称字符串。
 */
ITCM static void boot_thread_entry(void *parameter)
{
#if defined(BUILD_LOADER)
    LOG_D("launch from boot");
#elif defined(BUILD_USER)
    LOG_D("launch from user");
#elif defined(BUILD_OEM)
    LOG_D("launch from oem");
#else
    LOG_F("lost BUILD_XXX define");
#endif
    while (1)
    {
        LOG_D("<thread:%s> running", parameter);
        HAL_GPIO_TogglePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin);
        detect_apply();
        detect_reset();
        rt_thread_mdelay(500);
    }
}

/**
 * @brief 启动bootloader线程。
 * @return int 失败返回非0。
 */
static int boot_thread_init(void)
{
    const char *const name = "boot";
    rt_err_t result = RT_EOK;
    rt_thread_t tid =
        rt_thread_create(name, boot_thread_entry, (void *)name, 1024 * 2, 1, 0);
    if (tid != NULL)
    {
        LOG_D("<thread:%s> create success", name);
        result = rt_thread_startup(tid);
        if (result == RT_EOK)
        {
            LOG_D("<thread:%s> startup success", name);
        }
        else
        {
            LOG_D("<thread:%s> startup fail with %d", name, result);
        }
    }
    else
    {
        result = RT_ENOMEM;
        LOG_D("<thread:%s> create fail", name);
    }
    return result;
}
RUN_APP_EXPORT(boot_thread_init);