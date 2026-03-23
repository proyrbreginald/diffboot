#include <load/load.h>
#include <main.h>
#include <rthw.h>
#include <rtthread.h>
#include <string.h>
#include <ymodem.h>

// 配置调试日志
#define DBG_TAG __FILE_NAME__
#define DBG_LVL DBG_WARN
#include <rtdebug.h>

// void detect_app(void)
// {
//     // 待加载的app程序地址
//     uint32_t app_bin_addr;

//     // 校验跳转标志位
//     switch (*(volatile uint32_t *)MCU_BKPRAM_START)
//     {
//     case LOAD_USER_CHECKSUM:
//         // 赋值user程序分区的地址作为app程序地址
//         app_bin_addr = USER_START;
//         LOG_I("LOAD_USER_CHECKSUM");
//         break;
//     case LOAD_OEM_CHECKSUM:
//         // 赋值oem程序分区的地址作为app程序地址
//         app_bin_addr = OEM_START;
//         LOG_I("LOAD_OEM_CHECKSUM");
//         break;
//     default:
//         // 无效参数时不加载app程序
//         LOG_I("checksum invalid");
//         return;
//     }

//     // 获取app的栈指针和复位处理函数
//     // app程序的第一个4字节是栈地址，第二个是复位处理函数地址
//     const uint32_t new_msp = *(volatile uint32_t *)app_bin_addr;
//     const void_fn_void_t new_reset_handler =
//         (void_fn_void_t)(*(volatile uint32_t *)(app_bin_addr + 4));
//     LOG_I("VTOR: 0x%08x, new_msp: 0x%08x, new_reset_handler: 0x%08x",
//           app_bin_addr, new_msp, new_reset_handler);

//     // 执行mcu软件复位
//     NVIC_SystemReset();
// }

/**
 * @brief bootloader线程。
 * @param parameter 线程名称字符串。
 */
static void boot_thread_entry(void *parameter)
{
    {
        const load_error_t load_error = load_get_error();
        if (load_error != LOAD_ERROR_NONE)
        {
            LOG_E("load error: %d, which: 0x%02x, r_crc: 0x%04x, c_crc: 0x%04x",
                  load_error, load_read_config_which(), load_read_config_crc(),
                  load_update_config_crc());
        }
    }
    while (1)
    {
        LOG_D("<thread:%s> running", parameter);
        HAL_GPIO_TogglePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin);
        if (load_verify_config())
        {
            if ((load_read_config_which() == LOAD_APP_USER) ||
                (load_read_config_which() == LOAD_APP_OEM))
            {
                LOG_F("reset by software");
                rt_thread_mdelay(500);
                NVIC_SystemReset();
            }
        }
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