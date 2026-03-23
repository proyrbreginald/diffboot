/**
 * @file rtdef.h
 * @author reginald.yang (proyrb@yeah.net)
 * @version 0.1
 * @date 2026-03-21
 * @copyright Copyright (c) 2026
 * @brief 提供rtthread类型定义。
 */

#ifndef _RT_DEF_H_
#define _RT_DEF_H_

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <rtconfig.h>

/**
 * @brief rtthread版本定义。
 */
#define RT_VERSION    4u //!< 不兼容更新 */
#define RT_SUBVERSION 1u //!< 新增特性 */
#define RT_REVISION   1u //!< 修复问题 */

/**
 * @brief 返回rtthread版本号的int32_t值。
 */
#define RTTHREAD_VERSION VERSION_UINT32(RT_VERSION, RT_SUBVERSION, RT_REVISION)

/**
 * @brief 定义rtthread基本类型。
 */
typedef long rt_base_t;
typedef unsigned long rt_ubase_t;
typedef uint32_t rt_time_t;
typedef uint32_t rt_tick_t;
typedef rt_base_t rt_flag_t;
typedef rt_ubase_t rt_dev_t;
typedef rt_base_t rt_off_t;

/**
 * @brief 定义rtthread基本类型的取值范围。
 */
#define RT_TICK_MAX        UINT32_MAX
#define RT_SEM_VALUE_MAX   UINT16_MAX
#define RT_MUTEX_VALUE_MAX UINT16_MAX
#define RT_MUTEX_HOLD_MAX  UINT8_MAX
#define RT_MB_ENTRY_MAX    UINT16_MAX
#define RT_MQ_ENTRY_MAX    UINT16_MAX

/**
 * @brief 在rtthread启动阶段自动执行函数。
 */
#ifdef RT_USING_COMPONENTS_INIT
typedef int (*int_fn_void_t)(void);
#define RT_LAUNCH_RUN_EXPORT(fn, level)                                        \
    USED const int_fn_void_t rt_init_##fn SECTION(".rt_launch_run." level) = fn
#else
#define RT_LAUNCH_RUN_EXPORT(fn, level)
#endif

/**
 * @brief 分阶段自动执行，数值越小越先执行。
 */
#define RUN_PREV_EXPORT(fn)   RT_LAUNCH_RUN_EXPORT(fn, "1")
#define RUN_BOARD_EXPORT(fn)  RT_LAUNCH_RUN_EXPORT(fn, "2")
#define RUN_DEVICE_EXPORT(fn) RT_LAUNCH_RUN_EXPORT(fn, "3")
#define RUN_PLUGIN_EXPORT(fn) RT_LAUNCH_RUN_EXPORT(fn, "4")
#define RUN_ENV_EXPORT(fn)    RT_LAUNCH_RUN_EXPORT(fn, "5")
#define RUN_APP_EXPORT(fn)    RT_LAUNCH_RUN_EXPORT(fn, "6")

/**
 * @brief 如果没有导入finsh则定义为空，避免编译报错。
 */
#if !defined(RT_USING_FINSH)
#define FINSH_FUNCTION_EXPORT(name, desc)
#define FINSH_FUNCTION_EXPORT_ALIAS(name, alias, desc)
#define MSH_CMD_EXPORT(command, desc)
#define MSH_CMD_EXPORT_ALIAS(command, alias, desc)
#elif !defined(FINSH_USING_SYMTAB)
#define FINSH_FUNCTION_EXPORT_CMD(name, cmd, desc)
#endif

/**
 * @brief 内存管理选项。
 */
#define RT_MM_PAGE_SIZE 4096
#define RT_MM_PAGE_MASK (RT_MM_PAGE_SIZE - 1)
#define RT_MM_PAGE_BITS 12

/**
 * @brief 内核内存分配。
 */
#ifndef RT_KERNEL_MALLOC
#define RT_KERNEL_MALLOC(size) rt_malloc(size)
#endif

/**
 * @brief 内核内存释放。
 */
#ifndef RT_KERNEL_FREE
#define RT_KERNEL_FREE(ptr) rt_free(ptr)
#endif

/**
 * @brief 内核内存重新分配。
 */
#ifndef RT_KERNEL_REALLOC
#define RT_KERNEL_REALLOC(ptr, size) rt_realloc(ptr, size)
#endif

/**
 * @brief 定义rtthread错误枚举。
 */
typedef enum rt_err_t {
    RT_EOK = 0,  //!< 无错误
    RT_ERROR,    //!< 通用错误
    RT_ETIMEOUT, //!< 超时
    RT_EFULL,    //!< 资源已满
    RT_EEMPTY,   //!< 资源为空
    RT_ENOMEM,   //!< 无可用内存
    RT_ENOSYS,   //!< 操作接口未适配
    RT_EBUSY,    //!< 被占用
    RT_EIO,      //!< 输入输出错误
    RT_EINTR,    //!< 在中断中调用
    RT_EINVAL,   //!< 无效参数
} rt_err_t;

/**
 * @brief 返回最接近指定宽度对齐的连续大小。RT_ALIGN(13, 4)将返回 16。
 */
#define RT_ALIGN(size, align) (((size) + (align) - 1) & ~((align) - 1))

/**
 * @brief 返回指定宽度下的向下对齐数。RT_ALIGN_DOWN(13, 4)将返回 12。
 */
#define RT_ALIGN_DOWN(size, align) ((size) & ~((align) - 1))

/* 环形缓冲区结构体 */
struct rt_ringbuffer {
    uint8_t *buffer_ptr;
    uint16_t read_mirror : 1;
    uint16_t read_index : 15;
    uint16_t write_mirror : 1;
    uint16_t write_index : 15;
    uint16_t buffer_size;
};

/**
 * @brief 双向链表。
 */
struct rt_list_node {
    struct rt_list_node *next; //!< 下一个节点
    struct rt_list_node *prev; //!< 上一个节点
};
typedef struct rt_list_node rt_dlist_t;

/**
 * @brief 单向链表。
 */
struct rt_slist_node {
    struct rt_slist_node *next; //!< 下一个节点
};
typedef struct rt_slist_node rt_slist_t;

/**
 * @brief 内核对象结构体。
 */
struct rt_object {
    char name[RT_NAME_MAX]; //!< 对象名称
    uint8_t type;           //!< 对象类型
    uint8_t flag;           //!< 状态标志
#ifdef RT_USING_MODULE
    void *module_id; //!< 应用程序模块id
#endif
    rt_dlist_t list; //!< 内核对象的双向列表节点
};
typedef struct rt_object *rt_object_t;

/**
 * @brief 内核对象类型枚举。
 */
enum rt_object_type {
    RT_OBJ_TYPE_NULL      = 0x00, //!< 未使用
    RT_OBJ_TYPE_THREAD    = 0x01, //!< 线程
    RT_OBJ_TYPE_SEM       = 0x02, //!< 信号量
    RT_OBJ_TYPE_MUTEX     = 0x03, //!< 互斥锁
    RT_Object_Class_Event = 0x04, //!< 事件集
    RT_OBJ_TYPE_MAIL      = 0x05, //!< 邮箱
    RT_OBJ_TYPE_QUEUE     = 0x06, //!< 消息队列
    RT_OBJ_TYPE_HEAP      = 0x07, //!< 内存堆
    RT_OBJ_TYPE_POOL      = 0x08, //!< 内存池
    RT_OBJ_TYPE_DEVICE    = 0x09, //!< 设备
    RT_OBJ_TYPE_TIMER     = 0x0a, //!< 定时器
    RT_OBJ_TYPE_MODULE    = 0x0b, //!< 模块
    RT_OBJ_TYPE_MEM       = 0x0c, //!< 内存
    RT_OBJ_TYPE_UNKNOWN   = 0x0e, //!< 未知
    RT_OBJ_TYPE_STATIC    = 0x80  //!< 静态的
};

/**
 * @brief 内核对象信息。
 */
struct rt_object_information {
    enum rt_object_type type; //!< 对象类型
    rt_dlist_t object_list;   //!< 内核对象的双向列表节点
    size_t object_size;       //!< 对象大小
};

/**
 * @brief 钩子函数宏。
 */
#ifndef RT_USING_HOOK
#define __ON_HOOK_ARGS(__hook, argv)
#define RT_OBJECT_HOOK_CALL(func, argv)
#else
#define RT_OBJECT_HOOK_CALL(func, argv) __on_##func argv
#ifdef RT_HOOK_USING_FUNC_PTR
#define __ON_HOOK_ARGS(__hook, argv)                                           \
    do {                                                                       \
        if ((__hook) != NULL) __hook argv;                                     \
    } while (0)
#else
#define __ON_HOOK_ARGS(__hook, argv)
#endif
#endif

/**
 * @brief 定义钩子函数。
 */
#ifndef __on_rt_interrupt_switch_hook
#define __on_rt_interrupt_switch_hook()                                        \
    __ON_HOOK_ARGS(rt_interrupt_switch_hook, ())
#endif
#ifndef __on_rt_malloc_hook
#define __on_rt_malloc_hook(addr, size)                                        \
    __ON_HOOK_ARGS(rt_malloc_hook, (addr, size))
#endif
#ifndef __on_rt_free_hook
#define __on_rt_free_hook(rmem) __ON_HOOK_ARGS(rt_free_hook, (rmem))
#endif

/**
 * @brief 定义定时器选项。
 */
#define RT_TIMER_FLAG_DEACTIVATED     0x0 //!< 未激活
#define RT_TIMER_FLAG_ACTIVATED       0x1 //!< 已激活
#define RT_TIMER_FLAG_ONE_SHOT        0x0 //!< 单次触发
#define RT_TIMER_FLAG_PERIODIC        0x2 //!< 周期触发
#define RT_TIMER_FLAG_HARD_TIMER      0x0 //!< 硬件定时器
#define RT_TIMER_FLAG_SOFT_TIMER      0x4 //!< 软件定时器
#define RT_TIMER_CTRL_SET_TIME        0x0 //!< 设置控制指令
#define RT_TIMER_CTRL_GET_TIME        0x1 //!< 获取控制指令
#define RT_TIMER_CTRL_SET_ONESHOT     0x2 //!< 变更成单次触发
#define RT_TIMER_CTRL_SET_PERIODIC    0x3 //!< 变更成周期触发
#define RT_TIMER_CTRL_GET_STATE       0x4 //!< 获取当前是否激活状态
#define RT_TIMER_CTRL_GET_REMAIN_TIME 0x5 //!< 获取剩余悬挂时间

#ifndef RT_TIMER_SKIP_LIST_LEVEL
#define RT_TIMER_SKIP_LIST_LEVEL 1
#endif
#ifndef RT_TIMER_SKIP_LIST_MASK
#define RT_TIMER_SKIP_LIST_MASK 0x3 //!< 1 or 3
#endif

/**
 * @brief 定时器结构体。
 */
struct rt_timer {
    struct rt_object parent; //!< 从哪个内核对象继承而来
    rt_dlist_t row[RT_TIMER_SKIP_LIST_LEVEL];
    void (*timeout_func)(void *parameter); //!< 超时回调
    void *parameter;                       //!< 超时回调参数
    rt_tick_t init_tick;                   //!< 超时tick数阈值
    rt_tick_t timeout_tick;                //!< 已超时tick数
};
typedef struct rt_timer *rt_timer_t;

/**
 * @brief 导入信号并定义信号配置选项。
 */
#ifdef RT_USING_SIGNALS
#include <sys/signal.h>
typedef unsigned long rt_sigset_t;
typedef void (*rt_sighandler_t)(int signal);
typedef siginfo_t rt_siginfo_t;
#define RT_SIG_MAX 32
#endif

/**
 * @brief 定义线程状态。
 */
#define RT_THREAD_INIT      0x00 //!< Initialized status */
#define RT_THREAD_READY     0x01 //!< Ready status */
#define RT_THREAD_SUSPEND   0x02 //!< Suspend status */
#define RT_THREAD_RUNNING   0x03 //!< Running status */
#define RT_THREAD_CLOSE     0x04 //!< Closed status */
#define RT_THREAD_STAT_MASK 0x07
#define RT_THREAD_STAT_YIELD                                                   \
    0x08 //!< indicate whether remaining_tick has been reloaded since last
         //!< schedule
#define RT_THREAD_STAT_YIELD_MASK   RT_THREAD_STAT_YIELD
#define RT_THREAD_STAT_SIGNAL       0x10 //!< task hold signals */
#define RT_THREAD_STAT_SIGNAL_READY (RT_THREAD_STAT_SIGNAL | RT_THREAD_READY)
#define RT_THREAD_STAT_SIGNAL_WAIT  0x20 //!< task is waiting for signals */
#define RT_THREAD_STAT_SIGNAL_PENDING                                          \
    0x40 //!< signals is held and it has not been procressed */
#define RT_THREAD_STAT_SIGNAL_MASK 0xf0

/**
 * thread control command definitions
 */
#define RT_THREAD_CTRL_STARTUP         0x00 //!< Startup thread. */
#define RT_THREAD_CTRL_CLOSE           0x01 //!< Close thread. */
#define RT_THREAD_CTRL_CHANGE_PRIORITY 0x02 //!< Change thread priority. */
#define RT_THREAD_CTRL_INFO            0x03 //!< Get thread information. */
#define RT_THREAD_CTRL_BIND_CPU        0x04 //!< Set thread bind cpu. */

#ifdef RT_USING_SMP
#define RT_CPU_DETACHED RT_CPUS_NR //!< The thread not running on cpu. */
#define RT_CPU_MASK     ((1 << RT_CPUS_NR) - 1) //!< All CPUs mask bit. */
#ifndef RT_SCHEDULE_IPI
#define RT_SCHEDULE_IPI 0
#endif
#ifndef RT_STOP_IPI
#define RT_STOP_IPI 1
#endif

/**
 * @brief 定义cpu数据结构。
 */
struct rt_cpu {
    struct rt_thread *current_thread;
    uint16_t irq_nest;
    uint8_t irq_switch_flag;
    uint8_t current_priority;
    rt_dlist_t priority_table[RT_THREAD_PRIORITY_MAX];
#if RT_THREAD_PRIORITY_MAX > 32
    uint32_t priority_group;
    uint8_t ready_table[32];
#else
    uint32_t priority_group;
#endif
    rt_tick_t tick;
};
#endif

/**
 * Thread structure
 */
struct rt_thread {
    char name[RT_NAME_MAX]; //!< the name of thread */
    uint8_t type;           //!< type of object */
    uint8_t flags;          //!< thread's flags */
#ifdef RT_USING_MODULE
    void *module_id; //!< id of application module */
#endif
    rt_dlist_t list;     //!< the object list */
    rt_dlist_t tlist;    //!< the thread list */
    void *sp;            //!< stack point */
    void *entry;         //!< entry */
    void *parameter;     //!< parameter */
    void *stack_addr;    //!< stack address */
    uint32_t stack_size; //!< stack size */
    rt_err_t error;      //!< error code */
    uint8_t stat;        //!< thread status */
#ifdef RT_USING_SMP
    uint8_t bind_cpu;             //!< thread is bind to cpu */
    uint8_t oncpu;                //!< process on cpu */
    uint16_t scheduler_lock_nest; //!< scheduler lock count */
    uint16_t cpus_lock_nest;      //!< cpus lock count */
    uint16_t critical_lock_nest;  //!< critical lock count */
#endif
    uint8_t current_priority; //!< current priority */
#if RT_THREAD_PRIORITY_MAX > 32
    uint8_t number;
    uint8_t high_mask;
#endif
    uint32_t number_mask;
#ifdef RT_USING_EVENT
    uint32_t event_set;
    uint8_t event_info;
#endif
#ifdef RT_USING_SIGNALS
    rt_sigset_t sig_pending; //!< the pending signals */
    rt_sigset_t sig_mask;    //!< the mask bits of signal */
#ifndef RT_USING_SMP
    void *sig_ret;                //!< the return stack pointer from signal */
#endif                            /* RT_USING_SMP */
    rt_sighandler_t *sig_vectors; //!< vectors of signal handler */
    void *si_list;                //!< the signal infor list */
#endif                            /* RT_USING_SIGNALS */
    rt_ubase_t init_tick;         //!< thread's initialized tick */
    rt_ubase_t remaining_tick;    //!< remaining tick */
#ifdef RT_USING_CPU_USAGE
    uint64_t duration_tick; //!< cpu usage tick */
#endif
#ifdef RT_USING_PTHREADS
    void *pthread_data; //!< the handle of pthread data, adapt 32/64bit */
#endif
    struct rt_timer thread_timer; //!< built-in thread timer */
    void (*cleanup)(
        struct rt_thread *tid); //!< cleanup function when thread exit */
#ifdef RT_USING_LWP
    void *lwp;
#endif
    rt_ubase_t user_data; //!< private user data beyond this thread */
};
typedef struct rt_thread *rt_thread_t;

/**
 * IPC flags and control command definitions
 */
#define RT_IPC_FLAG_FIFO   0x00 //!< FIFOed IPC. @ref IPC. */
#define RT_IPC_FLAG_PRIO   0x01 //!< PRIOed IPC. @ref IPC. */
#define RT_IPC_CMD_UNKNOWN 0x00 //!< unknown IPC command */
#define RT_IPC_CMD_RESET   0x01 //!< reset IPC object */
#define RT_WAITING_FOREVER -1   //!< Block forever until get resource. */
#define RT_WAITING_NO      0    //!< Non-block. */

/**
 * Base structure of IPC object
 */
struct rt_ipc_object {
    struct rt_object parent;   //!< inherit from rt_object */
    rt_dlist_t suspend_thread; //!< threads pended on this resource */
};

#ifdef RT_USING_SEMAPHORE
/**
 * Semaphore structure
 */
struct rt_semaphore {
    struct rt_ipc_object parent; //!< inherit from ipc_object */
    uint16_t value;              //!< value of semaphore. */
    uint16_t reserved;           //!< reserved field */
};
typedef struct rt_semaphore *rt_sem_t;
#endif

#ifdef RT_USING_MUTEX
/**
 * Mutual exclusion (mutex) structure
 */
struct rt_mutex {
    struct rt_ipc_object parent; //!< inherit from ipc_object */
    uint16_t value;              //!< value of mutex */
    uint8_t original_priority;   //!< priority of last thread hold the mutex */
    uint8_t hold;                //!< numbers of thread hold the mutex */
    struct rt_thread *owner;     //!< current owner of mutex */
};
typedef struct rt_mutex *rt_mutex_t;
#endif

#ifdef RT_USING_EVENT
/**
 * flag definitions in event
 */
#define RT_EVENT_FLAG_AND   0x01 //!< logic and */
#define RT_EVENT_FLAG_OR    0x02 //!< logic or */
#define RT_EVENT_FLAG_CLEAR 0x04 //!< clear flag */

/*
 * event structure
 */
struct rt_event {
    struct rt_ipc_object parent; //!< inherit from ipc_object */
    uint32_t set;                //!< event set */
};
typedef struct rt_event *rt_event_t;
#endif

#ifdef RT_USING_MAILBOX
/**
 * mailbox structure
 */
struct rt_mailbox {
    struct rt_ipc_object parent; //!< inherit from ipc_object */
    rt_ubase_t *msg_pool;        //!< start address of message buffer */
    uint16_t size;               //!< size of message pool */
    uint16_t entry;              //!< index of messages in msg_pool */
    uint16_t in_offset;          //!< input offset of the message buffer */
    uint16_t out_offset;         //!< output offset of the message buffer */
    rt_dlist_t
        suspend_sender_thread; //!< sender thread suspended on this mailbox */
};
typedef struct rt_mailbox *rt_mailbox_t;
#endif

#ifdef RT_USING_MESSAGEQUEUE
/**
 * message queue structure
 */
struct rt_messagequeue {
    struct rt_ipc_object parent; //!< inherit from ipc_object */
    void *msg_pool;              //!< start address of message queue */
    uint16_t msg_size;           //!< message size of each message */
    uint16_t max_msgs;           //!< max number of messages */
    uint16_t entry;              //!< index of messages in the queue */
    void *msg_queue_head;        //!< list head */
    void *msg_queue_tail;        //!< list tail */
    void *msg_queue_free; //!< pointer indicated the free node of queue */
    rt_dlist_t suspend_sender_thread; //!< sender thread suspended on this
                                      //!< message queue
};
typedef struct rt_messagequeue *rt_mq_t;
#endif

#ifdef RT_USING_HEAP
/*
 * memory structure
 */
struct rt_memory {
    struct rt_object parent; //!< inherit from rt_object */
    const char *algorithm;   //!< Memory management algorithm name */
    rt_ubase_t address;      //!< memory start address */
    size_t total;            //!< memory size */
    size_t used;             //!< size used */
    size_t max;              //!< maximum usage */
};
typedef struct rt_memory *rt_mem_t;
#endif

/*
 * memory management
 * heap & partition
 */
#ifdef RT_USING_SMALL_MEM
typedef rt_mem_t rt_smem_t;
#endif

#ifdef RT_USING_SLAB
typedef rt_mem_t rt_slab_t;
#endif

#ifdef RT_USING_MEMHEAP
/**
 * memory item on the heap
 */
struct rt_memheap_item {
    uint32_t magic;                    //!< magic number for memheap */
    struct rt_memheap *pool_ptr;       //!< point of pool */
    struct rt_memheap_item *next;      //!< next memheap item */
    struct rt_memheap_item *prev;      //!< prev memheap item */
    struct rt_memheap_item *next_free; //!< next free memheap item */
    struct rt_memheap_item *prev_free; //!< prev free memheap item */
#ifdef RT_USING_MEMTRACE
    uint8_t owner_thread_name[4]; //!< owner thread name */
#endif
};

/**
 * Base structure of memory heap object
 */
struct rt_memheap {
    struct rt_object parent;            //!< inherit from rt_object */
    void *start_addr;                   //!< pool start address and size */
    size_t pool_size;                   //!< pool size */
    size_t available_size;              //!< available size */
    size_t max_used_size;               //!< maximum allocated size */
    struct rt_memheap_item *block_list; //!< used block list */
    struct rt_memheap_item *free_list;  //!< free block list */
    struct rt_memheap_item free_header; //!< free block list header */
    struct rt_semaphore lock;           //!< semaphore lock */
    bool locked;                        //!< External lock mark */
};
#endif

#ifdef RT_USING_MEMPOOL
/**
 * Base structure of Memory pool object
 */
struct rt_mempool {
    struct rt_object parent;   //!< inherit from rt_object */
    void *start_address;       //!< memory pool start */
    size_t size;               //!< size of memory pool */
    size_t block_size;         //!< size of memory blocks */
    uint8_t *block_list;       //!< memory blocks list */
    size_t block_total_count;  //!< numbers of memory block */
    size_t block_free_count;   //!< numbers of free memory block */
    rt_dlist_t suspend_thread; //!< threads pended on this resource */
};
typedef struct rt_mempool *rt_mp_t;
#endif

#ifdef RT_USING_DEVICE
/**
 * @brief 定义IO设备类型。
 */
enum rt_device_type {
    RT_DEVICE_TYPE_CHAR = 0,   //!< character device */
    RT_DEVICE_TYPE_BLOCK,      //!< block device */
    RT_DEVICE_TYPE_NET,        //!< net interface */
    RT_DEVICE_TYPE_MTD,        //!< memory device */
    RT_DEVICE_TYPE_CAN,        //!< CAN device */
    RT_DEVICE_TYPE_RTC,        //!< RTC device */
    RT_DEVICE_TYPE_SOUND,      //!< Sound device */
    RT_DEVICE_TYPE_GRAPHIC,    //!< Graphic device */
    RT_DEVICE_TYPE_I2C,        //!< I2C bus device */
    RT_DEVICE_TYPE_USB_DEVICE, //!< USB slave device */
    RT_DEVICE_TYPE_USB_HOST,   //!< USB host bus */
    RT_DEVICE_TYPE_USB_OTG,    //!< USB OTG bus */
    RT_DEVICE_TYPE_SPI,        //!< SPI bus device */
    RT_DEVICE_TYPE_SPI_DEVICE, //!< SPI device */
    RT_DEVICE_TYPE_SDIO,       //!< SDIO bus device */
    RT_DEVICE_TYPE_PM,         //!< PM pseudo device */
    RT_DEVICE_TYPE_PIPE,       //!< Pipe device */
    RT_DEVICE_TYPE_PORTAL,     //!< Portal device */
    RT_DEVICE_TYPE_TIMER,      //!< Timer device */
    RT_DEVICE_TYPE_SUNDRY,     //!< Miscellaneous device */
    RT_DEVICE_TYPE_SENSOR,     //!< Sensor device */
    RT_DEVICE_TYPE_TOUCH,      //!< Touch device */
    RT_DEVICE_TYPE_PHY,        //!< PHY device */
    RT_DEVICE_TYPE_SECURITY,   //!< Security device */
    RT_DEVICE_TYPE_WLAN,       //!< WLAN device */
    RT_DEVICE_TYPE_PIN,        //!< Pin device */
    RT_DEVICE_TYPE_ADC,        //!< ADC device */
    RT_DEVICE_TYPE_DAC,        //!< DAC device */
    RT_DEVICE_TYPE_WDT,        //!< WDT device */
    RT_DEVICE_TYPE_PWM,        //!< PWM device */
    RT_DEVICE_TYPE_UNKNOWN     //!< unknown device */
};

/**
 * @brief 定义设备状态类型。
 */
#define RT_DEVICE_FLAG_DEACTIVATE 0x000 //!< device is not not initialized */
#define RT_DEVICE_FLAG_RDONLY     0x001 //!< read only */
#define RT_DEVICE_FLAG_WRONLY     0x002 //!< write only */
#define RT_DEVICE_FLAG_RDWR       0x003 //!< read and write */
#define RT_DEVICE_FLAG_REMOVABLE  0x004 //!< removable device */
#define RT_DEVICE_FLAG_STANDALONE 0x008 //!< standalone device */
#define RT_DEVICE_FLAG_ACTIVATED  0x010 //!< device is activated */
#define RT_DEVICE_FLAG_SUSPENDED  0x020 //!< device is suspended */
#define RT_DEVICE_FLAG_STREAM     0x040 //!< stream mode */
#define RT_DEVICE_FLAG_INT_RX     0x100 //!< INT mode on Rx */
#define RT_DEVICE_FLAG_DMA_RX     0x200 //!< DMA mode on Rx */
#define RT_DEVICE_FLAG_INT_TX     0x400 //!< INT mode on Tx */
#define RT_DEVICE_FLAG_DMA_TX     0x800 //!< DMA mode on Tx */
#define RT_DEVICE_OFLAG_CLOSE     0x000 //!< device is closed */
#define RT_DEVICE_OFLAG_RDONLY    0x001 //!< read only access */
#define RT_DEVICE_OFLAG_WRONLY    0x002 //!< write only access */
#define RT_DEVICE_OFLAG_RDWR      0x003 //!< read and write */
#define RT_DEVICE_OFLAG_OPEN      0x008 //!< device is opened */
#define RT_DEVICE_OFLAG_MASK      0xf0f //!< mask of open flag */

/**
 * @brief 定义设备常用操作。
 */
#define RT_DEVICE_CTRL_RESUME  0x01 //!< resume device */
#define RT_DEVICE_CTRL_SUSPEND 0x02 //!< suspend device */
#define RT_DEVICE_CTRL_CONFIG  0x03 //!< configure device */
#define RT_DEVICE_CTRL_CLOSE   0x04 //!< close device */
#define RT_DEVICE_CTRL_SET_INT 0x10 //!< set interrupt */
#define RT_DEVICE_CTRL_CLR_INT 0x11 //!< clear interrupt */
#define RT_DEVICE_CTRL_GET_INT 0x12 //!< get interrupt status */

/**
 * @brief 特殊设备命令。
 */
#define RT_DEVICE_CTRL_BASE(Type) (RT_DEVICE_TYPE_##Type * 0x100)
#define RT_DEVICE_CTRL_CHAR_STREAM                                             \
    (RT_DEVICE_CTRL_BASE(Char) + 1) //!< stream mode on char device */
#define RT_DEVICE_CTRL_BLK_GETGEOME                                            \
    (RT_DEVICE_CTRL_BASE(Block) + 1) //!< get geometry information   */
#define RT_DEVICE_CTRL_BLK_SYNC                                                \
    (RT_DEVICE_CTRL_BASE(Block) + 2) //!< flush data to block device */
#define RT_DEVICE_CTRL_BLK_ERASE                                               \
    (RT_DEVICE_CTRL_BASE(Block) + 3) //!< erase block on block device */
#define RT_DEVICE_CTRL_BLK_AUTOREFRESH                                         \
    (RT_DEVICE_CTRL_BASE(Block) +                                              \
     4) //!< block device : enter/exit auto refresh mode */
#define RT_DEVICE_CTRL_NETIF_GETMAC                                            \
    (RT_DEVICE_CTRL_BASE(NetIf) + 1) //!< get mac address */
#define RT_DEVICE_CTRL_MTD_FORMAT                                              \
    (RT_DEVICE_CTRL_BASE(MTD) + 1) //!< format a MTD device */

/**
 * @brief 定义设备结构体。
 */
typedef struct rt_device *rt_device_t;
struct rt_device {
    struct rt_object parent;  //!< inherit from rt_object */
    enum rt_device_type type; //!< device type */
    uint16_t flag;            //!< device flag */
    uint16_t open_flag;       //!< device open flag */
    uint8_t ref_count;        //!< reference count */
    uint8_t device_id;        //!< 0 - 255 */
    rt_err_t (*rx_indicate)(rt_device_t dev, size_t size);
    rt_err_t (*tx_complete)(rt_device_t dev, void *buffer);
#ifdef RT_USING_DEVICE_OPS
    const struct rt_device_ops *ops;
#else
    /* common device interface */
    rt_err_t (*init)(rt_device_t dev);
    rt_err_t (*open)(rt_device_t dev, uint16_t oflag);
    rt_err_t (*close)(rt_device_t dev);
    size_t (*read)(rt_device_t dev, rt_off_t pos, void *buffer, size_t size);
    size_t (*write)(rt_device_t dev, rt_off_t pos, const void *buffer,
                    size_t size);
    rt_err_t (*control)(rt_device_t dev, int cmd, void *args);
#endif
#ifdef RT_USING_POSIX_DEVIO
    const struct dfs_file_ops *fops;
    struct rt_wqueue wait_queue;
#endif
    void *user_data; //!< device private data */
};

#ifdef RT_USING_DEVICE_OPS
/**
 * @brief 定义设备常用操作接口结构体。
 */
struct rt_device_ops {
    rt_err_t (*init)(rt_device_t dev);
    rt_err_t (*open)(rt_device_t dev, uint16_t oflag);
    rt_err_t (*close)(rt_device_t dev);
    size_t (*read)(rt_device_t dev, rt_off_t pos, void *buffer, size_t size);
    size_t (*write)(rt_device_t dev, rt_off_t pos, const void *buffer,
                    size_t size);
    rt_err_t (*control)(rt_device_t dev, int cmd, void *args);
};
#endif

/**
 * @brief 定义等待队列结构体。
 */
struct rt_wqueue {
    uint32_t flag;
    rt_dlist_t waiting_list;
};
typedef struct rt_wqueue rt_wqueue_t;

/**
 * @brief 定义块设备几何结构体。
 */
struct rt_device_blk_geometry {
    uint32_t sector_count;     //!< count of sectors */
    uint32_t bytes_per_sector; //!< number of bytes per sector */
    uint32_t block_size;       //!< number of bytes to erase one block */
};

/**
 * @brief 定义扇区在块设备上的结构安排。
 */
struct rt_device_blk_sectors {
    uint32_t sector_begin; //!< begin sector */
    uint32_t sector_end;   //!< end sector   */
};

/**
 * @brief 定义光标控制命令。
 */
#define RT_DEVICE_CTRL_CURSOR_SET_POSITION 0x10
#define RT_DEVICE_CTRL_CURSOR_SET_TYPE     0x11

/**
 * @brief 定义图形设备控制命令。
 */
#define RTGRAPHIC_CTRL_RECT_UPDATE    (RT_DEVICE_CTRL_BASE(Graphic) + 0)
#define RTGRAPHIC_CTRL_POWERON        (RT_DEVICE_CTRL_BASE(Graphic) + 1)
#define RTGRAPHIC_CTRL_POWEROFF       (RT_DEVICE_CTRL_BASE(Graphic) + 2)
#define RTGRAPHIC_CTRL_GET_INFO       (RT_DEVICE_CTRL_BASE(Graphic) + 3)
#define RTGRAPHIC_CTRL_SET_MODE       (RT_DEVICE_CTRL_BASE(Graphic) + 4)
#define RTGRAPHIC_CTRL_GET_EXT        (RT_DEVICE_CTRL_BASE(Graphic) + 5)
#define RTGRAPHIC_CTRL_SET_BRIGHTNESS (RT_DEVICE_CTRL_BASE(Graphic) + 6)
#define RTGRAPHIC_CTRL_GET_BRIGHTNESS (RT_DEVICE_CTRL_BASE(Graphic) + 7)
#define RTGRAPHIC_CTRL_GET_MODE       (RT_DEVICE_CTRL_BASE(Graphic) + 8)
#define RTGRAPHIC_CTRL_GET_STATUS     (RT_DEVICE_CTRL_BASE(Graphic) + 9)
#define RTGRAPHIC_CTRL_PAN_DISPLAY    (RT_DEVICE_CTRL_BASE(Graphic) + 10)
#define RTGRAPHIC_CTRL_WAIT_VSYNC     (RT_DEVICE_CTRL_BASE(Graphic) + 11)

/**
 * @brief 定义图形设备的像素格式。
 */
enum rt_graphic_pixel_format_t {
    RTGRAPHIC_PIXEL_FORMAT_MONO = 0,
    RTGRAPHIC_PIXEL_FORMAT_GRAY4,
    RTGRAPHIC_PIXEL_FORMAT_GRAY16,
    RTGRAPHIC_PIXEL_FORMAT_RGB332,
    RTGRAPHIC_PIXEL_FORMAT_RGB444,
    RTGRAPHIC_PIXEL_FORMAT_RGB565,
    RTGRAPHIC_PIXEL_FORMAT_RGB565P,
    RTGRAPHIC_PIXEL_FORMAT_BGR565 = RTGRAPHIC_PIXEL_FORMAT_RGB565P,
    RTGRAPHIC_PIXEL_FORMAT_RGB666,
    RTGRAPHIC_PIXEL_FORMAT_RGB888,
    RTGRAPHIC_PIXEL_FORMAT_BGR888,
    RTGRAPHIC_PIXEL_FORMAT_ARGB888,
    RTGRAPHIC_PIXEL_FORMAT_ABGR888,
    RTGRAPHIC_PIXEL_FORMAT_RESERVED,
};

/**
 * @brief 定义如何根据(x, y)坐标构建像素位置。
 */
#define RTGRAPHIC_PIXEL_POSITION(x, y) ((x << 16) | y)

/**
 * @brief 定义图形设备信息结构体。
 */
struct rt_device_graphic_info {
    uint8_t pixel_format;   //!< graphic format */
    uint8_t bits_per_pixel; //!< bits per pixel */
    uint16_t pitch;         //!< bytes per line */
    uint16_t width;         //!< width of graphic device */
    uint16_t height;        //!< height of graphic device */
    uint8_t *framebuffer;   //!< frame buffer */
    uint32_t smem_len;      //!< allocated frame buffer size */
};

/**
 * @brief 定义矩形信息结构体。
 */
struct rt_device_rect_info {
    uint16_t x;      //!< x coordinate */
    uint16_t y;      //!< y coordinate */
    uint16_t width;  //!< width */
    uint16_t height; //!< height */
};

/**
 * @brief 定义图形设备操作接口结构体。
 */
struct rt_device_graphic_ops {
    void (*set_pixel)(const char *pixel, int x, int y);
    void (*get_pixel)(char *pixel, int x, int y);
    void (*draw_hline)(const char *pixel, int x1, int x2, int y);
    void (*draw_vline)(const char *pixel, int x, int y1, int y2);
    void (*blit_line)(const char *pixel, int x, int y, size_t size);
};

#define RT_GRAPHIC_OPS(device)                                                 \
    ((struct rt_device_graphic_ops *)(device->user_data))

#endif

#ifdef RT_USING_MODULE
struct rt_module_symtab {
    void *addr;
    const char *name;
};
#define RTM_EXPORT(symbol)                                                     \
    const char __rtmsym_##symbol##_name[] SECTION(".rodata.name") = #symbol;   \
    const struct rt_module_symtab __rtmsym_##symbol SECTION(                   \
        ".rt_module_symbol") = {(void *)&symbol, __rtmsym_##symbol##_name};
#else
#define RTM_EXPORT(symbol)
#endif

#endif