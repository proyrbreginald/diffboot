#include <load/load.h>
#include <main.h>
#include <rthw.h>
#include <rtthread.h>
#include <string.h>
#include <ymodem.h>
#include <ymodem_port.h>

// 配置调试日志
#define DBG_TAG __FILE_NAME__
#define DBG_LVL DBG_VERBOSE
#include <rtdebug.h>

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
        load_write_config_which(LOAD_APP_USER);
    }
    else if (strcmp(name, "oem.bin") == 0)
    {
        flash_erase_configuration.Banks = FLASH_BANK_2;
        flash_erase_configuration.Sector =
            (OEM_START - MCU_FLASH_START) / MCU_FLASH_SECTOR_SIZE - 8;
        load_write_config_which(LOAD_APP_OEM);
    }
    else if (strcmp(name, "user.patch") == 0)
    {
        flash_erase_configuration.Banks = FLASH_BANK_2;
        flash_erase_configuration.Sector =
            (PATCH_START - MCU_FLASH_START) / MCU_FLASH_SECTOR_SIZE - 8;
        load_set_patch(LOAD_PATCH_USER);
        load_set_patch_size(size);
    }
    else if (strcmp(name, "oem.patch") == 0)
    {
        flash_erase_configuration.Banks = FLASH_BANK_2;
        flash_erase_configuration.Sector =
            (PATCH_START - MCU_FLASH_START) / MCU_FLASH_SECTOR_SIZE - 8;
        load_set_patch(LOAD_PATCH_OEM);
        load_set_patch_size(size);
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
    if (flash_erase_configuration.Banks == FLASH_BANK_1)
    {
        __HAL_FLASH_CLEAR_FLAG_BANK1(FLASH_FLAG_ALL_ERRORS_BANK1);
    }
    else
    {
        __HAL_FLASH_CLEAR_FLAG_BANK2(FLASH_FLAG_ALL_ERRORS_BANK2);
    }

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

    load_which_t which;
    if (!load_read_config_which(&which))
    {
        result = load_get_error();
        LOG_E("read load which fail with 0x%d", result);
        goto exit;
    }

    uint32_t addr;

    switch (which)
    {
    case LOAD_APP_USER:
        addr = USER_START + offset;
        break;
    case LOAD_APP_OEM:
        addr = OEM_START + offset;
        break;
    default:
        const load_patch_t patch = load_get_patch();
        switch (patch)
        {
        case LOAD_PATCH_USER:
        case LOAD_PATCH_OEM:
            addr = PATCH_START + offset;
            break;
        default:
            LOG_E("load patch error with %d", patch);
            goto exit;
        }
        break;
    }

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
    switch (which)
    {
    case LOAD_APP_USER:
        __HAL_FLASH_CLEAR_FLAG_BANK1(FLASH_FLAG_ALL_ERRORS_BANK1);
        break;
    case LOAD_APP_OEM:
        __HAL_FLASH_CLEAR_FLAG_BANK2(FLASH_FLAG_ALL_ERRORS_BANK2);
        break;
    default:
        const load_patch_t patch = load_get_patch();
        switch (patch)
        {
        case LOAD_PATCH_USER:
        case LOAD_PATCH_OEM:
            __HAL_FLASH_CLEAR_FLAG_BANK2(FLASH_FLAG_ALL_ERRORS_BANK2);
            break;
        default:
            LOG_E("load patch(%d) with error(%d)", patch, load_get_error());
            goto exit;
        }
        break;
    }

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

        load_which_t which;
        if (!load_read_config_which(&which))
        {
            LOG_E("read which fail with %d", load_get_error());
            return;
        }

        switch (which)
        {
        case LOAD_APP_USER:
        case LOAD_APP_OEM:
            uint8_t buffer[8] = {0};
            const uint32_t addr =
                ((which == LOAD_APP_USER) ? USER_START : OEM_START);
            memcpy(buffer, (void *)addr, sizeof(buffer));

            uint32_t stack = (uint32_t)buffer[3] << (3 * 8);
            stack |= (uint32_t)buffer[2] << (2 * 8);
            stack |= (uint32_t)buffer[1] << (1 * 8);
            stack |= (uint32_t)buffer[0] << (0 * 8);

            uint32_t reset = (uint32_t)buffer[7] << (3 * 8);
            reset |= (uint32_t)buffer[6] << (2 * 8);
            reset |= (uint32_t)buffer[5] << (1 * 8);
            reset |= (uint32_t)buffer[4] << (0 * 8);

            LOG_I("stack: 0x%08x, reset: 0x%08x", stack, reset);
            load_set_reset();
            break;
        default:
            const load_patch_t patch = load_get_patch();
            switch (patch)
            {
            case LOAD_PATCH_USER:
                load_set_apply(LOAD_APPLY_OEM);
                break;
            case LOAD_PATCH_OEM:
                load_set_apply(LOAD_APPLY_USER);
                break;
            default:
                LOG_E("load patch error with %d", patch);
                return;
            }
            break;
        }
    }
    else
    {
        LOG_E("download failed, error code: %d", status);
        load_write_config_which(LOAD_APP_INVALID); //!< 清除启动参数
    }
}

static int ymodem_env_init(void)
{
    // 组装接口对象
    static ymodem_ops_t ymodem_ops = {
        .on_begin = ymodem_on_begin,
        .on_data = ymodem_on_data,
        .on_end = ymodem_on_end,
    };

    ymodem_init();

    // 在初始化时注册
    ymodem_set_ops(&ymodem_ops);
    LOG_I("ymodem env init finish");

    return 0;
}
RUN_ENV_EXPORT(ymodem_env_init);

/**
 * @brief ymodem线程。
 * @param parameter 线程名称参数。
 */
static void ymodem_thread_entry(void *parameter)
{
    while (1)
    {
        // 实际应用中，可在此处判断是否进入下载模式
        ymodem_receive_loop();
    }
}

/**
 * @brief 创建ymodem线程。
 */
static int ymodem_thread_init(void)
{
    const char *const name = "ymodem";
    rt_err_t result = RT_EOK;
    rt_thread_t tid = rt_thread_create(name, ymodem_thread_entry, (void *)name,
                                       1024 * 2, 2, 0);
    if (tid != NULL)
    {
        LOG_I("<thread:%s> create success", name);
        result = rt_thread_startup(tid);
        if (result == RT_EOK)
        {
            LOG_I("<thread:%s> startup success", name);
        }
        else
        {
            LOG_I("<thread:%s> startup fail with %d", name, result);
        }
    }
    else
    {
        result = RT_ENOMEM;
        LOG_I("<thread:%s> create fail", name);
    }
    return result;
}
RUN_APP_EXPORT(ymodem_thread_init);