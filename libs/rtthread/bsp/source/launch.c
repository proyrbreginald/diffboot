#include <board.h>
#include <boot/boot.h>
#include <rthw.h>

// 配置调试日志
#define DBG_TAG __FILE_NAME__
#define DBG_LVL DBG_DEBUG
#include <rtdbg.h>

static int rti_start(void)
{
    LOG_I("start to auto init");
    return 0;
}
RT_LAUNCH_RUN_EXPORT(rti_start, "0");

static int rti_board_start(void)
{
    LOG_I("auto init board");
    return 0;
}
RT_LAUNCH_RUN_EXPORT(rti_board_start, "0.end");

static int rti_board_end(void)
{
    LOG_I("start to auto init components");
    return 0;
}
RT_LAUNCH_RUN_EXPORT(rti_board_end, "1.end");

static int rti_end(void)
{
    LOG_I("auto init finish");
    return 0;
}
RT_LAUNCH_RUN_EXPORT(rti_end, "6.end");

// 初始化系统板
void rt_hw_board_init(void)
{
    volatile const int_fn_void_t *fn_ptr;
    for (fn_ptr = &rt_init_rti_start; fn_ptr < &rt_init_rti_board_end;
         fn_ptr++)
    {
        (*fn_ptr)();
    }
}

#ifdef RT_USING_COMPONENTS_INIT
// 初始化rtos组件
void rt_components_init(void)
{
    volatile const int_fn_void_t *fn_ptr;
    for (fn_ptr = &rt_init_rti_board_end; fn_ptr <= &rt_init_rti_end;
         fn_ptr++)
    {
        (*fn_ptr)();
    }
}
#endif

// rtos启动流程
void rtthread_launch(void)
{
    // 屏蔽中断
    rt_hw_interrupt_disable();

    // 执行内核启动前的配置
    rt_hw_mcu_init();

    // 初始化系统板
    rt_hw_board_init();

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

    // 初始化空闲任务
    rt_thread_idle_init();

#ifdef RT_USING_TIMER_SOFT
    // 初始化系统定时器任务
    rt_system_timer_thread_init();
#endif

#ifdef RT_USING_COMPONENTS_INIT
    // 初始化rtos组件
    rt_components_init();
#endif

    // 初始化业务层
    rt_app_init();

    // 开始执行调度
    LOG_I("scheduler start");
    rt_system_scheduler_start();

    // 一般永远不会执行到这
    LOG_F("return from scheduler");
}