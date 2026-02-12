#ifndef _RTHW_H_
#define _RTHW_H_

#include "rtthread.h"

enum RT_HW_CACHE_OPS {
    RT_HW_CACHE_FLUSH      = 0x01,
    RT_HW_CACHE_INVALIDATE = 0x02,
};

void rt_hw_cpu_icache_enable(void);
void rt_hw_cpu_icache_disable(void);
rt_base_t rt_hw_cpu_icache_status(void);
void rt_hw_cpu_icache_ops(int ops, void *addr, int size);

void rt_hw_cpu_dcache_enable(void);
void rt_hw_cpu_dcache_disable(void);
rt_base_t rt_hw_cpu_dcache_status(void);
void rt_hw_cpu_dcache_ops(int ops, void *addr, int size);

void rt_hw_cpu_reset(void);
void rt_hw_cpu_shutdown(void);

rt_uint8_t *rt_hw_stack_init(void *entry, void *parameter,
                             rt_uint8_t *stack_addr, void *exit);

typedef void (*rt_isr_handler_t)(int vector, void *param);

struct rt_irq_desc {
    rt_isr_handler_t handler;
    void *param;
#ifdef RT_USING_INTERRUPT_INFO
    char name[RT_NAME_MAX];
    rt_uint32_t counter;
#endif
};

void rt_hw_interrupt_init(void);
void rt_hw_interrupt_mask(int vector);
void rt_hw_interrupt_umask(int vector);
rt_isr_handler_t rt_hw_interrupt_install(int vector, rt_isr_handler_t handler,
                                         void *param, const char *name);

#ifdef RT_USING_SMP
rt_base_t rt_hw_local_irq_disable();
void rt_hw_local_irq_enable(rt_base_t level);
#define rt_hw_interrupt_disable rt_cpus_lock
#define rt_hw_interrupt_enable  rt_cpus_unlock
#else
rt_base_t rt_hw_interrupt_disable(void);
void rt_hw_interrupt_enable(rt_base_t level);
#endif

#ifdef RT_USING_SMP
void rt_hw_context_switch(rt_ubase_t from, rt_ubase_t to,
                          struct rt_thread *to_thread);
void rt_hw_context_switch_to(rt_ubase_t to, struct rt_thread *to_thread);
void rt_hw_context_switch_interrupt(void *context, rt_ubase_t from,
                                    rt_ubase_t to, struct rt_thread *to_thread);
#else
void rt_hw_context_switch(rt_ubase_t from, rt_ubase_t to);
void rt_hw_context_switch_to(rt_ubase_t to);
void rt_hw_context_switch_interrupt(rt_ubase_t from, rt_ubase_t to);
#endif

void rt_hw_backtrace(rt_uint32_t *fp, rt_ubase_t thread_entry);
void rt_hw_show_memory(rt_uint32_t addr, rt_size_t size);

void rt_hw_exception_install(rt_err_t (*exception_handle)(void *context));

#ifdef RT_USING_SMP
typedef union {
    unsigned long slock;
    struct __arch_tickets {
        unsigned short owner;
        unsigned short next;
    } tickets;
} rt_hw_spinlock_t;

struct rt_spinlock {
    rt_hw_spinlock_t lock;
};

void rt_hw_spin_lock_init(rt_hw_spinlock_t *lock);
void rt_hw_spin_lock(rt_hw_spinlock_t *lock);
void rt_hw_spin_unlock(rt_hw_spinlock_t *lock);
int rt_hw_cpu_id(void);

extern rt_hw_spinlock_t _cpus_lock;
extern rt_hw_spinlock_t _rt_critical_lock;

#define __RT_HW_SPIN_LOCK_INITIALIZER(lockname) {0}
#define __RT_HW_SPIN_LOCK_UNLOCKED(lockname)                                   \
    (rt_hw_spinlock_t) __RT_HW_SPIN_LOCK_INITIALIZER(lockname)
#define RT_DEFINE_SPINLOCK(x) rt_hw_spinlock_t x = __RT_HW_SPIN_LOCK_UNLOCKED(x)
#define RT_DECLARE_SPINLOCK(x)
void rt_hw_ipi_send(int ipi_vector, unsigned int cpu_mask);
void rt_hw_secondary_cpu_up(void);
void rt_hw_secondary_cpu_idle_exec(void);
#else
#define RT_DEFINE_SPINLOCK(x)
#define RT_DECLARE_SPINLOCK(x)  rt_ubase_t x
#define rt_hw_spin_lock(lock)   *(lock) = rt_hw_interrupt_disable()
#define rt_hw_spin_unlock(lock) rt_hw_interrupt_enable(*(lock))
#endif

#endif
