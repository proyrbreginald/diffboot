#ifndef _RTTHREAD_H_
#define _RTTHREAD_H_

#include <rtconfig.h>
#include "rtdef.h"
#include "rtservice.h"

/**
 * @brief 初始化环形缓冲区。
 * @param rb   环形缓冲区对象指针。
 * @param pool 用于存储数据的缓冲区地址。
 * @param size 缓冲区的大小（字节）。
 */
void rt_ringbuffer_init(struct rt_ringbuffer *rb, uint8_t *pool, uint16_t size);

/**
 * @brief 向环形缓冲区中写入数据。
 * @param rb     环形缓冲区对象指针。
 * @param ptr    待写入数据的指针。
 * @param length 待写入数据的长度。
 * @return size_t 返回实际成功写入的字节数。
 */
size_t rt_ringbuffer_put(struct rt_ringbuffer *rb, const uint8_t *ptr,
                         uint16_t length);

/**
 * @brief 从环形缓冲区中读取数据。
 * @param rb     环形缓冲区对象指针。
 * @param ptr    存放读取数据的目标缓冲区指针。
 * @param length 期望读取数据的长度。
 * @return size_t 返回实际读取到的字节数。
 */
size_t rt_ringbuffer_get(struct rt_ringbuffer *rb, uint8_t *ptr,
                         uint16_t length);

/**
 * @brief 获取环形缓冲区内当前存放的数据长度。
 * @param rb 环形缓冲区对象指针。
 * @return size_t 当前缓冲区内的数据字节数。
 */
size_t rt_ringbuffer_data_len(struct rt_ringbuffer *rb);

/**
 * @brief 重置环形缓冲区。
 * @note 此操作会清空缓冲区内已存的数据，重置读写指针。
 * @param rb 环形缓冲区对象指针。
 */
void rt_ringbuffer_reset(struct rt_ringbuffer *rb);

/**
 * @brief 获取指定类型对象的信息结构体。
 * @param type 对象类型枚举。
 * @return 返回指向该类型对象信息结构体的指针。
 */
struct rt_object_information *rt_object_get_information(
    enum rt_object_type type);

/**
 * @brief 获取指定类型当前系统中已存在的对象数量。
 * @param type 对象类型枚举。
 * @return 对象数量。
 */
int rt_object_get_length(enum rt_object_type type);

/**
 * @brief 获取指定类型的对象指针列表。
 * @param type 对象类型枚举。
 * @param pointers 用于存放对象指针的数组缓冲区。
 * @param maxlen 数组最大长度。
 * @return 实际获取到的对象指针个数。
 */
int rt_object_get_pointers(enum rt_object_type type, rt_object_t *pointers,
                           int maxlen);

/**
 * @brief 初始化一个静态对象。
 * @param object 指向对象内存块的指针。
 * @param type 对象类型。
 * @param name 对象名称。
 */
void rt_object_init(struct rt_object *object, enum rt_object_type type,
                    const char *name);

/**
 * @brief 脱离一个对象 (从内核管理列表中移除)。
 * @param object 指向对象的指针。
 */
void rt_object_detach(rt_object_t object);

#ifdef RT_USING_HEAP
/**
 * @brief 从堆内存中分配并初始化一个对象。
 * @param type 对象类型。
 * @param name 对象名称。
 * @return 分配成功返回对象指针，失败返回 RT_NULL。
 */
rt_object_t rt_object_allocate(enum rt_object_type type, const char *name);

/**
 * @brief 删除并释放一个动态分配的对象。
 * @param object 指向对象的指针。
 */
void rt_object_delete(rt_object_t object);
#endif

/**
 * @brief 判断该对象是否为系统对象（静态对象）。
 * @param object 指向对象的指针。
 * @return 若为系统对象返回 true，否则返回 false。
 */
bool rt_object_is_systemobject(rt_object_t object);

/**
 * @brief 获取对象的类型枚举值。
 * @param object 指向对象的指针。
 * @return 对象的类型值。
 */
uint8_t rt_object_get_type(rt_object_t object);

/**
 * @brief 根据名称和类型查找对象。
 * @param name 对象名称。
 * @param type 对象类型。
 * @return 查找到的对象指针，未找到返回 RT_NULL。
 */
rt_object_t rt_object_find(const char *name, uint8_t type);

#ifdef RT_USING_HOOK
/**
 * @name 对象操作钩子设置函数。
 * @{
 */
void rt_object_attach_sethook(
    void (*hook)(struct rt_object *object)); ///< 设置对象初始化钩子
void rt_object_detach_sethook(
    void (*hook)(struct rt_object *object)); ///< 设置对象脱离钩子
void rt_object_trytake_sethook(
    void (*hook)(struct rt_object *object)); ///< 设置尝试获取对象钩子
void rt_object_take_sethook(
    void (*hook)(struct rt_object *object)); ///< 设置获取对象钩子
void rt_object_put_sethook(
    void (*hook)(struct rt_object *object)); ///< 设置释放对象钩子
#endif

/**
 * @brief 微秒级延时函数。
 * @param us 延时的微秒数。
 */
void rt_hw_us_delay(uint32_t us);

/**
 * @brief 获取当前的系统节拍数。
 * @return 当前系统节拍数值。
 */
rt_tick_t rt_tick_get(void);

/**
 * @brief 设置当前的系统节拍数。
 * @param tick 要设置的节拍数值。
 */
void rt_tick_set(rt_tick_t tick);

/**
 * @brief 系统节拍加一，通常在时钟中断服务例程中调用。
 */
void rt_tick_increase(void);

/**
 * @brief 将毫秒转换为系统节拍数。
 * @param ms 毫秒数。
 * @return 对应的节拍数。
 */
rt_tick_t rt_tick_from_millisecond(int32_t ms);

/**
 * @brief 获取当前的系统节拍对应的毫秒数。
 * @return 当前对应的毫秒总数。
 */
rt_tick_t rt_tick_get_millisecond(void);

#ifdef RT_USING_HOOK
/**
 * @brief 设置系统节拍的钩子函数。
 * @param hook 钩子函数指针。
 */
void rt_tick_sethook(void (*hook)(void));
#endif

/**
 * @brief 初始化系统定时器相关硬件与数据结构。
 */
void rt_system_timer_init(void);

/**
 * @brief 初始化系统定时器线程。
 */
void rt_system_timer_thread_init(void);

/**
 * @brief 静态初始化定时器对象。
 * @param timer 定时器对象指针。
 * @param name 定时器名称。
 * @param timeout 超时处理函数指针。
 * @param parameter 超时处理函数的参数。
 * @param time 定时时间（节拍数）。
 * @param flag 定时器标志（如周期性、单次等）。
 */
void rt_timer_init(rt_timer_t timer, const char *name,
                   void (*timeout)(void *parameter), void *parameter,
                   rt_tick_t time, uint8_t flag);

/**
 * @brief 从系统中脱离（注销）定时器。
 * @param timer 定时器对象指针。
 * @return RT_EOK 成功。
 */
rt_err_t rt_timer_detach(rt_timer_t timer);

#ifdef RT_USING_HEAP
/**
 * @brief 动态创建并创建一个定时器对象。
 * @param name 定时器名称。
 * @param timeout 超时处理函数指针。
 * @param parameter 超时处理函数的参数。
 * @param time 定时时间。
 * @param flag 定时器标志。
 * @return 创建成功的定时器句柄，失败则返回 RT_NULL。
 */
rt_timer_t rt_timer_create(const char *name, void (*timeout)(void *parameter),
                           void *parameter, rt_tick_t time, uint8_t flag);

/**
 * @brief 删除动态创建的定时器。
 * @param timer 定时器对象指针。
 * @return RT_EOK 成功。
 */
rt_err_t rt_timer_delete(rt_timer_t timer);
#endif

/**
 * @brief 启动定时器。
 * @param timer 定时器对象指针。
 * @return RT_EOK 成功。
 */
rt_err_t rt_timer_start(rt_timer_t timer);

/**
 * @brief 停止定时器。
 * @param timer 定时器对象指针。
 * @return RT_EOK 成功。
 */
rt_err_t rt_timer_stop(rt_timer_t timer);

/**
 * @brief 控制定时器（修改属性、参数等）。
 * @param timer 定时器对象指针。
 * @param cmd 命令字。
 * @param arg 参数指针。
 * @return RT_EOK 成功。
 */
rt_err_t rt_timer_control(rt_timer_t timer, int cmd, void *arg);

/**
 * @brief 获取下一个定时器超时的时间戳。
 * @return 下一个定时器超时的节拍数值。
 */
rt_tick_t rt_timer_next_timeout_tick(void);

/**
 * @brief 检查定时器链表，处理超时的定时器。
 */
void rt_timer_check(void);

#ifdef RT_USING_HOOK
/**
 * @brief 设置定时器进入回调的钩子函数。
 * @param hook 钩子函数指针。
 */
void rt_timer_enter_sethook(void (*hook)(struct rt_timer *timer));

/**
 * @brief 设置定时器退出回调的钩子函数。
 * @param hook 钩子函数指针。
 */
void rt_timer_exit_sethook(void (*hook)(struct rt_timer *timer));
#endif

/**
 * @brief 初始化一个线程对象（静态线程）。
 * @param thread 线程控制块指针。
 * @param name 线程名称。
 * @param entry 线程入口函数。
 * @param parameter 传递给入口函数的参数。
 * @param stack_start 线程栈起始地址。
 * @param stack_size 线程栈大小（字节）。
 * @param priority 线程优先级。
 * @param tick 线程时间片长度。
 * @return rt_err_t RT_EOK 表示成功，其他值表示错误。
 */
rt_err_t rt_thread_init(struct rt_thread *thread, const char *name,
                        void (*entry)(void *parameter), void *parameter,
                        void *stack_start, uint32_t stack_size,
                        uint8_t priority, uint32_t tick);

/**
 * @brief 将一个静态线程从系统内核中脱离。
 * @param thread 线程控制块指针。
 * @return rt_err_t RT_EOK 表示成功，其他值表示错误。
 */
rt_err_t rt_thread_detach(rt_thread_t thread);

#ifdef RT_USING_HEAP
/**
 * @brief 创建一个线程（动态线程）。
 * @param name 线程名称。
 * @param entry 线程入口函数。
 * @param parameter 传递给入口函数的参数。
 * @param stack_size 线程栈大小。
 * @param priority 线程优先级。
 * @param tick 线程时间片长度。
 * @return rt_thread_t 成功返回线程句柄，失败返回 NULL。
 */
rt_thread_t rt_thread_create(const char *name, void (*entry)(void *parameter),
                             void *parameter, uint32_t stack_size,
                             uint8_t priority, uint32_t tick);

/**
 * @brief 删除一个动态线程并释放资源。
 * @param thread 线程句柄。
 * @return rt_err_t RT_EOK 表示成功，其他值表示错误。
 */
rt_err_t rt_thread_delete(rt_thread_t thread);
#endif

/**
 * @brief 获取当前正在运行的线程句柄。
 * @return rt_thread_t 当前线程句柄。
 */
rt_thread_t rt_thread_self(void);

/**
 * @brief 根据名称查找线程。
 * @param name 线程名称。
 * @return rt_thread_t 找到返回线程句柄，未找到返回 NULL。
 */
rt_thread_t rt_thread_find(char *name);

/**
 * @brief 启动一个线程，将其加入调度器就绪队列。
 * @param thread 线程句柄。
 * @return rt_err_t RT_EOK 表示成功，其他值表示错误。
 */
rt_err_t rt_thread_startup(rt_thread_t thread);

/**
 * @brief 当前线程让出处理器使用权。
 * @return rt_err_t RT_EOK 表示成功。
 */
rt_err_t rt_thread_yield(void);

/**
 * @brief 当前线程挂起指定的系统时钟节拍（Tick）。
 * @param tick 延时的节拍数。
 * @return rt_err_t RT_EOK 表示成功。
 */
rt_err_t rt_thread_delay(rt_tick_t tick);

/**
 * @brief 当前线程挂起直到指定的时刻。
 * @param tick 指向存储上次唤醒时间的指针。
 * @param inc_tick 递增的节拍间隔。
 * @return rt_err_t RT_EOK 表示成功。
 */
rt_err_t rt_thread_delay_until(rt_tick_t *tick, rt_tick_t inc_tick);

/**
 * @brief 当前线程延时指定的毫秒数。
 * @param ms 延时的毫秒数。
 * @return rt_err_t RT_EOK 表示成功。
 */
rt_err_t rt_thread_mdelay(int32_t ms);

/**
 * @brief 对线程进行控制（配置或获取信息）。
 * @param thread 线程句柄。
 * @param cmd 控制命令。
 * @param arg 参数指针。
 * @return rt_err_t RT_EOK 表示成功，其他值表示错误。
 */
rt_err_t rt_thread_control(rt_thread_t thread, int cmd, void *arg);

/**
 * @brief 手动挂起一个线程。
 * @param thread 线程句柄。
 * @return rt_err_t RT_EOK 表示成功，其他值表示错误。
 */
rt_err_t rt_thread_suspend(rt_thread_t thread);

/**
 * @brief 恢复一个挂起的线程。
 * @param thread 线程句柄。
 * @return rt_err_t RT_EOK 表示成功，其他值表示错误。
 */
rt_err_t rt_thread_resume(rt_thread_t thread);

#ifdef RT_USING_SIGNALS
/**
 * @brief 为线程分配信号资源。
 * @param tid 线程句柄。
 */
void rt_thread_alloc_sig(rt_thread_t tid);

/**
 * @brief 释放线程的信号资源。
 * @param tid 线程句柄。
 */
void rt_thread_free_sig(rt_thread_t tid);

/**
 * @brief 向指定线程发送信号。
 * @param tid 目标线程句柄。
 * @param sig 信号编号。
 * @return int 0 表示成功，负数表示失败。
 */
int rt_thread_kill(rt_thread_t tid, int sig);
#endif

#ifdef RT_USING_HOOK

/**
 * @brief 设置线程挂起回调钩子函数。
 * 当线程被挂起时，该钩子函数会被调用。
 * @param hook 指向线程挂起钩子函数的指针，函数原型为 void hook(rt_thread_t
 * thread)。 若传入 RT_NULL，则表示清除该钩子。
 */
void rt_thread_suspend_sethook(void (*hook)(rt_thread_t thread));

/**
 * @brief 设置线程恢复回调钩子函数。
 * 当线程从挂起状态恢复运行时，该钩子函数会被调用。
 * @param hook 指向线程恢复钩子函数的指针，函数原型为 void hook(rt_thread_t
 * thread)。 若传入 RT_NULL，则表示清除该钩子。
 */
void rt_thread_resume_sethook(void (*hook)(rt_thread_t thread));

/**
 * @brief 设置线程初始化回调钩子函数。
 * 当一个线程对象被成功初始化（rt_thread_init）后，该钩子函数会被调用。
 * @param hook 指向线程初始化钩子函数的指针，函数原型为 void hook(rt_thread_t
 * thread)。 若传入 RT_NULL，则表示清除该钩子。
 * @note 该钩子通常用于线程创建时的调试或资源追踪。
 */
void rt_thread_inited_sethook(void (*hook)(rt_thread_t thread));

#endif

/**
 * @brief 初始化空闲线程。
 */
void rt_thread_idle_init(void);

/**
 * @brief 获取空闲线程句柄。
 * @return rt_thread_t 空闲线程的句柄。
 */
rt_thread_t rt_thread_idle_gethandler(void);

#if defined(RT_USING_HOOK) || defined(RT_USING_IDLE_HOOK)
/**
 * @brief 设置空闲线程的钩子函数。
 * @param hook 指向钩子函数的指针。
 * @return rt_err_t RT_EOK 表示成功，其他值表示错误。
 */
rt_err_t rt_thread_idle_sethook(void (*hook)(void));

/**
 * @brief 删除空闲线程的钩子函数。
 * @param hook 指向要删除的钩子函数的指针。
 * @return rt_err_t RT_EOK 表示成功，其他值表示错误。
 */
rt_err_t rt_thread_idle_delhook(void (*hook)(void));
#endif

/**
 * @brief 获取空闲线程累计睡眠时间。
 * @return uint64_t 累计睡眠时间（通常以 Tick 为单位）。
 */
uint64_t rt_idle_total_sleep_get(void);

/**
 * @brief 清除空闲线程的累计睡眠时间记录。
 */
void rt_idle_total_sleep_clear(void);

/**
 * @brief 初始化系统调度器。
 */
void rt_system_scheduler_init(void);

/**
 * @brief 启动系统调度器，开始任务调度。
 */
void rt_system_scheduler_start(void);

/**
 * @brief 执行一次任务调度（查找最高优先级就绪线程并切换）。
 */
void rt_schedule(void);

/**
 * @brief 将线程插入到调度器的就绪列表中。
 * @param thread 线程控制块指针。
 */
void rt_schedule_insert_thread(struct rt_thread *thread);

/**
 * @brief 从调度器的就绪列表中移除线程。
 * @param thread 线程控制块指针。
 */
void rt_schedule_remove_thread(struct rt_thread *thread);

/**
 * @brief 进入临界区（关闭中断或禁止抢占）。
 */
void rt_enter_critical(void);

/**
 * @brief 退出临界区。
 */
void rt_exit_critical(void);

/**
 * @brief 获取当前的临界区嵌套层数。
 * @return uint16_t 嵌套层数。
 */
uint16_t rt_critical_level(void);

#ifdef RT_USING_HOOK
/**
 * @brief 设置调度器切换钩子（线程切换时调用）。
 * @param hook 钩子函数指针，参数分别为：from（切出线程），to（切入线程）。
 */
void rt_scheduler_sethook(void (*hook)(rt_thread_t from, rt_thread_t to));

/**
 * @brief 设置线程切换钩子（仅针对特定线程切换）。
 * @param hook 钩子函数指针，参数为目标线程句柄。
 */
void rt_scheduler_switch_sethook(void (*hook)(struct rt_thread *tid));
#endif

#ifdef RT_USING_SMP
/**
 * @brief 多核模式下的调度器 IPI (处理器间中断) 处理函数。
 * @param vector 中断向量号。
 * @param param 附加参数。
 */
void rt_scheduler_ipi_handler(int vector, void *param);
#endif

#ifdef RT_USING_SIGNALS
/**
 * @brief 屏蔽指定的信号。
 * @param signo 信号编号。
 */
void rt_signal_mask(int signo);

/**
 * @brief 解除指定信号的屏蔽。
 * @param signo 信号编号。
 */
void rt_signal_unmask(int signo);

/**
 * @brief 安装信号处理函数。
 * @param signo 信号编号。
 * @param handler 信号处理函数指针。
 * @return rt_sighandler_t 返回之前的处理函数指针，若出错返回 SIG_ERR。
 */
rt_sighandler_t rt_signal_install(int signo, rt_sighandler_t handler);

/**
 * @brief 等待指定的信号。
 * @param set 待等待的信号集指针。
 * @param si 用于存储信号信息的结构体指针。
 * @param timeout 等待超时时间（以 Tick 为单位，RT_WAITING_FOREVER
 * 表示永久等待）。
 * @return int 成功返回 0，否则返回负错误码。
 */
int rt_signal_wait(const rt_sigset_t *set, rt_siginfo_t *si, int32_t timeout);

/**
 * @brief 初始化系统信号模块。
 * @return int 0 表示成功，负数表示失败。
 */
int rt_system_signal_init(void);
#endif

#ifdef RT_USING_MEMPOOL
/**
 * @brief 初始化一个内存池对象。
 * @param mp         内存池控制块指针。
 * @param name       内存池名称。
 * @param start      内存池的起始地址。
 * @param size       内存池的总大小（字节）。
 * @param block_size 每个内存块的大小（字节）。
 * @return 错误码，RT_EOK 表示成功。
 */
rt_err_t rt_mp_init(struct rt_mempool *mp, const char *name, void *start,
                    size_t size, size_t block_size);

/**
 * @brief 脱离（销毁）一个静态初始化的内存池。
 * @param mp 内存池控制块指针。
 * @return 错误码，RT_EOK 表示成功。
 */
rt_err_t rt_mp_detach(struct rt_mempool *mp);

#ifdef RT_USING_HEAP
/**
 * @brief 在堆上创建一个内存池。
 * @param name        内存池名称。
 * @param block_count 内存池中块的数量。
 * @param block_size  每块的大小（字节）。
 * @return 成功返回内存池句柄，失败返回 RT_NULL。
 */
rt_mp_t rt_mp_create(const char *name, size_t block_count, size_t block_size);

/**
 * @brief 删除通过 rt_mp_create 创建的内存池。
 * @param mp 内存池句柄。
 * @return 错误码，RT_EOK 表示成功。
 */
rt_err_t rt_mp_delete(rt_mp_t mp);
#endif

/**
 * @brief 从内存池中分配一个内存块。
 * @param mp   内存池句柄
 * @param time 等待超时时间（单位：tick），若为 RT_WAITING_NO 则不等待
 *
 * @return 成功返回内存块首地址，失败返回 RT_NULL
 */
void *rt_mp_alloc(rt_mp_t mp, int32_t time);

/**
 * @brief 释放一个内存块回内存池。
 * @param block 待释放的内存块指针。
 */
void rt_mp_free(void *block);

#ifdef RT_USING_HOOK
/**
 * @brief 设置内存池分配钩子函数。
 * @param hook 钩子函数指针，函数原型为void (*hook)(struct rt_mempool *mp, void
 * *block)。
 */
void rt_mp_alloc_sethook(void (*hook)(struct rt_mempool *mp, void *block));

/**
 * @brief 设置内存池释放钩子函。
 * @param hook 钩子函数指针，函数原型为 void (*hook)(struct rt_mempool *mp, void
 * *block)。
 */
void rt_mp_free_sethook(void (*hook)(struct rt_mempool *mp, void *block));
#endif
#endif

#ifdef RT_USING_HEAP
/**
 * @brief 初始化系统堆内存管理系统。
 * @param begin_addr 堆内存起始地址。
 * @param end_addr   堆内存结束地址。
 */
void rt_system_heap_init(void *begin_addr, void *end_addr);

/**
 * @brief 从系统堆中分配内存。
 * @param nbytes 需要分配的字节数。
 * @return void* 分配成功返回内存首地址，失败返回 NULL。
 */
void *rt_malloc(size_t nbytes);

/**
 * @brief 释放内存块。
 * @param ptr 需要释放的内存地址。
 */
void rt_free(void *ptr);

/**
 * @brief 重新分配内存大小。
 * @param ptr 原内存地址。
 * @param nbytes 新的字节数。
 * @return void* 成功返回新地址，失败返回 NULL。
 */
void *rt_realloc(void *ptr, size_t nbytes);

/**
 * @brief 分配内存并清零。
 * @param count 元素个数。
 * @param size  每个元素的大小。
 * @return void* 成功返回内存首地址，失败返回 NULL。
 */
void *rt_calloc(size_t count, size_t size);

/**
 * @brief 对齐分配内存。
 * @param size  需要分配的字节数。
 * @param align 对齐字节数（必须是 2 的幂）。
 * @return void* 成功返回对齐后的内存地址。
 */
void *rt_malloc_align(size_t size, size_t align);

/**
 * @brief 释放对齐分配的内存。
 * @param ptr 需要释放的内存地址。
 */
void rt_free_align(void *ptr);

/**
 * @brief 获取内存堆使用信息。
 * @param total    输出：总堆大小。
 * @param used     输出：已使用堆大小。
 * @param max_used 输出：历史最大使用量。
 */
void rt_memory_info(size_t *total, size_t *used, size_t *max_used);

#ifdef RT_USING_HOOK
/**
 * @brief 设置内存分配钩子函数。
 * @param hook 钩子函数指针，函数原型为 void (*)(void *ptr, size_t size)。
 */
void rt_malloc_sethook(void (*hook)(void *ptr, size_t size));

/**
 * @brief 设置内存释放钩子函数。
 * @param hook 钩子函数指针，函数原型为 void (*)(void *ptr)。
 */
void rt_free_sethook(void (*hook)(void *ptr));
#endif

#ifdef RT_USING_SMALL_MEM
/**
 * @brief 初始化小内存管理算法。
 * @param name       内存池名称。
 * @param begin_addr 内存池起始地址。
 * @param size       内存池大小。
 * @return rt_smem_t 成功返回句柄，失败返回 NULL。
 */
rt_smem_t rt_smem_init(const char *name, void *begin_addr, size_t size);

/**
 * @brief 分离（卸载）小内存管理对象。
 * @param m 内存管理器句柄。
 * @return rt_err_t 成功返回 RT_EOK。
 */
rt_err_t rt_smem_detach(rt_smem_t m);

/**
 * @brief 从指定小内存池中分配内存。
 * @param m    内存管理器句柄。
 * @param size 分配大小。
 * @return void* 成功返回内存地址。
 */
void *rt_smem_alloc(rt_smem_t m, size_t size);

/**
 * @brief 重新分配小内存块大小。
 * @param m    内存管理器句柄。
 * @param rmem 原内存地址。
 * @param newsize 新的大小。
 * @return void* 成功返回新地址。
 */
void *rt_smem_realloc(rt_smem_t m, void *rmem, size_t newsize);

/**
 * @brief 释放小内存块。
 * @param rmem 需要释放的内存地址。
 */
void rt_smem_free(void *rmem);
#endif

#ifdef RT_USING_SLAB
/**
 * @brief 初始化 SLAB 内存管理算法。
 * @param name       SLAB 内存池名称。
 * @param begin_addr 内存池起始地址。
 * @param size       内存池大小。
 * @return rt_slab_t 成功返回句柄，失败返回 NULL。
 */
rt_slab_t rt_slab_init(const char *name, void *begin_addr, size_t size);

/**
 * @brief 分离 SLAB 内存管理器。
 * @param m 内存管理器句柄。
 * @return rt_err_t 成功返回 RT_EOK。
 */
rt_err_t rt_slab_detach(rt_slab_t m);

/**
 * @brief 分配 SLAB 页。
 * @param m      内存管理器句柄。
 * @param npages 页数量。
 * @return void* 成功返回内存地址。
 */
void *rt_slab_page_alloc(rt_slab_t m, size_t npages);

/**
 * @brief 释放 SLAB 页。
 * @param m      内存管理器句柄。
 * @param addr   内存起始地址。
 * @param npages 页数量。
 */
void rt_slab_page_free(rt_slab_t m, void *addr, size_t npages);

/**
 * @brief 从 SLAB 中分配内存。
 * @param m    内存管理器句柄。
 * @param size 大小。
 * @return void* 成功返回内存地址。
 */
void *rt_slab_alloc(rt_slab_t m, size_t size);

/**
 * @brief 调整 SLAB 分配的内存大小。
 * @param m    内存管理器句柄。
 * @param ptr  原内存地址。
 * @param size 新的大小。
 * @return void* 成功返回新地址。
 */
void *rt_slab_realloc(rt_slab_t m, void *ptr, size_t size);

/**
 * @brief 释放 SLAB 内存。
 * @param m   内存管理器句柄。
 * @param ptr 需要释放的内存地址。
 */
void rt_slab_free(rt_slab_t m, void *ptr);

#ifdef RT_USING_SLAB_AS_HEAP
/**
 * @brief 分配系统页（作为通用堆使用）。
 * @param npages 页数量。
 * @return void* 成功返回内存地址。
 */
void *rt_page_alloc(size_t npages);

/**
 * @brief 释放系统页。
 * @param addr   内存地址，
 * @param npages 页数量，
 */
void rt_page_free(void *addr, size_t npages);
#endif
#endif
#endif

#ifdef RT_USING_MEMHEAP
/**
 * @brief 初始化内存堆对象。
 * @param memheap    内存堆对象指针。
 * @param name       内存堆对象的名称。
 * @param start_addr 内存堆的起始地址。
 * @param size       内存堆的总大小（字节）。
 * @return rt_err_t  成功返回 RT_EOK，失败返回错误码。
 */
rt_err_t rt_memheap_init(struct rt_memheap *memheap, const char *name,
                         void *start_addr, size_t size);

/**
 * @brief 分离内存堆对象。
 * @param heap 内存堆对象指针。
 * @return rt_err_t 成功返回 RT_EOK，失败返回错误码。
 */
rt_err_t rt_memheap_detach(struct rt_memheap *heap);

/**
 * @brief 从指定的内存堆中分配内存。
 * @param heap 内存堆对象指针。
 * @param size 需要分配的字节数。
 * @return void* 成功返回内存首地址，失败返回 NULL。
 */
void *rt_memheap_alloc(struct rt_memheap *heap, size_t size);

/**
 * @brief 重新调整内存堆中内存块的大小。
 * @param heap    内存堆对象指针。
 * @param ptr     原内存地址。
 * @param newsize 新的大小。
 * @return void*  成功返回调整后的内存地址，失败返回 NULL。
 */
void *rt_memheap_realloc(struct rt_memheap *heap, void *ptr, size_t newsize);

/**
 * @brief 释放内存堆中的内存块。
 * @param ptr 需要释放的内存地址。
 */
void rt_memheap_free(void *ptr);

/**
 * @brief 获取内存堆的状态信息。
 * @param heap     内存堆对象指针。
 * @param total    输出：总大小。
 * @param used     输出：已使用大小。
 * @param max_used 输出：历史最大使用量。
 */
void rt_memheap_info(struct rt_memheap *heap, size_t *total, size_t *used,
                     size_t *max_used);
#endif

#ifdef RT_USING_SEMAPHORE
/**
 * @brief 初始化一个静态信号量对象。
 * @param sem   信号量对象句柄。
 * @param name  信号量名称。
 * @param value 初始信号量值。
 * @param flag  信号量标志，例如 RT_IPC_FLAG_FIFO 或 RT_IPC_FLAG_PRIO。
 * @return rt_err_t 成功返回 RT_EOK，失败返回错误码。
 */
rt_err_t rt_sem_init(rt_sem_t sem, const char *name, uint32_t value,
                     uint8_t flag);

/**
 * @brief 脱离（删除）静态信号量对象。
 * @param sem 信号量对象句柄。
 * @return rt_err_t 成功返回 RT_EOK，失败返回错误码。
 */
rt_err_t rt_sem_detach(rt_sem_t sem);

#ifdef RT_USING_HEAP
/**
 * @brief 创建并初始化一个动态信号量对象。
 * @param name  信号量名称。
 * @param value 初始信号量值。
 * @param flag  信号量标志，例如 RT_IPC_FLAG_FIFO 或 RT_IPC_FLAG_PRIO。
 * @return rt_sem_t 成功返回信号量句柄，失败返回 RT_NULL。
 */
rt_sem_t rt_sem_create(const char *name, uint32_t value, uint8_t flag);

/**
 * @brief 删除一个动态信号量对象。
 * @param sem 信号量对象句柄。
 * @return rt_err_t 成功返回 RT_EOK，失败返回错误码。
 */
rt_err_t rt_sem_delete(rt_sem_t sem);
#endif

/**
 * @brief 获取信号量（等待）。
 * @param sem     信号量对象句柄。
 * @param timeout 等待时间（以时钟节拍为单位，RT_WAITING_FOREVER
 * 表示永久等待）。
 * @return rt_err_t 成功返回 RT_EOK，超时或出错返回错误码。
 */
rt_err_t rt_sem_take(rt_sem_t sem, int32_t timeout);

/**
 * @brief 尝试获取信号量（非阻塞）。
 * @param sem 信号量对象句柄。
 * @return rt_err_t 成功返回 RT_EOK，信号量不可用返回 -RT_ETIMEOUT 或
 * -RT_ERROR。
 */
rt_err_t rt_sem_trytake(rt_sem_t sem);

/**
 * @brief 释放信号量。
 * @param sem 信号量对象句柄。
 * @return rt_err_t 成功返回 RT_EOK，失败返回错误码。
 */
rt_err_t rt_sem_release(rt_sem_t sem);

/**
 * @brief 控制信号量。
 * @param sem 信号量对象句柄。
 * @param cmd 命令字。
 * @param arg 参数指针。
 * @return rt_err_t 成功返回 RT_EOK，失败返回错误码。
 */
rt_err_t rt_sem_control(rt_sem_t sem, int cmd, void *arg);
#endif

#ifdef RT_USING_MUTEX
/**
 * @brief 初始化一个静态互斥量对象。
 * @param mutex 互斥量对象句柄。
 * @param name  互斥量名称。
 * @param flag  互斥量标志（RT_IPC_FLAG_FIFO 或 RT_IPC_FLAG_PRIO）。
 * @return rt_err_t 成功返回 RT_EOK，失败返回错误码。
 */
rt_err_t rt_mutex_init(rt_mutex_t mutex, const char *name, uint8_t flag);

/**
 * @brief 脱离（删除）静态互斥量对象。
 * @param mutex 互斥量对象句柄。
 * @return rt_err_t 成功返回 RT_EOK，失败返回错误码。
 */
rt_err_t rt_mutex_detach(rt_mutex_t mutex);

#ifdef RT_USING_HEAP
/**
 * @brief 创建并初始化一个动态互斥量对象。
 * @param name 互斥量名称。
 * @param flag 互斥量标志。
 * @return rt_mutex_t 成功返回互斥量句柄，失败返回 RT_NULL。
 */
rt_mutex_t rt_mutex_create(const char *name, uint8_t flag);

/**
 * @brief 删除一个动态互斥量对象。
 * @param mutex 互斥量对象句柄。
 * @return rt_err_t 成功返回 RT_EOK，失败返回错误码。
 */
rt_err_t rt_mutex_delete(rt_mutex_t mutex);
#endif

/**
 * @brief 获取互斥量（加锁）。
 * @param mutex   互斥量对象句柄。
 * @param timeout 等待时间（以时钟节拍为单位，RT_WAITING_FOREVER
 * 表示永久等待）。
 * @return rt_err_t 成功返回 RT_EOK，失败返回错误码。
 */
rt_err_t rt_mutex_take(rt_mutex_t mutex, int32_t timeout);

/**
 * @brief 尝试获取互斥量（非阻塞）。
 * @param mutex 互斥量对象句柄。
 * @return rt_err_t 成功返回 RT_EOK，如果互斥量被占用返回 -RT_ETIMEOUT。
 */
rt_err_t rt_mutex_trytake(rt_mutex_t mutex);

/**
 * @brief 释放互斥量（解锁）。
 * @param mutex 互斥量对象句柄。
 * @return rt_err_t 成功返回 RT_EOK，失败返回错误码。
 */
rt_err_t rt_mutex_release(rt_mutex_t mutex);

/**
 * @brief 控制互斥量。
 * @param mutex 互斥量对象句柄。
 * @param cmd   控制命令。
 * @param arg   参数指针。
 * @return rt_err_t 成功返回 RT_EOK，失败返回错误码。
 */
rt_err_t rt_mutex_control(rt_mutex_t mutex, int cmd, void *arg);
#endif

#ifdef RT_USING_EVENT
/**
 * @brief 初始化一个静态事件集对象。
 * @param event 事件集对象句柄。
 * @param name  事件集名称。
 * @param flag  事件集标志（RT_IPC_FLAG_FIFO 或 RT_IPC_FLAG_PRIO）。
 * @return rt_err_t 成功返回 RT_EOK，失败返回错误码。
 */
rt_err_t rt_event_init(rt_event_t event, const char *name, uint8_t flag);

/**
 * @brief 脱离（删除）静态事件集对象。
 * @param event 事件集对象句柄。
 * @return rt_err_t 成功返回 RT_EOK，失败返回错误码。
 */
rt_err_t rt_event_detach(rt_event_t event);

#ifdef RT_USING_HEAP
/**
 * @brief 创建并初始化一个动态事件集对象。
 * @param name 事件集名称。
 * @param flag 事件集标志。
 * @return rt_event_t 成功返回事件集句柄，失败返回 RT_NULL。
 */
rt_event_t rt_event_create(const char *name, uint8_t flag);

/**
 * @brief 删除一个动态事件集对象。
 * @param event 事件集对象句柄。
 * @return rt_err_t 成功返回 RT_EOK，失败返回错误码。
 */
rt_err_t rt_event_delete(rt_event_t event);
#endif

/**
 * @brief 发送事件集。
 * @param event 事件集对象句柄。
 * @param set   要发送的事件标志位（位图）。
 * @return rt_err_t 成功返回 RT_EOK。
 */
rt_err_t rt_event_send(rt_event_t event, uint32_t set);

/**
 * @brief 接收事件集。
 * @param event   事件集对象句柄。
 * @param set     感兴趣的事件标志位。
 * @param opt     接收选项（RT_EVENT_FLAG_AND 或 RT_EVENT_FLAG_OR）。
 * @param timeout 等待时间。
 * @param recved  输出：实际接收到的事件标志位。
 * @return rt_err_t 成功返回 RT_EOK，超时返回 -RT_ETIMEOUT。
 */
rt_err_t rt_event_recv(rt_event_t event, uint32_t set, uint8_t opt,
                       int32_t timeout, uint32_t *recved);

/**
 * @brief 控制事件集。
 * @param event 事件集对象句柄。
 * @param cmd   控制命令。
 * @param arg   参数指针。
 * @return rt_err_t 成功返回 RT_EOK，失败返回错误码。
 */
rt_err_t rt_event_control(rt_event_t event, int cmd, void *arg);
#endif

#ifdef RT_USING_MAILBOX
/**
 * @brief 初始化一个静态邮箱对象。
 * @param mb      邮箱对象句柄。
 * @param name    邮箱名称。
 * @param msgpool 邮箱消息缓冲区起始地址。
 * @param size    邮箱容量（存放的消息数）。
 * @param flag    邮箱标志（RT_IPC_FLAG_FIFO 或 RT_IPC_FLAG_PRIO）。
 * @return rt_err_t 成功返回 RT_EOK，失败返回错误码。
 */
rt_err_t rt_mb_init(rt_mailbox_t mb, const char *name, void *msgpool,
                    size_t size, uint8_t flag);

/**
 * @brief 脱离（删除）静态邮箱对象。
 * @param mb 邮箱对象句柄。
 * @return rt_err_t 成功返回 RT_EOK，失败返回错误码。
 */
rt_err_t rt_mb_detach(rt_mailbox_t mb);

#ifdef RT_USING_HEAP
/**
 * @brief 创建并初始化一个动态邮箱对象。
 * @param name 邮箱名称。
 * @param size 邮箱容量。
 * @param flag 邮箱标志。
 * @return rt_mailbox_t 成功返回邮箱句柄，失败返回 RT_NULL。
 */
rt_mailbox_t rt_mb_create(const char *name, size_t size, uint8_t flag);

/**
 * @brief 删除一个动态邮箱对象。
 * @param mb 邮箱对象句柄。
 * @return rt_err_t 成功返回 RT_EOK，失败返回错误码。
 */
rt_err_t rt_mb_delete(rt_mailbox_t mb);
#endif

/**
 * @brief 发送邮件到邮箱。
 * @param mb    邮箱对象句柄。
 * @param value 邮件内容（指针或整数）。
 * @return rt_err_t 成功返回 RT_EOK，邮箱满返回 -RT_EFULL。
 */
rt_err_t rt_mb_send(rt_mailbox_t mb, rt_ubase_t value);

/**
 * @brief 带等待功能的发送邮件。
 * @param mb      邮箱对象句柄。
 * @param value   邮件内容。
 * @param timeout 超时时间。
 * @return rt_err_t 成功返回 RT_EOK，超时返回 -RT_ETIMEOUT。
 */
rt_err_t rt_mb_send_wait(rt_mailbox_t mb, rt_ubase_t value, int32_t timeout);

/**
 * @brief 发送紧急邮件（插队到队列头部）。
 * @param mb    邮箱对象句柄。
 * @param value 邮件内容。
 * @return rt_err_t 成功返回 RT_EOK。
 */
rt_err_t rt_mb_urgent(rt_mailbox_t mb, rt_ubase_t value);

/**
 * @brief 从邮箱接收邮件。
 * @param mb      邮箱对象句柄。
 * @param value   输出：接收到的邮件内容。
 * @param timeout 超时时间。
 * @return rt_err_t 成功返回 RT_EOK，空邮箱返回 -RT_ETIMEOUT。
 */
rt_err_t rt_mb_recv(rt_mailbox_t mb, rt_ubase_t *value, int32_t timeout);

/**
 * @brief 控制邮箱。
 * @param mb  邮箱对象句柄。
 * @param cmd 命令字。
 * @param arg 参数指针。
 * @return rt_err_t 成功返回 RT_EOK。
 */
rt_err_t rt_mb_control(rt_mailbox_t mb, int cmd, void *arg);
#endif

#ifdef RT_USING_MESSAGEQUEUE
/**
 * @brief 初始化一个静态消息队列对象。
 * @param mq        消息队列对象句柄。
 * @param name      消息队列名称。
 * @param msgpool   消息缓冲区起始地址。
 * @param msg_size  单条消息的大小。
 * @param pool_size 消息缓冲区总大小。
 * @param flag      消息队列标志。
 * @return rt_err_t 成功返回 RT_EOK。
 */
rt_err_t rt_mq_init(rt_mq_t mq, const char *name, void *msgpool,
                    size_t msg_size, size_t pool_size, uint8_t flag);

/**
 * @brief 脱离（删除）静态消息队列对象。
 * @param mq 消息队列对象句柄。
 * @return rt_err_t 成功返回 RT_EOK。
 */
rt_err_t rt_mq_detach(rt_mq_t mq);

#ifdef RT_USING_HEAP
/**
 * @brief 创建并初始化一个动态消息队列对象。
 * @param name     消息队列名称。
 * @param msg_size 单条消息大小。
 * @param max_msgs 最大消息条数。
 * @param flag     消息队列标志。
 * @return rt_mq_t 成功返回消息队列句柄。
 */
rt_mq_t rt_mq_create(const char *name, size_t msg_size, size_t max_msgs,
                     uint8_t flag);

/**
 * @brief 删除一个动态消息队列对象。
 * @param mq 消息队列对象句柄。
 * @return rt_err_t 成功返回 RT_EOK。
 */
rt_err_t rt_mq_delete(rt_mq_t mq);
#endif

/**
 * @brief 发送消息到消息队列。
 * @param mq     消息队列对象句柄。
 * @param buffer 消息内容数据指针。
 * @param size   数据大小。
 * @return rt_err_t 成功返回 RT_EOK。
 */
rt_err_t rt_mq_send(rt_mq_t mq, const void *buffer, size_t size);

/**
 * @brief 带等待功能的发送消息。
 * @param mq      消息队列对象句柄。
 * @param buffer  消息内容数据指针。
 * @param size    数据大小。
 * @param timeout 超时时间。
 * @return rt_err_t 成功返回 RT_EOK。
 */
rt_err_t rt_mq_send_wait(rt_mq_t mq, const void *buffer, size_t size,
                         int32_t timeout);

/**
 * @brief 发送紧急消息（插队到队列头部）。
 * @param mq     消息队列对象句柄。
 * @param buffer 消息内容数据指针。
 * @param size   数据大小。
 * @return rt_err_t 成功返回 RT_EOK。
 */
rt_err_t rt_mq_urgent(rt_mq_t mq, const void *buffer, size_t size);

/**
 * @brief 接收消息队列中的消息。
 * @param mq      消息队列对象句柄。
 * @param buffer  存放接收数据的缓冲区。
 * @param size    缓冲区大小。
 * @param timeout 超时时间。
 * @return rt_err_t 成功返回 RT_EOK。
 */
rt_err_t rt_mq_recv(rt_mq_t mq, void *buffer, size_t size, int32_t timeout);

/**
 * @brief 控制消息队列。
 * @param mq  消息队列对象句柄。
 * @param cmd 命令字。
 * @param arg 参数指针。
 * @return rt_err_t 成功返回 RT_EOK。
 */
rt_err_t rt_mq_control(rt_mq_t mq, int cmd, void *arg);
#endif

/**
 * @brief 将结束的线程加入到 defunct 列表（内部使用）。
 * @param thread 待清理的线程控制块指针。
 */
void rt_thread_defunct_enqueue(rt_thread_t thread);

/**
 * @brief 从 defunct 列表中取出结束的线程（内部使用）。
 * @return rt_thread_t 返回线程控制块指针或 RT_NULL。
 */
rt_thread_t rt_thread_defunct_dequeue(void);

#ifdef RT_USING_SMP
/**
 * @brief 自旋锁对象结构体声明。
 */
struct rt_spinlock;

/**
 * @brief 初始化自旋锁。
 * @param lock 指向自旋锁对象的指针。
 */
void rt_spin_lock_init(struct rt_spinlock *lock);

/**
 * @brief 获取自旋锁（在 SMP 系统中会一直自旋等待，直至获取锁）。
 * @param lock 指向自旋锁对象的指针。
 */
void rt_spin_lock(struct rt_spinlock *lock);

/**
 * @brief 释放自旋锁。
 * @param lock 指向自旋锁对象的指针。
 */
void rt_spin_unlock(struct rt_spinlock *lock);

/**
 * @brief 获取自旋锁并关闭中断（保存当前中断状态）。
 * @param lock 指向自旋锁对象的指针。
 * @return rt_base_t 返回关闭中断前的中断状态值。
 */
rt_base_t rt_spin_lock_irqsave(struct rt_spinlock *lock);

/**
 * @brief 释放自旋锁并恢复中断状态。
 * @param lock  指向自旋锁对象的指针。
 * @param level 使用 rt_spin_lock_irqsave() 获取的中断状态值。
 */
void rt_spin_unlock_irqrestore(struct rt_spinlock *lock, rt_base_t level);

#else
/* 以下为单核架构下的兼容宏定义 */
/** @brief 在非 SMP 系统中，rt_spin_lock_init 为空操作 */
#define rt_spin_lock_init(lock)

/** @brief 在非 SMP 系统中，rt_spin_lock 等价于进入临界区 */
#define rt_spin_lock(lock)                     rt_enter_critical()

/** @brief 在非 SMP 系统中，rt_spin_unlock 等价于退出临界区 */
#define rt_spin_unlock(lock)                   rt_exit_critical()

/**
 * @brief 在非 SMP 系统中，rt_spin_lock_irqsave 等价于关闭中断 。
 * @return 返回关闭中断前的状态值。
 */
#define rt_spin_lock_irqsave(lock)             rt_hw_interrupt_disable()

/**
 * @brief 在非 SMP 系统中，rt_spin_unlock_irqrestore 等价于恢复中断 。
 * @param level 中断状态值。
 */
#define rt_spin_unlock_irqrestore(lock, level) rt_hw_interrupt_enable(level)
#endif

#ifdef RT_USING_DEVICE
/**
 * @brief 根据名称查找系统设备。
 * @param name 设备名称。
 * @return rt_device_t 成功返回设备句柄，失败返回 NULL。
 */
rt_device_t rt_device_find(const char *name);

/**
 * @brief 注册设备到内核对象系统中。
 * @param dev   设备句柄。
 * @param name  设备名称。
 * @param flags 设备标志（如 RT_DEVICE_FLAG_RDWR 等）。
 * @return rt_err_t 成功返回 RT_EOK，失败返回错误码。
 */
rt_err_t rt_device_register(rt_device_t dev, const char *name, uint16_t flags);

/**
 * @brief 从内核对象系统中注销设备。
 * @param dev 设备句柄。
 * @return rt_err_t 成功返回 RT_EOK。
 */
rt_err_t rt_device_unregister(rt_device_t dev);

#ifdef RT_USING_HEAP
/**
 * @brief 创建一个新的设备对象（动态分配内存）。
 * @param type        设备类型。
 * @param attach_size 附加数据大小。
 * @return rt_device_t 成功返回设备句柄，失败返回 NULL。
 */
rt_device_t rt_device_create(int type, int attach_size);

/**
 * @brief 销毁设备对象（释放内存）。
 * @param device 设备句柄。
 */
void rt_device_destroy(rt_device_t device);
#endif

/**
 * @brief 设置接收回调函数（当有数据到来时调用）。
 * @param dev    设备句柄。
 * @param rx_ind 回调函数指针。
 * @return rt_err_t 成功返回 RT_EOK。
 */
rt_err_t rt_device_set_rx_indicate(rt_device_t dev,
                                   rt_err_t (*rx_ind)(rt_device_t dev,
                                                      size_t size));

/**
 * @brief 设置发送完成回调函数。
 * @param dev     设备句柄。
 * @param tx_done 回调函数指针。
 * @return rt_err_t 成功返回 RT_EOK。
 */
rt_err_t rt_device_set_tx_complete(rt_device_t dev,
                                   rt_err_t (*tx_done)(rt_device_t dev,
                                                       void *buffer));

/**
 * @brief 初始化设备。
 * @param dev 设备句柄。
 * @return rt_err_t 成功返回 RT_EOK。
 */
rt_err_t rt_device_init(rt_device_t dev);

/**
 * @brief 打开设备。
 * @param dev   设备句柄。
 * @param oflag 打开方式标志（如 RT_DEVICE_OFLAG_RDONLY 等）。
 * @return rt_err_t 成功返回 RT_EOK。
 */
rt_err_t rt_device_open(rt_device_t dev, uint16_t oflag);

/**
 * @brief 关闭设备。
 * @param dev 设备句柄。
 * @return rt_err_t 成功返回 RT_EOK。
 */
rt_err_t rt_device_close(rt_device_t dev);

/**
 * @brief 从设备读取数据。
 * @param dev    设备句柄。
 * @param pos    读取数据的偏移量。
 * @param buffer 数据缓冲区。
 * @param size   读取大小。
 * @return size_t 返回实际读取的数据大小。
 */
size_t rt_device_read(rt_device_t dev, rt_off_t pos, void *buffer, size_t size);

/**
 * @brief 向设备写入数据。
 * @param dev    设备句柄。
 * @param pos    写入数据的偏移量。
 * @param buffer 数据缓冲区。
 * @param size   写入大小。
 * @return size_t 返回实际写入的数据大小。
 */
size_t rt_device_write(rt_device_t dev, rt_off_t pos, const void *buffer,
                       size_t size);

/**
 * @brief 控制设备配置。
 * @param dev 设备句柄。
 * @param cmd 命令字。
 * @param arg 参数指针。
 * @return rt_err_t 成功返回 RT_EOK。
 */
rt_err_t rt_device_control(rt_device_t dev, int cmd, void *arg);
#endif

/**
 * @brief 通知内核进入中断上下文。
 * @note 此函数通常在中断服务例程（ISR）的最开始调用，用于记录中断嵌套深度。
 */
void rt_interrupt_enter(void);

/**
 * @brief 通知内核离开中断上下文。
 * @note 此函数通常在中断服务例程的最后调用，
 * 用于更新中断嵌套深度，并可能触发任务调度。
 */
void rt_interrupt_leave(void);

#ifdef RT_USING_HOOK
/**
 * @brief 设置进入中断钩子函数。
 * @param hook 当调用 rt_interrupt_enter 时触发的钩子函数。
 */
void rt_interrupt_enter_sethook(void (*hook)(void));

/**
 * @brief 设置离开中断钩子函数。
 * @param hook 当调用 rt_interrupt_leave 时触发的钩子函数。
 */
void rt_interrupt_leave_sethook(void (*hook)(void));
#endif

/**
 * @brief 获取当前中断嵌套深度。
 * @return uint8_t 中断嵌套深度，0 表示处于线程上下文。
 */
uint8_t rt_interrupt_get_nest(void);

#ifdef RT_USING_SMP
/**
 * @brief 获取多核锁（禁止调度并锁定 CPU）。
 * @return rt_base_t 返回当前锁的状态（用于后续解锁操作）。
 */
rt_base_t rt_cpus_lock(void);

/**
 * @brief 释放多核锁。
 * @param level 由 rt_cpus_lock 返回的锁状态值。
 */
void rt_cpus_unlock(rt_base_t level);

/**
 * @brief 获取当前执行任务的 CPU 结构体对象。
 * @return struct rt_cpu* 当前 CPU 结构体指针。
 */
struct rt_cpu *rt_cpu_self(void);

/**
 * @brief 根据索引获取指定 CPU 的结构体对象。
 * @param index CPU 编号。
 * @return struct rt_cpu* 指定 CPU 的结构体指针。
 */
struct rt_cpu *rt_cpu_index(int index);
#endif

#ifdef RT_USING_COMPONENTS_INIT
/**
 * @brief 执行系统自动初始化的组件。
 * @note 该函数通常在系统启动阶段调用，按照初始化级别（如 board, pre, device,
 * app）依次执行注册的初始化函数。
 */
void rt_components_init(void);
#endif

/**
 * @brief 系统内核格式化输出函数。
 * @param fmt 格式化字符串。
 * @param ... 可变参数。
 * @return int 实际打印的字符个数。
 */
int rt_kprintf(const char *fmt, ...);

#if defined(RT_USING_DEVICE) && defined(RT_USING_CONSOLE)
/**
 * @brief 设置系统控制台设备。
 * @param name 控制台设备名称（如 "uart1"）。
 * @return rt_device_t 返回切换前的旧控制台设备句柄。
 */
rt_device_t rt_console_set_device(const char *name);

/**
 * @brief 获取当前系统控制台设备。
 * @return rt_device_t 返回当前控制台设备句柄。
 */
rt_device_t rt_console_get_device(void);
#endif

/**
 * @brief 获取当前线程的错误码。
 * @return rt_err_t 当前错误码。
 */
rt_err_t rt_get_errno(void);

/**
 * @brief 设置当前线程的错误码。
 * @param no 需要设置的错误码。
 */
void rt_set_errno(rt_err_t no);

/**
 * @brief 获取当前线程错误码的地址（内部函数）。
 * @return int* 指向线程局部变量 errno 的指针。
 */
int *_rt_errno(void);

/**
 * @brief 根据错误码获取错误描述字符串。
 * @param error 错误码。
 * @return const char* 错误描述字符串指针。
 */
const char *rt_strerror(rt_err_t error);

#if !defined(RT_USING_NEWLIB) && !defined(_WIN32)
#ifndef errno
/**
 * @brief 宏定义 errno，方便通过全局变量访问当前线程错误码 。
 */
#define errno *_rt_errno()
#endif
#endif

/**
 * @brief 查找数值中第一个为 1 的位（从低位到高位）。
 * @param value 待查找的数值。
 * @return int 第一个为 1 的位的索引（从 1 开始），若输入为 0 则返回 0。
 */
int __rt_ffs(int value);

/**
 * @brief 在控制台显示当前 RT-Thread 内核版本信息。
 */
void rt_show_version(void);

#ifdef RT_USING_FINSH
/* 包含 FinSH 控制台相关头文件 */
#include <finsh.h>
#endif

#endif