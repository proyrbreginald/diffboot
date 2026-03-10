#include <main.h>
#include <rtthread.h>
#include <ymodem.h>

// 配置调试日志
#define DBG_TAG __FILE_NAME__
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

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
            LOG_D("cpu usage per s: %.2f%%",
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
    rt_thread_t tid = rt_thread_create(THREAD_NAME, count_thread_entry, RT_NULL,
                                       1024, RT_THREAD_PRIORITY_MAX - 2, 0);
    if (tid != RT_NULL)
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
INIT_APP_EXPORT(count_thread_init);

/**
 * @brief ymodem接收到文件头时执行的回调。
 *
 * @param name 接收到的文件名字符串。
 * @param size 文件内容的字节大小。
 * @return int 返回0表示继续接收此文件，否则拒绝接收。
 */
static int ymodem_on_begin(const char *name, uint32_t size)
{
    LOG_I("start download: %s (%d bytes)", name, size);
    // 这里可以执行Flash擦除操作
    return 0;
}

/**
 * @brief ymodem接收到文件数据包时执行的回调。
 *
 * @param data 数据指针。
 * @param len 数据长度。
 * @param offset 当前文件偏移量。
 * @return int 0: 处理成功; 非0: 错误 (例如写入失败)，将终止传输。
 */
static int ymodem_on_data(const uint8_t *data, uint32_t len, uint32_t offset)
{
    LOG_I("offset: %u len: %u", offset, len); // 打印进度条
    // 这里将数据块写入 Flash
    // flash_write(PARTITION_ADDR + offset, data, len);
    return 0;
}

/**
 * @brief 传输结束。
 *
 * @param status 0: 成功; 非0: 异常终止 (超时、校验失败、被取消等)
 */
static void ymodem_on_end(int status)
{
    if (status == 0)
    {
        LOG_I("download success!");
    }
    else
    {
        LOG_I("download failed, error code: %d", status);
    }
}

// 组装接口对象
static ymodem_ops_t ymodem_ops = {
    .on_begin = ymodem_on_begin,
    .on_data = ymodem_on_data,
    .on_end = ymodem_on_end,
};

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
        rt_thread_create(THREAD_NAME, boot_thread_entry, RT_NULL, 1024, 1, 0);
    if (tid != RT_NULL)
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
INIT_APP_EXPORT(boot_thread_init);

// 创建业务层任务
void rt_app_init(void)
{
    LOG_I("start to init app");
}