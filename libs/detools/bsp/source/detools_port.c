/* START OF FILE detools_port.c */
#include <detools_port.h>
#include <main.h>
#include <rtthread.h>

// 配置调试日志
#define DBG_TAG __FILE_NAME__
#define DBG_LVL DBG_DEBUG
#include <rtdebug.h>

// 假设已经实现了 Flash 擦除和写入的底层封装函数
// 注意：Flash 写入前必须确保对应的 Sector/Page 已经被擦除！
extern int bsp_flash_write(uint32_t addr, const uint8_t *data, uint32_t size);
extern int bsp_flash_erase_sector_by_addr(uint32_t addr);

/* ====================================================================
 * 1. 回调函数：读取旧固件 (From Read)
 * ==================================================================== */
ITCM static int cb_from_read(void *arg_p, uint8_t *buf_p, size_t size)
{
    detools_ctx_t *ctx = (detools_ctx_t *)arg_p;

    // STM32 Flash 支持直接内存读取，非常高效
    uint8_t *src = (uint8_t *)(ctx->old_app_base + ctx->old_app_offset);
    memcpy(buf_p, src, size);

    ctx->old_app_offset += size;
    return 0; // 返回 0 表示成功
}

/* ====================================================================
 * 2. 回调函数：移动旧固件读取指针 (From Seek)
 * ==================================================================== */
ITCM static int cb_from_seek(void *arg_p, int offset)
{
    detools_ctx_t *ctx = (detools_ctx_t *)arg_p;

    // detools 可能会向前或向后 seek，直接修改偏移量即可
    ctx->old_app_offset += offset;
    return 0;
}

/* ====================================================================
 * 3. 回调函数：读取差分包 (Patch Read)
 * ==================================================================== */
ITCM static int cb_patch_read(void *arg_p, uint8_t *buf_p, size_t size)
{
    detools_ctx_t *ctx = (detools_ctx_t *)arg_p;

    uint8_t *src = (uint8_t *)(ctx->patch_base + ctx->patch_offset);
    memcpy(buf_p, src, size);

    ctx->patch_offset += size;
    return 0;
}

/* ====================================================================
 * 写入生成的新固件 (To Write) - 带 32 字节缓存的 Flash 写入
 * ==================================================================== */
ITCM static int cb_to_write(void *arg_p, const uint8_t *buf_p, size_t size)
{
    detools_ctx_t *ctx = (detools_ctx_t *)arg_p;
    uint32_t bytes_processed = 0;
    int result = HAL_OK;

    while (bytes_processed < size)
    {
        // 计算当前还能往缓存里塞多少字节
        uint32_t copy_len = 32 - ctx->write_buf_len;
        if (copy_len > (size - bytes_processed))
        {
            copy_len = size - bytes_processed;
        }

        // 拷贝到缓存
        memcpy(&ctx->write_buf[ctx->write_buf_len], &buf_p[bytes_processed],
               copy_len);
        ctx->write_buf_len += copy_len;
        bytes_processed += copy_len;

        // 如果缓存满 32 字节，则执行一次真实的 Flash 写入
        if (ctx->write_buf_len == 32)
        {
            uint32_t write_addr = ctx->new_app_base + ctx->new_app_offset;

            __disable_irq(); // 关闭全局中断
            result = HAL_FLASH_Unlock();

            if (result == HAL_OK)
            {
                // 清除 Bank1 和 Bank2
                // 的所有错误标志，防止历史遗留错误导致编程失败
                // (此处为了通用性直接把两个 Bank
                // 的标志都清了，你也可以按需修改)
                __HAL_FLASH_CLEAR_FLAG_BANK1(FLASH_FLAG_ALL_ERRORS_BANK1);
                __HAL_FLASH_CLEAR_FLAG_BANK2(
                    FLASH_FLAG_ALL_ERRORS_BANK2); // 若单片机支持双Bank

                result =
                    HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, write_addr,
                                      (uint32_t)ctx->write_buf);

                HAL_FLASH_Lock();
            }
            __enable_irq(); // 恢复中断

            if (result != HAL_OK)
            {
                // LOG_E("flash program fail at 0x%08X", write_addr);
                return -1; // detools 要求的错误返回值为负数
            }

            // 写入成功，偏移量增加 32 字节，清空缓存长度
            ctx->new_app_offset += 32;
            ctx->write_buf_len = 0;
        }
    }

    return 0; // 0 表示成功处理
}

/* ====================================================================
 * 5. 顶层暴露接口
 * ==================================================================== */
ITCM int detools_apply_patch(uint32_t old_app_addr, uint32_t patch_addr,
                        uint32_t patch_size, uint32_t new_app_addr)
{
    detools_ctx_t ctx;
    int res;

    // 初始化上下文环境
    ctx.old_app_base = old_app_addr;
    ctx.old_app_offset = 0;

    ctx.patch_base = patch_addr;
    ctx.patch_offset = 0;

    ctx.new_app_base = new_app_addr;
    ctx.new_app_offset = 0;

    // TODO: 在这里执行新固件存放区的 Flash 擦除操作 (推荐做法)
    // erase_app_partition(new_app_addr, EXPECTED_NEW_APP_SIZE);

    // 调用 detools 核心 API
    res = detools_apply_patch_callbacks(cb_from_read, cb_from_seek,
                                        cb_patch_read, patch_size, cb_to_write,
                                        &ctx // arg_p 会透传给所有的 cb_xxx 函数
    );

    // 3. 核心步骤 (Flush)：如果升级成功，且缓存里还有没写满 32
    // 字节的零头数据，必须补 0xFF 写进去
    if ((res >= 0) && (ctx.write_buf_len > 0))
    {
        uint32_t write_addr = ctx.new_app_base + ctx.new_app_offset;

        // 剩余部分填充 0xFF
        memset(&ctx.write_buf[ctx.write_buf_len], 0xFF, 32 - ctx.write_buf_len);

        __disable_irq();
        int flash_res = HAL_FLASH_Unlock();

        if (flash_res == HAL_OK)
        {
            __HAL_FLASH_CLEAR_FLAG_BANK1(FLASH_FLAG_ALL_ERRORS_BANK1);
            __HAL_FLASH_CLEAR_FLAG_BANK1(FLASH_FLAG_ALL_ERRORS_BANK2);

            flash_res = HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD,
                                          write_addr, (uint32_t)ctx.write_buf);
            HAL_FLASH_Lock();
        }
        __enable_irq();

        if (flash_res != HAL_OK)
        {
            // LOG_E("flash flush fail at 0x%08X", write_addr);
            return -1;
        }

        // 累加最后的零头数据量
        ctx.new_app_offset += ctx.write_buf_len;
    }

    // 如果 res >= 0，代表成功，且返回值是新固件的总大小 (Bytes)
    // 如果 res < 0，代表失败，可以通过 detools_error_as_string(res)
    // 打印错误原因

    if (res > 0)
    {
        LOG_I("detools(%d)", res);
        res = DETOOLS_OK;
    }
    else
    {
        LOG_E("detools(%d): %s", res, detools_error_as_string(res));
    }

    return res;
}