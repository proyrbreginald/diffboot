#include <board.h>
#include <rthw.h>

// 配置调试日志
#define DBG_TAG __FILE_NAME__
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

/*
 * 初始化顺序
 * rti_start        --> 0
 * rti_board_start  --> 0.end
 * BOARD_EXPORT     --> 1       ->  INIT_BOARD_EXPORT
 * rti_board_end    --> 1.end
 * PREV_EXPORT      --> 2       ->  INIT_PREV_EXPORT
 * DEVICE_EXPORT    --> 3       ->  INIT_DEVICE_EXPORT
 * COMPONENT_EXPORT --> 4       ->  INIT_COMPONENT_EXPORT
 * ENV_EXPORT       --> 5       ->  INIT_ENV_EXPORT
 * APP_EXPORT       --> 6       ->  INIT_APP_EXPORT
 * rti_end          --> 6.end
 */
static int rti_start(void)
{
    return 0;
}
INIT_EXPORT(rti_start, "0");

static int rti_board_start(void)
{
    return 0;
}
INIT_EXPORT(rti_board_start, "0.end");

static int rti_board_end(void)
{
    return 0;
}
INIT_EXPORT(rti_board_end, "1.end");

static int rti_end(void)
{
    return 0;
}
INIT_EXPORT(rti_end, "6.end");

// 初始化系统板
void rt_hw_board_init(void)
{
#if RT_DEBUG_INIT
    const struct rt_init_desc *desc;
    LOG_N("start to initialize board");
    for (desc = &__rt_init_desc_rti_board_start;
         desc < &__rt_init_desc_rti_board_end; desc++)
    {
        LOG_D("initialize %s : %d done", desc->fn_name, desc->fn());
    }
#else
    volatile const init_fn_t *fn_ptr;
    for (fn_ptr = &__rt_init_rti_board_start; fn_ptr < &__rt_init_rti_board_end;
         fn_ptr++)
    {
        (*fn_ptr)();
    }
#endif /* RT_DEBUG_INIT */
}

// 初始化rtos组件
void rt_components_init(void)
{
#if RT_DEBUG_INIT
    const struct rt_init_desc *desc;
    LOG_N("start to initialize components");
    for (desc = &__rt_init_desc_rti_board_end; desc < &__rt_init_desc_rti_end;
         desc++)
    {
        LOG_D("initialize %s : %d done", desc->fn_name, desc->fn());
    }
#else
    volatile const init_fn_t *fn_ptr;
    for (fn_ptr = &__rt_init_rti_board_end; fn_ptr < &__rt_init_rti_end;
         fn_ptr++)
    {
        (*fn_ptr)();
    }
#endif /* RT_DEBUG_INIT */
}

// 创建业务层任务
RT_WEAK void rt_app_init(void)
{
    return;
}

// rtos启动流程
void rtthread_startup(void)
{
    // 屏蔽中断
    rt_hw_interrupt_disable();

    // 执行内核启动前的配置
    rt_hw_mcu_init();

    // 初始化系统板
    rt_hw_board_init();

    // 初始化rtos组件
    rt_components_init();

    // 打印rtos版本到控制台
    rt_show_version();

    // 初始化系统定时器
    rt_system_timer_init();

    // 初始化调度系统
    rt_system_scheduler_init();

#ifdef RT_USING_SIGNALS
    // 初始化信号
    rt_system_signal_init();
#endif

    // 初始化系统定时器任务
    rt_system_timer_thread_init();

    // 初始化空闲任务
    rt_thread_idle_init();

    // 初始化业务层
    rt_app_init();

    // 开始执行调度
    rt_system_scheduler_start();

    // 一般永远不会执行到这
    LOG_W("return from scheduler");
}