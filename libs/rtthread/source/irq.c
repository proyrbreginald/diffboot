#include <rthw.h>
#include <rtthread.h>
#include <rtdebug.h>

#ifndef __on_rt_interrupt_enter_hook
#define __on_rt_interrupt_enter_hook()                                         \
    __ON_HOOK_ARGS(rt_interrupt_enter_hook, ())
#endif
#ifndef __on_rt_interrupt_leave_hook
#define __on_rt_interrupt_leave_hook()                                         \
    __ON_HOOK_ARGS(rt_interrupt_leave_hook, ())
#endif

#if defined(RT_USING_HOOK) && defined(RT_HOOK_USING_FUNC_PTR)
static void (*rt_interrupt_enter_hook)(void);
static void (*rt_interrupt_leave_hook)(void);

/**
 * @ingroup Hook
 *
 * @brief This function set a hook function when the system enter a interrupt
 *
 * @note The hook function must be simple and never be blocked or suspend.
 *
 * @param hook the function point to be called
 */
void rt_interrupt_enter_sethook(void (*hook)(void))
{
    rt_interrupt_enter_hook = hook;
}

/**
 * @ingroup Hook
 *
 * @brief This function set a hook function when the system exit a interrupt.
 *
 * @note The hook function must be simple and never be blocked or suspend.
 *
 * @param hook the function point to be called
 */
void rt_interrupt_leave_sethook(void (*hook)(void))
{
    rt_interrupt_leave_hook = hook;
}
#endif

#ifdef RT_USING_SMP
#define rt_interrupt_nest rt_cpu_self()->irq_nest
#else
volatile uint8_t rt_interrupt_nest = 0;
#endif

/**
 * @brief This function will be invoked by BSP, when enter interrupt service
 * routine
 *
 * @note Please don't invoke this routine in application
 *
 * @see rt_interrupt_leave
 */
void rt_interrupt_enter(void)
{
    rt_base_t level = rt_hw_interrupt_disable();
    rt_interrupt_nest++;
    RT_OBJECT_HOOK_CALL(rt_interrupt_enter_hook, ());
    rt_hw_interrupt_enable(level);

    RT_DEBUG_LOG(RT_DEBUG_IRQ, ("irq has come, irq current nest:%d\n",
                                (int32_t)rt_interrupt_nest));
}
RTM_EXPORT(rt_interrupt_enter);

/**
 * @brief This function will be invoked by BSP, when leave interrupt service
 * routine
 *
 * @note Please don't invoke this routine in application
 *
 * @see rt_interrupt_enter
 */
void rt_interrupt_leave(void)
{
    RT_DEBUG_LOG(RT_DEBUG_IRQ, ("irq is going to leave, irq current nest:%d\n",
                                (int32_t)rt_interrupt_nest));

    rt_base_t level = rt_hw_interrupt_disable();
    RT_OBJECT_HOOK_CALL(rt_interrupt_leave_hook, ());
    rt_interrupt_nest--;
    rt_hw_interrupt_enable(level);
}
RTM_EXPORT(rt_interrupt_leave);

/**
 * @brief This function will return the nest of interrupt.
 *
 * User application can invoke this function to get whether current
 * context is interrupt context.
 *
 * @return the number of nested interrupts.
 */
WEAK uint8_t rt_interrupt_get_nest(void)
{
    rt_base_t level = rt_hw_interrupt_disable();
    uint8_t ret = rt_interrupt_nest;
    rt_hw_interrupt_enable(level);
    return ret;
}
RTM_EXPORT(rt_interrupt_get_nest);

RTM_EXPORT(rt_hw_interrupt_disable);
RTM_EXPORT(rt_hw_interrupt_enable);