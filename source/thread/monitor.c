#include <rtthread.h>

// 配置调试日志
#define DBG_TAG __FILE_NAME__
#define DBG_LVL DBG_DEBUG
#include <rtdebug.h>

/**
 * @brief 监视器线程。
 * @param parameter 线程名称字符串。
 */
static void monitor_thread_entry(void *parameter)
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

/**
 * @brief 启动监视器线程。
 * @return int 非0为失败。
 */
static int monitor_thread_init(void)
{
    const char *const name = "monitor";
    rt_err_t result = RT_EOK;
    rt_thread_t tid = rt_thread_create(name, monitor_thread_entry, (void *)name,
                                       1024 * 2, RT_THREAD_PRIORITY_MAX - 2, 0);
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
            LOG_E("<thread:%s> startup fail with %d", name, result);
        }
    }
    else
    {
        result = RT_ENOMEM;
        LOG_E("<thread:%s> create fail", name);
    }
    return result;
}
RUN_APP_EXPORT(monitor_thread_init);