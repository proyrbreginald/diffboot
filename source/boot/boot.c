#include <load/load.h>
#include <main.h>
#include <mcu.h>
#include <rthw.h>
#include <rtthread.h>
#include <string.h>
#include <ymodem.h>

// 配置调试日志
#define DBG_TAG __FILE_NAME__
#define DBG_LVL DBG_VERBOSE
#include <rtdebug.h>

#undef THREAD_NAME
#define THREAD_NAME "count"
// cpu使用率统计线程

static void count_thread_entry(void *parameter)
{
    // 将毫秒转换为系统的Tick数
    const rt_tick_t period_tick = rt_tick_from_millisecond(1000);

    // 获取进入循环前的当前Tick时间作为基准
    rt_tick_t last_wakeup_tick = rt_tick_get();

    while (1)
    {
        /* 取出数据并清零 */
        rt_enter_critical();
        uint64_t sleep_cnt = rt_idle_total_sleep_get();
        rt_idle_total_sleep_clear();
        rt_exit_critical();

        /* 理论上1秒钟LPTIM应该走1000000个Tick */
        if (sleep_cnt > 1000000)
        {
            sleep_cnt = 1000000; // 修正误差，防止出现负数
        }

        static uint8_t cnt = 0;
        if (++cnt % 1 == 0)
        {
            /* 占用率 = 100 - (睡眠占比) */
            LOG_I("cpu usage per s: %.2f%%",
                  100.0f - ((float)sleep_cnt / 1000000) * 100.0f);
            cnt = 0;
        }

        /*
         * 阻塞延时至下一个绝对时间点
         * 内核会自动计算：需要sleep多久 = (last_wakeup_tick + period_tick) -
         * current_tick 并自动更新 last_wakeup_tick
         */
        rt_thread_delay_until(&last_wakeup_tick, period_tick);
    }
}

// 启动计算线程
static int count_thread_init(void)
{
    rt_err_t result = RT_EOK;
    rt_thread_t tid = rt_thread_create(THREAD_NAME, count_thread_entry, NULL,
                                       1024 * 2, RT_THREAD_PRIORITY_MAX - 2, 0);
    if (tid != NULL)
    {
        LOG_I(THREAD_NAME " thread create success");
        result = rt_thread_startup(tid);
        if (result == RT_EOK)
        {
            LOG_I(THREAD_NAME " thread startup success");
        }
        else
        {
            LOG_E(THREAD_NAME " thread startup fail with %d", result);
        }
    }
    else
    {
        result = RT_ENOMEM;
        LOG_E(THREAD_NAME " thread create fail");
    }
    return result;
}
RUN_APP_EXPORT(count_thread_init);

typedef enum
{
    IAP_USER,
    IAP_OEM,
} iap_zone_t;

static iap_zone_t iap_zone = IAP_USER;

/**
 * @brief ymodem接收到文件头时执行的回调。
 * @param name 接收到的文件名字符串。
 * @param size 文件内容的字节大小。
 * @return int 返回0表示继续接收此文件，否则拒绝接收。
 */
ITCM static int ymodem_on_begin(const char *name, uint32_t size)
{
    int result = 0;
    FLASH_EraseInitTypeDef flash_erase_configuration = {
        .TypeErase = FLASH_TYPEERASE_SECTORS,
        .NbSectors = (1 + size / MCU_FLASH_SECTOR_SIZE),
        .VoltageRange = FLASH_VOLTAGE_RANGE_3,
    };
    if (strcmp(name, "user.bin") == 0)
    {
        flash_erase_configuration.Banks = FLASH_BANK_1;
        flash_erase_configuration.Sector =
            (USER_START - MCU_FLASH_START) / MCU_FLASH_SECTOR_SIZE;
        iap_zone = IAP_USER;
    }
    else if (strcmp(name, "oem.bin") == 0)
    {
        flash_erase_configuration.Banks = FLASH_BANK_2;
        flash_erase_configuration.Sector =
            OEM_START / MCU_FLASH_SECTOR_SIZE - 8;
        iap_zone = IAP_OEM;
    }
    else
    {
        LOG_E("unsupport file: %s (%d bytes)", name, size);
        goto exit;
    }
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
        goto exit;
    }

    // 清除ECC标志
    __HAL_FLASH_CLEAR_FLAG_BANK1(
        (flash_erase_configuration.Banks == FLASH_BANK_1)
            ? FLASH_FLAG_ALL_ERRORS_BANK1
            : FLASH_FLAG_ALL_ERRORS_BANK2);

    // 执行擦除
    uint32_t sector_error;
    result = HAL_FLASHEx_Erase(&flash_erase_configuration, &sector_error);
    if (0 != result)
    {
        LOG_E("flash erase fail with %u", sector_error);
        goto exit;
    }

    LOG_I("start download: %s (%d bytes)", name, size);

exit:
    // 上锁Flash控制寄存器
    result = HAL_FLASH_Lock();
    if (0 != result)
    {
        LOG_E("flash lock fail");
    }

    // 使能全局中断
    __enable_irq();

    return 0;
}

/**
 * @brief ymodem接收到文件数据包时执行的回调。
 * @param data 数据指针。
 * @param len 数据长度。
 * @param offset 当前文件偏移量。
 * @return int 0: 处理成功; 非0: 错误 (例如写入失败)，将终止传输。
 */
ITCM static int ymodem_on_data(const uint8_t *data, uint32_t len,
                               uint32_t offset)
{
    int result = 0;

    const uint32_t addr =
        ((iap_zone == IAP_USER) ? USER_START : OEM_START) + offset;
    // 检查地址是否32字节对齐
    if ((addr & 31) != 0)
    {
        LOG_E("flash address 0x%08X not 32-byte aligned", addr);
        goto exit;
    }
    LOG_D("flash program from 0x%08X", addr);

    // 在写前关闭全局中断
    __disable_irq();

    // 解锁Flash控制寄存器
    result = HAL_FLASH_Unlock();
    if (0 != result)
    {
        LOG_E("flash unlock fail");
        goto exit;
    }

    // 清除ECC标志
    __HAL_FLASH_CLEAR_FLAG_BANK1((iap_zone == IAP_USER)
                                     ? FLASH_FLAG_ALL_ERRORS_BANK1
                                     : FLASH_FLAG_ALL_ERRORS_BANK2);

    uint32_t bytes_processed = 0;
    static uint8_t buffer[32] __attribute__((aligned(32)));

    while (bytes_processed < len)
    {
        uint32_t remaining = len - bytes_processed;
        uint32_t write_addr = addr + bytes_processed;
        uint8_t *src_ptr = (uint8_t *)(data + bytes_processed);

        if (remaining >= 32)
        {
            // 情况1:剩余数据足够写一个完整的32字节块
            // 注意：如果源数据地址本身就是32位对齐的，可以直接传src_ptr
            // 否则为了安全，如果src_ptr不对齐，建议先copy到align_buffer
            if (((uint32_t)src_ptr & 31) == 0)
            {
                result = HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD,
                                           write_addr, (uint32_t)src_ptr);
            }
            else
            {
                memcpy(buffer, src_ptr, 32);
                result = HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD,
                                           write_addr, (uint32_t)buffer);
            }
            bytes_processed += 32;
        }
        else
        {
            // 情况2: 剩余数据不足32字节，进行填充
            memset(buffer, 0xFF, 32);           // 先全部填0xFF
            memcpy(buffer, src_ptr, remaining); // 拷入剩余数据

            result = HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, write_addr,
                                       (uint32_t)buffer);
            bytes_processed += remaining; // 处理完成
        }

        if (result != HAL_OK)
        {
            LOG_E("flash program fail at 0x%08X with %u", write_addr, result);
            goto exit;
        }
    }

exit:
    // 上锁Flash控制寄存器
    result = HAL_FLASH_Lock();
    if (0 != result)
    {
        LOG_E("flash lock fail");
    }

    // 使能全局中断
    __enable_irq();

    return 0;
}

/**
 * @brief 传输结束。
 * @param status 0: 成功; 非0: 异常终止 (超时、校验失败、被取消等)
 */
ITCM static void ymodem_on_end(int status)
{
    if (status == 0)
    {
        LOG_I("download success!");

        uint8_t buffer[8] = {0};
        const uint32_t addr = ((iap_zone == IAP_USER) ? USER_START : OEM_START);
        memcpy(buffer, (void *)addr, sizeof(buffer));

        uint32_t stack = (uint32_t)buffer[3] << (3 * 8);
        stack |= (uint32_t)buffer[2] << (2 * 8);
        stack |= (uint32_t)buffer[1] << (1 * 8);
        stack |= (uint32_t)buffer[0] << (0 * 8);

        uint32_t reset = (uint32_t)buffer[7] << (3 * 8);
        reset |= (uint32_t)buffer[6] << (2 * 8);
        reset |= (uint32_t)buffer[5] << (1 * 8);
        reset |= (uint32_t)buffer[4] << (0 * 8);

        // 设置加载app标志位
        // *(volatile uint32_t *)MCU_BKPRAM_START =
        //     ((iap_zone == IAP_USER) ? LOAD_USER_CHECKSUM :
        //     LOAD_OEM_CHECKSUM);

        LOG_I("stack: 0x%08x, reset: 0x%08x, checksum: 0x%08x", stack, reset,
              *(volatile uint32_t *)MCU_BKPRAM_START);
    }
    else
    {
        LOG_E("download failed, error code: %d", status);
    }
}

// 组装接口对象
static ymodem_ops_t ymodem_ops = {
    .on_begin = ymodem_on_begin,
    .on_data = ymodem_on_data,
    .on_end = ymodem_on_end,
};

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

#undef THREAD_NAME
#define THREAD_NAME "boot"
// 差分启动线程
static void boot_thread_entry(void *parameter)
{
    // 在初始化时注册
    ymodem_set_ops(&ymodem_ops);
    LOG_I("set ymodem ops");

    while (1)
    {
        LOG_D(THREAD_NAME " thread running");
        HAL_GPIO_TogglePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin);
        rt_thread_mdelay(500);
    }
}

// 启动差分启动线程
static int boot_thread_init(void)
{
    rt_err_t result = RT_EOK;
    rt_thread_t tid =
        rt_thread_create(THREAD_NAME, boot_thread_entry, NULL, 1024 * 2, 1, 0);
    if (tid != NULL)
    {
        LOG_I(THREAD_NAME " thread create success");
        result = rt_thread_startup(tid);
        if (result == RT_EOK)
        {
            LOG_I(THREAD_NAME " thread startup success");
        }
        else
        {
            LOG_E(THREAD_NAME " thread startup fail with %d", result);
        }
    }
    else
    {
        result = RT_ENOMEM;
        LOG_E(THREAD_NAME " thread create fail");
    }
    return result;
}
RUN_APP_EXPORT(boot_thread_init);

// 创建业务层任务
void rt_app_init(void)
{
    LOG_I("start to init app");
}