#include <algo/algo.h>
#include <launch.h>
#include <load/load.h>
#include <main.h>
#include <mcu.h>

SHARE static load_config_t load_config;

bool load_verify_config(void)
{
    const uint16_t c_crc =
        algo_crc16((uint8_t *)&load_config, sizeof(load_config_info_t));
    if (c_crc != load_config.crc)
    {
        load_set_error(LOAD_ERROR_VERIFY);
        return false;
    }
    else
    {
        return true;
    }
}

uint16_t load_update_config_crc(void)
{
    load_config.crc =
        algo_crc16((uint8_t *)&load_config, sizeof(load_config_info_t));
    return load_config.crc;
}

uint16_t load_read_config_crc(void)
{
    return load_config.crc;
}

void load_set_error(load_error_t error)
{
    if (load_config.info.error != error)
    {
        load_config.info.error = error;
        load_update_config_crc(); //!< 更新校验值
    }
}

void load_clear_error(void)
{
    if (load_config.info.error != LOAD_ERROR_INVALID)
    {
        load_config.info.error = LOAD_ERROR_INVALID;
        load_update_config_crc(); //!< 更新校验值
    }
}

load_error_t load_get_error(void)
{
    return load_config.info.error;
}

void load_set_reset(void)
{
    if (load_config.info.reset != LOAD_RESET)
    {
        load_config.info.reset = LOAD_RESET;
        load_update_config_crc(); //!< 更新校验值
    }
}

void load_clear_reset(void)
{
    if (load_config.info.reset != LOAD_RESET_INVALID)
    {
        load_config.info.reset = LOAD_RESET_INVALID;
        load_update_config_crc(); //!< 更新校验值
    }
}

load_reset_t load_get_reset(void)
{
    return load_config.info.reset;
}

void load_write_config_which(load_which_t which)
{
    if (load_config.info.which != which)
    {
        load_config.info.which = which;
        load_update_config_crc(); //!< 更新校验值
    }
}

bool load_read_config_which(load_which_t *which)
{
    if (!which)
    {
        return false;
    }

    if (load_verify_config())
    {
        *which = load_config.info.which;
    }

    return true;
}

void load_set_apply(load_apply_t apply)
{
    if (load_config.info.apply != apply)
    {
        load_config.info.apply = apply;
        load_update_config_crc(); //!< 更新校验值
    }
}

void load_clear_apply(void)
{
    if (load_config.info.apply != LOAD_APPLY_INVALID)
    {
        load_config.info.apply = LOAD_APPLY_INVALID;
        load_update_config_crc(); //!< 更新校验值
    }
}

load_apply_t load_get_apply(void)
{
    return load_config.info.apply;
}

void load_set_patch(load_patch_t patch)
{
    if (load_config.info.patch != patch)
    {
        load_config.info.patch = patch;
        load_update_config_crc(); //!< 更新校验值
    }
}

void load_clear_patch(void)
{
    if (load_config.info.patch != LOAD_PATCH_INVALID)
    {
        load_config.info.patch = LOAD_PATCH_INVALID;
        load_update_config_crc(); //!< 更新校验值
    }
}

load_patch_t load_get_patch(void)
{
    return load_config.info.patch;
}

void load_set_patch_size(uint32_t patch_size)
{
    if (load_config.info.patch_size != patch_size)
    {
        load_config.info.patch_size = patch_size;
        load_update_config_crc(); //!< 更新校验值
    }
}

uint32_t load_get_patch_size(void)
{
    return load_config.info.patch_size;
}

void load_app(void)
{
    load_clear_error(); //!< 清除复位前设置的错误码
    load_clear_reset(); //!< 清除复位前设置的复位需求

    uint32_t app_bin_addr; //!< 待启动的app程序地址
    load_which_t which;
    if (!load_read_config_which(&which))
    {
        return;
    }
    switch (which) //!< 读取应该启动哪个程序
    {
    case LOAD_APP_LOADER:
        return; //!< 从boot启动
    case LOAD_APP_USER:
        app_bin_addr = USER_START; //!< 从用户程序启动
        break;
    case LOAD_APP_OEM:
        app_bin_addr = OEM_START; //!< 从厂商程序启动
        break;
    default:
        load_set_error(LOAD_ERROR_WHICH);
        return; //!< 无效参数时不加载app程序
    }

    load_write_config_which(LOAD_APP_INVALID); //!< 清除启动配置

    const uint32_t new_msp =
        *(volatile uint32_t *)app_bin_addr; //!< 获取app的栈指针
    const void_fn_void_t new_reset_handler = (void_fn_void_t)(*(
        volatile uint32_t *)(app_bin_addr + 4)); //!< 获取app的复位处理函数

    __disable_irq();          //!< 防止在跳转过程中被中断打断
    SCB->VTOR = app_bin_addr; //!< 设置向量表偏移
    __set_MSP(new_msp);       //!< 更新到app的初始栈指针
    new_reset_handler();      //!< 执行跳转
}

void load_boot(void)
{
    main();            //!< 配置时钟树与外设
    rtthread_launch(); //!< 启动rtthread
}