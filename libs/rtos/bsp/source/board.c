#include <board.h>
#include <boot/ld.h>
#include <boot/section.h>
#include <main.h>
#include <rthw.h>

// 配置调试日志
#define DBG_TAG __FILE_NAME__
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

// 执行内核启动前的配置
void rt_hw_mcu_init(void)
{
    // 更新MCU内核时钟
    SystemCoreClockUpdate();
    LOG_N("mcu reset");

    // 配置systick中断频率
    const uint32_t cpu_clock_frequency = HAL_RCC_GetSysClockFreq();
    HAL_SYSTICK_Config(cpu_clock_frequency / RT_TICK_PER_SECOND);
    LOG_I("cpu clock frequency: %u Hz", cpu_clock_frequency);
    LOG_I("systick frequency: %u Hz", RT_TICK_PER_SECOND);

#if defined(RT_USING_HEAP)
    // 初始化rtos堆内存
    rt_system_heap_init((void *)&_heap_start, (void *)&heap_end);
    LOG_I("heap: [0x%p, 0x%p]", &_heap_start, &heap_end);
#endif
}

// 差分启动的主任务
void diffboot_thread_entry(void *parameter)
{
    LOG_I("diffboot task start");
    while (1)
    {
        HAL_GPIO_TogglePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin);
        rt_thread_mdelay(1000);
    }
}

// 创建业务层任务
void rt_app_init(void)
{
    LOG_N("start to initialize app");
    rt_thread_t tid =
        rt_thread_create("diffboot", diffboot_thread_entry, RT_NULL, 512, 1, 0);

    if (tid != RT_NULL)
    {
        LOG_N("diffboot task create success");
        rt_err_t result = rt_thread_startup(tid);
        if (result == RT_EOK)
        {
            LOG_N("diffboot task startup success");
        }
        else
        {
            LOG_W("diffboot task startup fail with %d", result);
        }
    }
    else
    {
        LOG_W("diffboot task create fail");
    }
}