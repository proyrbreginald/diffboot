#include <board.h>
#include <boot/ld.h>
#include <boot/section.h>
#include <lptim.h>
#include <rthw.h>

// 配置调试日志
#define DBG_TAG __FILE_NAME__
#define DBG_LVL DBG_VERBOSE
#include <rtdbg.h>

static rt_uint32_t ticks_per_us;

/* 累加睡眠的Tick数(虽然LPTIM是16位，但累加变量我们要用64位防止总数溢出) */
static volatile uint64_t total_sleep_ticks = 0;

FAST uint64_t rt_total_sleep_get(void)
{
    return total_sleep_ticks;
}

FAST void rt_total_sleep_clear(void)
{
    total_sleep_ticks = 0;
}

// 空闲钩子进行低功耗处理
FAST static void idle_hook_wfi(void)
{
    uint16_t start, end;
    rt_base_t level;

    // 关中断
    level = rt_hw_interrupt_disable();

    // 读开始时间
    start = (uint16_t)LPTIM1->CNT;

    __WFI();

    // 读结束时间
    end = (uint16_t)LPTIM1->CNT;

    // 开中断
    rt_hw_interrupt_enable(level);

    /* 计算差值并累加 (处理16位回环，逻辑很简单) */
    if (end >= start)
    {
        total_sleep_ticks += (end - start);
    }
    else
    {
        // 发生了回环 (例如 65535 -> 1)
        total_sleep_ticks += (0xFFFF - start + end + 1);
    }
}

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
FAST void rt_hw_us_delay(rt_uint32_t us)
{
    rt_uint32_t ticks = us * ticks_per_us;
    rt_uint32_t start = DWT->CYCCNT;

    // 循环等待直到达到目标周期数
    // 无符号减法会自动处理0xFFFFFFFF到0的翻转
    while ((DWT->CYCCNT - start) < ticks)
        ;
}

// 执行内核启动前的配置
void rt_hw_mcu_init(void)
{
    // MCU复位启动完成
    LOG_F("mcu reset finish");

    // 更新MCU内核时钟
    SystemCoreClockUpdate();
    LOG_V("cpu clock per s: %u", HAL_RCC_GetSysClockFreq());

    // 计算1us对应的时钟周期数
    ticks_per_us = HAL_RCC_GetSysClockFreq() / 1000000;
    LOG_V("tick per us: %u", ticks_per_us);

    // 初始化内核计数器
    bool result = rt_hw_dwt_init();
    if (result)
    {
        LOG_E("DWT init fail");
    }
    else
    {
        LOG_I("DWT init success");
    }

    LL_LPTIM_Enable(LPTIM1);
    LL_LPTIM_SetAutoReload(LPTIM1, 0xFFFF); // 设置最大计数值
    LL_LPTIM_StartCounter(LPTIM1, LL_LPTIM_OPERATING_MODE_CONTINUOUS);
    LOG_I("start lptim counter");

    // 配置systick中断频率
    HAL_SYSTICK_Config(HAL_RCC_GetSysClockFreq() / RT_TICK_PER_SECOND);
    LOG_V("systick per s: %u", RT_TICK_PER_SECOND);

#if defined(RT_USING_HEAP)
    // 初始化rtos堆内存
    rt_system_heap_init((void *)&_heap_start, (void *)&heap_end);
    LOG_V("heap: [0x%p, 0x%p]", &_heap_start, &heap_end);
#endif

    // 设置空闲钩子
    rt_thread_idle_sethook(idle_hook_wfi);
    LOG_I("add idle hook: idle_hook_wfi");
}

FAST void SysTick_Handler(void)
{
    rt_interrupt_enter();
    HAL_IncTick();
    rt_tick_increase();
    rt_interrupt_leave();
}