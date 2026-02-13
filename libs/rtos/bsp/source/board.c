#include <board.h>
#include <boot/ld.h>
#include <rthw.h>

// 配置调试日志
#define DBG_TAG __FILE_NAME__
#define DBG_LVL DBG_VERBOSE
#include <rtdbg.h>

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
    // MCU复位启动完成
    LOG_F("mcu reset finish");

    // 更新MCU内核时钟
    SystemCoreClockUpdate();

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
    
    LOG_V("cpu clock per s: %u", HAL_RCC_GetSysClockFreq());

    // 计算1us对应的时钟周期数
    ticks_per_us = HAL_RCC_GetSysClockFreq() / 1000000; 
    LOG_V("tick per us: %u", ticks_per_us);

    // 配置systick中断频率
    HAL_SYSTICK_Config(HAL_RCC_GetSysClockFreq() / RT_TICK_PER_SECOND);
    LOG_V("systick per s: %u", RT_TICK_PER_SECOND);

#if defined(RT_USING_HEAP)
    // 初始化rtos堆内存
    rt_system_heap_init((void *)&_heap_start, (void *)&heap_end);
    LOG_V("heap: [0x%p, 0x%p]", &_heap_start, &heap_end);
#endif
}