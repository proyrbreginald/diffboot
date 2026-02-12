#include <main.h>
#include <rtthread.h>

// 配置调试日志
#define DBG_TAG __FILE_NAME__
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

// 差分启动线程
static void boot_thread_entry(void *parameter)
{
    LOG_I("boot thread start");
    while (1)
    {
        HAL_GPIO_TogglePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin);
        rt_thread_mdelay(1000);
    }
}

// 测试线程
static void test_thread_entry(void *parameter)
{
    LOG_I("test thread start");
    while (1)
    {
        HAL_GPIO_TogglePin(LED_RED_GPIO_Port, LED_RED_Pin);
        rt_hw_us_delay(1000 * 500);
    }
}

// 创建业务层任务
void rt_app_init(void)
{
    LOG_N("start to init app");

    rt_thread_t tid =
        rt_thread_create("boot", boot_thread_entry, RT_NULL, 512, 1, 0);
    if (tid != RT_NULL)
    {
        LOG_N("boot thread create success");
        rt_err_t result = rt_thread_startup(tid);
        if (result == RT_EOK)
        {
            LOG_N("boot thread startup success");
        }
        else
        {
            LOG_W("boot thread startup fail with %d", result);
        }
    }
    else
    {
        LOG_W("boot thread create fail");
    }

    tid = rt_thread_create("test", test_thread_entry, RT_NULL, 512,
                           RT_THREAD_PRIORITY_MAX - 2, 0);
    if (tid != RT_NULL)
    {
        LOG_N("test thread create success");
        rt_err_t result = rt_thread_startup(tid);
        if (result == RT_EOK)
        {
            LOG_N("test thread startup success");
        }
        else
        {
            LOG_W("test thread startup fail with %d", result);
        }
    }
    else
    {
        LOG_W("test thread create fail");
    }
}