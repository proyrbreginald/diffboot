#ifndef _RTCONFIG_H_
#define _RTCONFIG_H_

#define RT_USING_CPU_FFS                   // 使用快速位运算
#define RT_USING_LIBC                      // 使用libc
#define RT_KSERVICE_USING_STDLIB           // 使用标准库
#define RT_KSERVICE_USING_STDLIB_MEMORY    // 使用标准库内存操作接口
#define RT_ALIGN_SIZE                   4  // 内存对齐
#define RT_NAME_MAX                     16 // object字符串名称长度

#define RT_TICK_PER_SECOND              1000 // systick每秒触发频率
#define RT_THREAD_PRIORITY_MAX          8    // 优先级数量

#define RT_USING_COMPONENTS_INIT            // 使用组件形式自动初始化
#define RT_USING_SIGNALS                    // 使用异步信号
#define RT_USING_HOOK                       // 使用钩子
#define RT_USING_IDLE_HOOK                  // 使用空闲下线程钩子
#define RT_IDLE_HOOK_LIST_SIZE          1   // 空闲线程的钩子数量
#define IDLE_THREAD_STACK_SIZE          512 // 定义空闲钩子栈大小

#define RT_USING_MEMPOOL                // 使用内存池
#define RT_USING_HEAP                   // 使用堆内存
#define RT_USING_SMALL_MEM              // 开启小内存算法
#define RT_USING_SMALL_MEM_AS_HEAP      // 用小内存算法实现堆
// #define RT_USING_SLAB                   // 开启SLAB算法
// #define RT_USING_SLAB_AS_HEAP           // 用SLAB算法实现堆
// #define RT_USING_MEMHEAP                // 使用不连续内存堆

#define RT_USING_SEMAPHORE         // 使用信号量
#define RT_USING_MUTEX             // 使用互斥锁
#define RT_USING_EVENT             // 使用事件集
#define RT_USING_MAILBOX           // 使用邮箱
#define RT_USING_MESSAGEQUEUE      // 使用消息队列

// #define RT_USING_TIMER_SOFT            // 使用软件定时器
#define RT_TIMER_THREAD_PRIO       0   // 软件定时器线程优先级
#define RT_TIMER_THREAD_STACK_SIZE 512 // 软件定时器线程栈大小

#define RT_DEBUG                     // 开启调试日志
#define RT_DEBUG_COLOR               // 开启颜色日志
#define RT_DEBUG_IRQ               0 // 中断日志
#define RT_DEBUG_MEM               0 // 内存日志
#define RT_DEBUG_MEMHEAP           0 // 堆内存日志
#define RT_DEBUG_SLAB              0 // SLAB使用日志
#define RT_DEBUG_DEVICE            0 // 设备日志
#define RT_DEBUG_SCHEDULER         0 // 调度器日志
#define RT_DEBUG_CONTEXT_CHECK     1 // 上下文日志
#define RT_DEBUG_MODULE            0 // 模块日志
#define RT_DEBUG_THREAD            0 // 线程日志
#define RT_DEBUG_IPC               0 // 线程间通信日志
#define RT_DEBUG_TIMER             0 // 定时器日志
#define RT_USING_OVERFLOW_CHECK      // 栈溢出检查

#define RT_USING_DEVICE            // 使用设备驱动框架
#define RT_USING_DEVICE_OPS        // 使用设备驱动标准接口

// #define RT_USING_FINSH
// #define FINSH_USING_SYMTAB
// #define FINSH_USING_DESCRIPTION
// #define RT_CONSOLEBUF_SIZE      256
// #define FINSH_THREAD_PRIORITY   (RT_THREAD_PRIORITY_MAX - 2)
// #define FINSH_THREAD_STACK_SIZE 2048
// #define FINSH_CMD_SIZE          64
// #define FINSH_USING_HISTORY
// #define FINSH_HISTORY_LINES 8

#include <printf.h>

#endif