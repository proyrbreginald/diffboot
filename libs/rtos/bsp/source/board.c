#include <board.h>
#include <boot/ld.h>
#include <main.h>
#include <rthw.h>

// 配置调试日志
#define DBG_TAG __FILE_NAME__
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

static uint32_t cpu_clock_frequency;
static rt_uint32_t ticks_per_us;

static bool rt_hw_dwt_init(void)
{
    /* 禁用DWT计数器（先停止，方便配置） */
    DWT->CTRL &= ~DWT_CTRL_CYCCNTENA_Msk;

    /* 启用CoreDebug的TRCENA位，开启跟踪组件 */
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;

    /* 解锁DWT寄存器的访问权限 (STM32H7特有步骤) */
    DWT->LAR = 0xC5ACCE55;

    /* 清零循环计数器 */
    DWT->CYCCNT = 0;

    /* 启用DWT循环计数器 */
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

    /* 检查是否启动成功（有些环境下可能无法开启） */
    __DSB(); // 数据同步屏障
    if (DWT->CYCCNT == 0)
    {
        return 1;
    }
    return 0;
}

/**
 * @brief 精确微秒延时实现
 * @param us 延时长度(微秒)
 */
void rt_hw_us_delay(rt_uint32_t us)
{
    rt_uint32_t ticks = us * ticks_per_us;
    rt_uint32_t start = DWT->CYCCNT;

    /* 循环等待直到达到目标周期数 */
    /* 无符号减法会自动处理0xFFFFFFFF到0的翻转 */
    while ((DWT->CYCCNT - start) < ticks)
        ;
}

// 执行内核启动前的配置
void rt_hw_mcu_init(void)
{
    // 更新MCU内核时钟
    SystemCoreClockUpdate();
    LOG_N("mcu reset finish");

    // 配置systick中断频率
    cpu_clock_frequency = HAL_RCC_GetSysClockFreq();
    ticks_per_us = cpu_clock_frequency / 1000000; // 计算1us对应的时钟周期数
    HAL_SYSTICK_Config(cpu_clock_frequency / RT_TICK_PER_SECOND);
    LOG_I("cpu clock per second: %u", cpu_clock_frequency);
    LOG_I("tick per us: %u", ticks_per_us);
    LOG_I("systick per second: %u", RT_TICK_PER_SECOND);

    // 初始化内核计数器
    bool result = rt_hw_dwt_init();
    if (result)
    {
        LOG_E("DWT init fail");
    }
    else
    {
        LOG_N("DWT init success");
    }

#if defined(RT_USING_HEAP)
    // 初始化rtos堆内存
    rt_system_heap_init((void *)&_heap_start, (void *)&heap_end);
    LOG_I("heap: [0x%p, 0x%p]", &_heap_start, &heap_end);
#endif
}

// 差分启动的线程
void diffboot_thread_entry(void *parameter)
{
    LOG_I("diffboot thread start");
    while (1)
    {
        HAL_GPIO_TogglePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin);
        rt_thread_mdelay(1000);
    }
}

// 测试的线程
void test_thread_entry(void *parameter)
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
    LOG_N("start to initialize app");

    rt_thread_t tid =
        rt_thread_create("diffboot", diffboot_thread_entry, RT_NULL, 512, 1, 0);
    if (tid != RT_NULL)
    {
        LOG_N("diffboot thread create success");
        rt_err_t result = rt_thread_startup(tid);
        if (result == RT_EOK)
        {
            LOG_N("diffboot thread startup success");
        }
        else
        {
            LOG_W("diffboot thread startup fail with %d", result);
        }
    }
    else
    {
        LOG_W("diffboot thread create fail");
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