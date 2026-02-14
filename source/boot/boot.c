#include <main.h>
#include <rtthread.h>

// 配置调试日志
#define DBG_TAG __FILE_NAME__
#define DBG_LVL DBG_VERBOSE
#include <rtdbg.h>

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
        uint64_t sleep_cnt = rt_total_sleep_get();
        rt_total_sleep_clear();
        rt_exit_critical();

        /* 理论上1秒钟LPTIM应该走1000000个Tick */
        if (sleep_cnt > 1000000)
        {
            sleep_cnt = 1000000; // 修正误差，防止出现负数
        }

        /* 占用率 = 100 - (睡眠占比) */
        float usage = 100.0f - ((float)sleep_cnt / 1000000) * 100.0f;

        static uint8_t cnt = 0;
        if (++cnt % 1 == 0)
        {
            LOG_V("cpu usage per s: %.2f%%", usage);
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
#undef THREAD_NAME
INIT_APP_EXPORT(count_thread_init);

#define THREAD_NAME "boot"
// 差分启动线程
static void boot_thread_entry(void *parameter)
{
    while (1)
    {
        LOG_I(THREAD_NAME " thread running");
        HAL_GPIO_TogglePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin);
        rt_thread_mdelay(250);
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
#undef THREAD_NAME
INIT_APP_EXPORT(boot_thread_init);

// 创建业务层任务
void rt_app_init(void)
{
    LOG_I("start to init app");
}