#include <main.h>
#include <rtthread.h>

// 配置调试日志
#define DBG_TAG __FILE_NAME__
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

// 差分启动线程
static void boot_thread_entry(void *parameter)
{
    LOG_D("boot thread start");
    while (1)
    {
        HAL_GPIO_TogglePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin);
        rt_thread_mdelay(1000);
    }
}

// 测试线程
static void test_thread_entry(void *parameter)
{
    LOG_D("test thread start");
    while (1)
    {
        HAL_GPIO_TogglePin(LED_RED_GPIO_Port, LED_RED_Pin);
        rt_hw_us_delay(250 * 1000);
    }
}

// 创建业务层任务
void rt_app_init(void)
{
    LOG_I("start to init app");

    rt_thread_t tid =
        rt_thread_create("boot", boot_thread_entry, RT_NULL, 512, 1, 0);
    if (tid != RT_NULL)
    {
        LOG_I("boot thread create success");
        rt_err_t result = rt_thread_startup(tid);
        if (result == RT_EOK)
        {
            LOG_I("boot thread startup success");
        }
        else
        {
            LOG_E("boot thread startup fail with %d", result);
        }
    }
    else
    {
        LOG_E("boot thread create fail");
    }

    tid = rt_thread_create("test", test_thread_entry, RT_NULL, 512,
                           RT_THREAD_PRIORITY_MAX - 2, 0);
    if (tid != RT_NULL)
    {
        LOG_I("test thread create success");
        rt_err_t result = rt_thread_startup(tid);
        if (result == RT_EOK)
        {
            LOG_I("test thread startup success");
        }
        else
        {
            LOG_E("test thread startup fail with %d", result);
        }
    }
    else
    {
        LOG_E("test thread create fail");
    }
}