/**
 * @file rthw.h
 * @author reginald.yang (proyrb@yeah.net)
 * @version 0.1
 * @date 2026-03-23
 * @copyright Copyright (c) 2026
 * @brief 提供为rtthread适配的硬件操作接口。
 */

#ifndef _RTHW_H_
#define _RTHW_H_

#include "rtdef.h"

/**
 * @brief CPU Cache 操作类型枚举。
 */
enum RT_HW_CACHE_OPS {
    RT_HW_CACHE_FLUSH = 0x01, /**< Cache 清除（将 Cache 数据写回内存） */
    RT_HW_CACHE_INVALIDATE =
        0x02, /**< Cache 无效化（丢弃 Cache 数据，从内存重新读取） */
};

/**
 * @brief 使能指令 Cache。
 */
void rt_hw_cpu_icache_enable(void);

/**
 * @brief 禁用指令 Cache。
 */
void rt_hw_cpu_icache_disable(void);

/**
 * @brief 获取指令 Cache 的状态。
 * @return rt_base_t 1 表示使能，0 表示禁用。
 */
rt_base_t rt_hw_cpu_icache_status(void);

/**
 * @brief 对指令 Cache 执行指定的操作。
 * @param ops  操作类型 (参考 enum RT_HW_CACHE_OPS)。
 * @param addr 操作起始地址。
 * @param size 操作数据大小。
 */
void rt_hw_cpu_icache_ops(int ops, void *addr, int size);

/**
 * @brief 使能数据 Cache。
 */
void rt_hw_cpu_dcache_enable(void);

/**
 * @brief 禁用数据 Cache。
 */
void rt_hw_cpu_dcache_disable(void);

/**
 * @brief 获取数据 Cache 的状态。
 * @return rt_base_t 1 表示使能，0 表示禁用。
 */
rt_base_t rt_hw_cpu_dcache_status(void);

/**
 * @brief 对数据 Cache 执行指定的操作。
 * @param ops  操作类型 (参考 enum RT_HW_CACHE_OPS)。
 * @param addr 操作起始地址。
 * @param size 操作数据大小。
 */
void rt_hw_cpu_dcache_ops(int ops, void *addr, int size);

/**
 * @brief 复位 CPU。
 */
void rt_hw_cpu_reset(void);

/**
 * @brief 关闭 CPU。
 */
void rt_hw_cpu_shutdown(void);

/**
 * @brief 初始化线程堆栈。
 * @param entry      线程入口函数地址。
 * @param parameter  传递给入口函数的参数。
 * @param stack_addr 堆栈起始地址。
 * @param exit       线程退出时的清理函数地址。
 * @return uint8_t*  初始化完成后的栈顶指针。
 */
uint8_t *rt_hw_stack_init(void *entry, void *parameter, uint8_t *stack_addr,
                          void *exit);

/**
 * @brief 中断服务例程 (ISR) 函数指针类型定义。
 * @param vector 中断向量号。
 * @param param  注册中断时传递的自定义参数。
 */
typedef void (*rt_isr_handler_t)(int vector, void *param);

/**
 * @brief 中断描述符结构体。
 */
struct rt_irq_desc {
    rt_isr_handler_t handler; //!< 中断服务例程
    void *param;              //!< 中断服务例程参数
#ifdef RT_USING_INTERRUPT_INFO
    char name[RT_NAME_MAX]; //!< 中断名称
    uint32_t counter;       //!< 中断触发计数
#endif
};

/**
 * @brief 初始化硬件中断控制器。
 */
void rt_hw_interrupt_init(void);

/**
 * @brief 屏蔽指定的中断向量。
 * @param vector 中断向量号。
 */
void rt_hw_interrupt_mask(int vector);

/**
 * @brief 取消屏蔽（使能）指定的中断向量。
 * @param vector 中断向量号。
 */
void rt_hw_interrupt_umask(int vector);

/**
 * @brief 安装中断服务例程。
 * @param vector  中断向量号。
 * @param handler 中断处理函数。
 * @param param   传递给中断处理函数的参数。
 * @param name    中断名称（用于调试信息）。
 * @return rt_isr_handler_t 返回旧的中断处理函数指针。
 */
rt_isr_handler_t rt_hw_interrupt_install(int vector, rt_isr_handler_t handler,
                                         void *param, const char *name);

#ifdef RT_USING_SMP
/**
 * @brief 关闭当前 CPU 的本地中断。
 * @return rt_base_t 返回关闭前的中断状态（用于恢复）。
 */
rt_base_t rt_hw_local_irq_disable(void);

/**
 * @brief 恢复当前 CPU 的本地中断状态。
 * @param level 使用 rt_hw_local_irq_disable 获取的中断状态。
 */
void rt_hw_local_irq_enable(rt_base_t level);

/** @brief SMP 下使用多核锁实现全局中断屏蔽 */
#define rt_hw_interrupt_disable rt_cpus_lock
/** @brief SMP 下使用多核锁实现全局中断恢复 */
#define rt_hw_interrupt_enable rt_cpus_unlock
#else
/**
 * @brief 全局关闭中断。
 * @return rt_base_t 返回关闭中断前的中断状态（寄存器值）。
 */
rt_base_t rt_hw_interrupt_disable(void);

/**
 * @brief 全局恢复中断。
 * @param level 通过 rt_hw_interrupt_disable 获取的中断状态值。
 */
void rt_hw_interrupt_enable(rt_base_t level);
#endif

#ifdef RT_USING_SMP
/**
 * @brief 线程间上下文切换。
 * @param from      切换前线程的栈指针地址。
 * @param to        切换后线程的栈指针地址。
 * @param to_thread 切换后的目标线程控制块指针。
 */
void rt_hw_context_switch(rt_ubase_t from, rt_ubase_t to,
                          struct rt_thread *to_thread);

/**
 * @brief 从无线程状态切换到第一个线程。
 * @param to        目标线程的栈指针地址。
 * @param to_thread 目标线程控制块指针。
 */
void rt_hw_context_switch_to(rt_ubase_t to, struct rt_thread *to_thread);

/**
 * @brief 中断上下文切换。
 * @param context   触发中断时的原始上下文指针。
 * @param from      切换前线程的栈指针地址。
 * @param to        切换后线程的栈指针地址。
 * @param to_thread 切换后的目标线程控制块指针。
 */
void rt_hw_context_switch_interrupt(void *context, rt_ubase_t from,
                                    rt_ubase_t to, struct rt_thread *to_thread);
#else
/**
 * @brief 线程间上下文切换。
 * @param from 切换前线程的栈指针地址。
 * @param to   切换后线程的栈指针地址。
 */
void rt_hw_context_switch(rt_ubase_t from, rt_ubase_t to);

/**
 * @brief 从无线程状态切换到第一个线程。
 * @param to 目标线程的栈指针地址。
 */
void rt_hw_context_switch_to(rt_ubase_t to);

/**
 * @brief 中断上下文切换。
 * @param from 切换前线程的栈指针地址。
 * @param to   切换后线程的栈指针地址。
 */
void rt_hw_context_switch_interrupt(rt_ubase_t from, rt_ubase_t to);
#endif

/**
 * @brief 打印当前线程的函数调用栈（Backtrace）。
 * @param fp           当前帧指针（Frame Pointer）地址。
 * @param thread_entry 线程入口地址（用于判定回溯范围）。
 */
void rt_hw_backtrace(uint32_t *fp, rt_ubase_t thread_entry);

/**
 * @brief 以十六进制格式显示指定区域的内存数据。
 * @param addr 内存起始地址。
 * @param size 需要显示的内存大小。
 */
void rt_hw_show_memory(uint32_t addr, size_t size);

/**
 * @brief 安装硬件异常处理函数。
 * @param exception_handle 自定义的异常处理钩子函数，返回非零值表示处理成功。
 */
void rt_hw_exception_install(rt_err_t (*exception_handle)(void *context));

#ifdef RT_USING_SMP

/**
 * @brief 硬件自旋锁类型定义。
 * 使用票据锁（Ticket Lock）机制确保多核访问公平性。
 */
typedef union {
    unsigned long slock; //!< 锁原始值
    struct __arch_tickets {
        unsigned short owner; //!< 当前拥有锁的持有者
        unsigned short next;  //!< 下一个等待者
    } tickets;
} rt_hw_spinlock_t;

/**
 * @brief 自旋锁结构体。
 */
struct rt_spinlock {
    rt_hw_spinlock_t lock; //!< 包含的硬件自旋锁实例
};

/**
 * @brief 初始化硬件自旋锁。
 * @param lock 指向硬件自旋锁的指针。
 */
void rt_hw_spin_lock_init(rt_hw_spinlock_t *lock);

/**
 * @brief 获取硬件自旋锁。
 * @param lock 指向硬件自旋锁的指针。
 */
void rt_hw_spin_lock(rt_hw_spinlock_t *lock);

/**
 * @brief 释放硬件自旋锁。
 * @param lock 指向硬件自旋锁的指针。
 */
void rt_hw_spin_unlock(rt_hw_spinlock_t *lock);

/**
 * @brief 获取当前核心编号。
 * @return int CPU 核心索引。
 */
int rt_hw_cpu_id(void);

/** @brief 全局 CPU 锁 */
extern rt_hw_spinlock_t _cpus_lock;
/** @brief 全局临界区锁 */
extern rt_hw_spinlock_t _rt_critical_lock;

/** @brief 硬件自旋锁初始化值宏 */
#define __RT_HW_SPIN_LOCK_INITIALIZER(lockname) {0}
/** @brief 硬件自旋锁解锁状态宏 */
#define __RT_HW_SPIN_LOCK_UNLOCKED(lockname)                                   \
    (rt_hw_spinlock_t) __RT_HW_SPIN_LOCK_INITIALIZER(lockname)
/** @brief 定义并初始化一个自旋锁变量 */
#define RT_DEFINE_SPINLOCK(x) rt_hw_spinlock_t x = __RT_HW_SPIN_LOCK_UNLOCKED(x)
/** @brief 声明一个自旋锁变量 */
#define RT_DECLARE_SPINLOCK(x)

/**
 * @brief 发送处理器间中断 (IPI)。
 * @param ipi_vector 中断向量号。
 * @param cpu_mask   目标 CPU 的掩码位图。
 */
void rt_hw_ipi_send(int ipi_vector, unsigned int cpu_mask);

/**
 * @brief 启动从处理器 (Secondary CPU)。
 */
void rt_hw_secondary_cpu_up(void);

/**
 * @brief 从处理器空闲态执行任务。
 */
void rt_hw_secondary_cpu_idle_exec(void);
#else
/** @brief 定义自旋锁（单核模式下为空） */
#define RT_DEFINE_SPINLOCK(x)
/** @brief 声明自旋锁为基类型变量 */
#define RT_DECLARE_SPINLOCK(x)  rt_ubase_t x
/** @brief 单核环境下通过关中断实现自旋锁 */
#define rt_hw_spin_lock(lock)   *(lock) = rt_hw_interrupt_disable()
/** @brief 单核环境下通过恢复中断实现解锁 */
#define rt_hw_spin_unlock(lock) rt_hw_interrupt_enable(*(lock))
#endif

/**
 * @brief 底层控制台字符输出函数。
 * @param str 需要输出的字符串。
 */
void rt_hw_console_output(const char *str);

#endif