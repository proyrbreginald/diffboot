/*
 * Copyright (c) 2006-2022, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2006-03-16     Bernard      the first version
 * 2006-05-25     Bernard      rewrite vsprintf
 * 2006-08-10     Bernard      add rt_show_version
 * 2010-03-17     Bernard      remove rt_strlcpy function
 *                             fix gcc compiling issue.
 * 2010-04-15     Bernard      remove weak definition on ICCM16C compiler
 * 2012-07-18     Arda         add the alignment display for signed integer
 * 2012-11-23     Bernard      fix IAR compiler error.
 * 2012-12-22     Bernard      fix rt_kprintf issue, which found by Grissiom.
 * 2013-06-24     Bernard      remove rt_kprintf if RT_USING_CONSOLE is not
 * defined. 2013-09-24     aozima       make sure the device is in STREAM mode
 * when used by rt_kprintf. 2015-07-06     Bernard      Add rt_assert_handler
 * routine. 2021-02-28     Meco Man     add RT_KSERVICE_USING_STDLIB 2021-12-20
 * Meco Man     implement rt_strcpy() 2022-01-07     Gabriel      add
 * __on_rt_assert_hook 2022-06-04     Meco Man     remove strnlen
 */

#include <printf.h>
#include <rtdebug.h>
#include <rthw.h>
#include <rtthread.h>
#include <string.h>

#ifdef RT_USING_MODULE
#include <dlmodule.h>
#endif /* RT_USING_MODULE */

/* use precision */
#define RT_PRINTF_PRECISION

/**
 * @addtogroup KernelService
 */

/**@{*/

/* global errno in RT-Thread */
static volatile int __rt_errno;

#if defined(RT_USING_DEVICE) && defined(RT_USING_CONSOLE)
static rt_device_t _console_device = NULL;
#endif

WEAK void rt_hw_us_delay(uint32_t us)
{
    (void)us;
    RT_DEBUG_LOG(
        RT_DEBUG_DEVICE,
        ("rt_hw_us_delay() doesn't support for this board."
         "Please consider implementing rt_hw_us_delay() in another file.\n"));
}

static const char *rt_errno_strs[] = {
    "OK",     "ERROR", "ETIMOUT", "ERSFULL", "ERSEPTY", "ENOMEM",
    "ENOSYS", "EBUSY", "EIO",     "EINTRPT", "EINVAL",  "EUNKNOW"};

/**
 * This function return a pointer to a string that contains the
 * message of error.
 * @param error the errorno code
 * @return a point to error message string
 */
const char *rt_strerror(rt_err_t error)
{
    if (error < 0)
        error = -error;

    return (error > RT_EINVAL + 1) ? rt_errno_strs[RT_EINVAL + 1]
                                   : rt_errno_strs[error];
}
RTM_EXPORT(rt_strerror);

/**
 * This function gets the global errno for the current thread.
 * @return errno
 */
rt_err_t rt_get_errno(void)
{
    rt_thread_t tid;

    if (rt_interrupt_get_nest() != 0)
    {
        /* it's in interrupt context */
        return __rt_errno;
    }

    tid = rt_thread_self();
    if (tid == NULL)
        return __rt_errno;

    return tid->error;
}
RTM_EXPORT(rt_get_errno);

/**
 * This function sets the global errno for the current thread.
 * @param error is the errno shall be set.
 */
void rt_set_errno(rt_err_t error)
{
    rt_thread_t tid;

    if (rt_interrupt_get_nest() != 0)
    {
        /* it's in interrupt context */
        __rt_errno = error;

        return;
    }

    tid = rt_thread_self();
    if (tid == NULL)
    {
        __rt_errno = error;

        return;
    }

    tid->error = error;
}
RTM_EXPORT(rt_set_errno);

/**
 * This function returns the address of the current thread errno.
 * @return The errno address.
 */
int *_rt_errno(void)
{
    rt_thread_t tid;

    if (rt_interrupt_get_nest() != 0)
        return (int *)&__rt_errno;

    tid = rt_thread_self();
    if (tid != NULL)
        return (int *)&(tid->error);

    return (int *)&__rt_errno;
}
RTM_EXPORT(_rt_errno);

/**
 * This function will show the version of rt-thread rtos
 */
void rt_show_version(void)
{
    rt_kprintf(" \\ | /  Thread Operating System of %s\n",
#if defined(BUILD_LOADER)
               "LOADER"
#elif defined(BUILD_USER)
               "USER"
#elif defined(BUILD_OEM)
               "OEM"
#else
/* 无有效参数时报错 */
#error "BUILD_LOADER, BUILD_USER or BUILD_OEM must be defined"
#endif
    );
    rt_kprintf("- R T - %d.%d.%d build %s %s\n", RT_VERSION, RT_SUBVERSION,
               RT_REVISION, __DATE__, __TIME__);
    rt_kprintf(" / | \\  2006-2022 Copyright by RT-Thread team\n");
}
RTM_EXPORT(rt_show_version);

/**
 * This function will duplicate a string.
 * @param  n is the string to be duplicated.
 * @param  base is support divide instructions value.
 * @return the duplicated string pointer.
 */
#ifdef RT_KPRINTF_USING_LONGLONG
INLINE static inline int divide(long long *n, int base)
#else
INLINE static inline int divide(long *n, int base)
#endif /* RT_KPRINTF_USING_LONGLONG */
{
    int res;

    /* optimized for processor which does not support divide instructions. */
    if (base == 10)
    {
#ifdef RT_KPRINTF_USING_LONGLONG
        res = (int)(((unsigned long long)*n) % 10U);
        *n = (long long)(((unsigned long long)*n) / 10U);
#else
        res = (int)(((unsigned long)*n) % 10U);
        *n = (long)(((unsigned long)*n) / 10U);
#endif
    }
    else
    {
#ifdef RT_KPRINTF_USING_LONGLONG
        res = (int)(((unsigned long long)*n) % 16U);
        *n = (long long)(((unsigned long long)*n) / 16U);
#else
        res = (int)(((unsigned long)*n) % 16U);
        *n = (long)(((unsigned long)*n) / 16U);
#endif
    }

    return res;
}

#ifdef RT_USING_CONSOLE

#ifdef RT_USING_DEVICE
/**
 * This function returns the device using in console.
 * @return Returns the console device pointer or NULL.
 */
rt_device_t rt_console_get_device(void)
{
    return _console_device;
}
RTM_EXPORT(rt_console_get_device);

/**
 * This function will set a device as console device.
 * After set a device to console, all output of rt_kprintf will be
 * redirected to this new device.
 * @param  name is the name of new console device.
 * @return the old console device handler on successful, or NULL on failure.
 */
rt_device_t rt_console_set_device(const char *name)
{
    rt_device_t new_device, old_device;

    /* save old device */
    old_device = _console_device;

    /* find new console device */
    new_device = rt_device_find(name);

    /* check whether it's a same device */
    if (new_device == old_device)
        return NULL;

    if (new_device != NULL)
    {
        if (_console_device != NULL)
        {
            /* close old console device */
            rt_device_close(_console_device);
        }

        /* set new console device */
        rt_device_open(new_device,
                       RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_STREAM);
        _console_device = new_device;
    }

    return old_device;
}
RTM_EXPORT(rt_console_set_device);
#endif /* RT_USING_DEVICE */

/* 日志消息结构 */
typedef struct rt_log_msg
{
    char data[ASYNC_LOG_BUF_SIZE];
} rt_log_msg_t;

static struct rt_messagequeue log_mq;
static uint8_t
    mq_pool[(sizeof(rt_log_msg_t) + sizeof(void *)) * ASYNC_LOG_MSG_COUNT];
static struct rt_thread log_thread;
static uint8_t log_stack[ASYNC_LOG_THREAD_STK];
static bool is_async_ready = false;

/**
 * 消费线程：负责把队列里的日志真正打印出来
 */
static void log_thread_entry(void *parameter)
{
    rt_log_msg_t msg;
    while (1)
    {
        if (rt_mq_recv(&log_mq, &msg, sizeof(rt_log_msg_t),
                       RT_WAITING_FOREVER) == RT_EOK)
        {
            rt_hw_console_output(msg.data);
        }
    }
}

/**
 * 初始化异步打印系统
 */
int log_thread_init(void)
{
    if (is_async_ready)
        return 0;

    rt_mq_init(&log_mq, "log_mq", &mq_pool[0], sizeof(rt_log_msg_t),
               sizeof(mq_pool), RT_IPC_FLAG_FIFO);

    rt_thread_init(&log_thread, "log", log_thread_entry, NULL, &log_stack[0],
                   sizeof(log_stack), ASYNC_LOG_THREAD_PRIO, 0);

    rt_thread_startup(&log_thread);
    is_async_ready = true;
    return 0;
}
RUN_APP_EXPORT(log_thread_init); // 自动初始化

/**
 * This function will print a formatted string on system console.
 * @param fmt is the format parameters.
 * @return The number of characters actually written to buffer.
 */
int rt_kprintf(const char *fmt, ...)
{
    va_list args;
    size_t length;

    /* 使用局部变量替代static，确保线程安全 */
    char local_buf[ASYNC_LOG_BUF_SIZE];

    va_start(args, fmt);
    length = vsnprintf(local_buf, sizeof(local_buf) - 1, fmt, args);
    if (length > ASYNC_LOG_BUF_SIZE - 1)
        length = ASYNC_LOG_BUF_SIZE - 1;
    local_buf[length] = '\0'; // 确保字符串结束
    va_end(args);

    /* 判断当前环境是否支持异步 */
    /* 如果OS已启动、不在中断中、且队列已初始化 */
    if (is_async_ready && rt_thread_self() != NULL &&
        rt_interrupt_get_nest() == 0)
    {
        rt_log_msg_t msg;
        memcpy(msg.data, local_buf, length + 1);

        // 发送给异步线程，如果不满则立即发送，满了则丢弃（避免阻塞业务线程）
        if (rt_mq_send(&log_mq, &msg, sizeof(rt_log_msg_t)) != RT_EOK)
        {
            // 可选：如果队列满了，为了不丢失重要日志，可以尝试同步输出
        }
    }
    else
    {
        /* 同步模式：直接输出 (用于中断、系统启动前、异常状态) */
        rt_hw_console_output(local_buf);
    }

    return length;
}
RTM_EXPORT(rt_kprintf);
#endif /* RT_USING_CONSOLE */

#if defined(RT_USING_HEAP) && !defined(RT_USING_USERHEAP)
#ifdef RT_USING_HOOK
static void (*rt_malloc_hook)(void *ptr, size_t size);
static void (*rt_free_hook)(void *ptr);

/**
 * @addtogroup Hook
 */

/**@{*/

/**
 * @brief This function will set a hook function, which will be invoked when a
 * memory block is allocated from heap memory.
 * @param hook the hook function.
 */
void rt_malloc_sethook(void (*hook)(void *ptr, size_t size))
{
    rt_malloc_hook = hook;
}

/**
 * @brief This function will set a hook function, which will be invoked when a
 * memory block is released to heap memory.
 * @param hook the hook function
 */
void rt_free_sethook(void (*hook)(void *ptr))
{
    rt_free_hook = hook;
}

/**@}*/

#endif /* RT_USING_HOOK */

#if defined(RT_USING_HEAP_ISR)
#elif defined(RT_USING_MUTEX)
static struct rt_mutex _lock;
#endif

INLINE static inline void _heap_lock_init(void)
{
#if defined(RT_USING_HEAP_ISR)
#elif defined(RT_USING_MUTEX)
    rt_mutex_init(&_lock, "heap", RT_IPC_FLAG_PRIO);
#endif
}

INLINE static inline rt_base_t _heap_lock(void)
{
#if defined(RT_USING_HEAP_ISR)
    return rt_hw_interrupt_disable();
#elif defined(RT_USING_MUTEX)
    if (rt_thread_self())
        return rt_mutex_take(&_lock, RT_WAITING_FOREVER);
    else
        return RT_EOK;
#else
    rt_enter_critical();
    return RT_EOK;
#endif
}

INLINE static inline void _heap_unlock(rt_base_t level)
{
#if defined(RT_USING_HEAP_ISR)
    rt_hw_interrupt_enable(level);
#elif defined(RT_USING_MUTEX)
    RT_ASSERT(level == RT_EOK);
    if (rt_thread_self())
        rt_mutex_release(&_lock);
#else
    rt_exit_critical();
#endif
}

#if defined(RT_USING_SMALL_MEM_AS_HEAP)
static rt_smem_t system_heap;
INLINE static inline void _smem_info(size_t *total, size_t *used,
                                     size_t *max_used)
{
    if (total)
        *total = system_heap->total;
    if (used)
        *used = system_heap->used;
    if (max_used)
        *max_used = system_heap->max;
}
#define _MEM_INIT(_name, _start, _size)                                        \
    system_heap = rt_smem_init(_name, _start, _size)
#define _MEM_MALLOC(_size) rt_smem_alloc(system_heap, _size)
#define _MEM_REALLOC(_ptr, _newsize)                                           \
    rt_smem_realloc(system_heap, _ptr, _newsize)
#define _MEM_FREE(_ptr) rt_smem_free(_ptr)
#define _MEM_INFO(_total, _used, _max) _smem_info(_total, _used, _max)
#elif defined(RT_USING_MEMHEAP_AS_HEAP)
static struct rt_memheap system_heap;
void *_memheap_alloc(struct rt_memheap *heap, size_t size);
void _memheap_free(void *rmem);
void *_memheap_realloc(struct rt_memheap *heap, void *rmem, size_t newsize);
#define _MEM_INIT(_name, _start, _size)                                        \
    rt_memheap_init(&system_heap, _name, _start, _size)
#define _MEM_MALLOC(_size) _memheap_alloc(&system_heap, _size)
#define _MEM_REALLOC(_ptr, _newsize)                                           \
    _memheap_realloc(&system_heap, _ptr, _newsize)
#define _MEM_FREE(_ptr) _memheap_free(_ptr)
#define _MEM_INFO(_total, _used, _max)                                         \
    rt_memheap_info(&system_heap, _total, _used, _max)
#elif defined(RT_USING_SLAB_AS_HEAP)
static rt_slab_t system_heap;
INLINE static inline void _slab_info(size_t *total, size_t *used,
                                     size_t *max_used)
{
    if (total)
        *total = system_heap->total;
    if (used)
        *used = system_heap->used;
    if (max_used)
        *max_used = system_heap->max;
}
#define _MEM_INIT(_name, _start, _size)                                        \
    system_heap = rt_slab_init(_name, _start, _size)
#define _MEM_MALLOC(_size) rt_slab_alloc(system_heap, _size)
#define _MEM_REALLOC(_ptr, _newsize)                                           \
    rt_slab_realloc(system_heap, _ptr, _newsize)
#define _MEM_FREE(_ptr) rt_slab_free(system_heap, _ptr)
#define _MEM_INFO _slab_info
#else
#define _MEM_INIT(...)
#define _MEM_MALLOC(...) NULL
#define _MEM_REALLOC(...) NULL
#define _MEM_FREE(...)
#define _MEM_INFO(...)
#endif

/**
 * @brief This function will init system heap.
 * @param begin_addr the beginning address of system page.
 * @param end_addr the end address of system page.
 */
WEAK void rt_system_heap_init(void *begin_addr, void *end_addr)
{
    rt_ubase_t begin_align = RT_ALIGN((rt_ubase_t)begin_addr, RT_ALIGN_SIZE);
    rt_ubase_t end_align = RT_ALIGN_DOWN((rt_ubase_t)end_addr, RT_ALIGN_SIZE);

    RT_ASSERT(end_align > begin_align);

    /* Initialize system memory heap */
    _MEM_INIT("heap", begin_addr, end_align - begin_align);
    /* Initialize multi thread contention lock */
    _heap_lock_init();
}

/**
 * @brief Allocate a block of memory with a minimum of 'size' bytes.
 * @param size is the minimum size of the requested block in bytes.
 * @return the pointer to allocated memory or NULL if no free memory was found.
 */
WEAK void *rt_malloc(size_t size)
{
    rt_base_t level;
    void *ptr;

    /* Enter critical zone */
    level = _heap_lock();
    /* allocate memory block from system heap */
    ptr = _MEM_MALLOC(size);
    /* Exit critical zone */
    _heap_unlock(level);
    /* call 'rt_malloc' hook */
    RT_OBJECT_HOOK_CALL(rt_malloc_hook, (ptr, size));
    return ptr;
}
RTM_EXPORT(rt_malloc);

/**
 * @brief This function will change the size of previously allocated memory
 * block.
 * @param rmem is the pointer to memory allocated by rt_malloc.
 * @param newsize is the required new size.
 * @return the changed memory block address.
 */
WEAK void *rt_realloc(void *rmem, size_t newsize)
{
    rt_base_t level;
    void *nptr;

    /* Enter critical zone */
    level = _heap_lock();
    /* Change the size of previously allocated memory block */
    nptr = _MEM_REALLOC(rmem, newsize);
    /* Exit critical zone */
    _heap_unlock(level);
    return nptr;
}
RTM_EXPORT(rt_realloc);

/**
 * @brief  This function will contiguously allocate enough space for count
 * objects that are size bytes of memory each and returns a pointer to the
 * allocated memory.
 *
 * @note   The allocated memory is filled with bytes of value zero.
 * @param  count is the number of objects to allocate.
 * @param  size is the size of one object to allocate.
 * @return pointer to allocated memory / NULL pointer if there is an error.
 */
WEAK void *rt_calloc(size_t count, size_t size)
{
    void *p;

    /* allocate 'count' objects of size 'size' */
    p = rt_malloc(count * size);
    /* zero the memory */
    if (p)
    {
        memset(p, 0, count * size);
    }
    return p;
}
RTM_EXPORT(rt_calloc);

/**
 * @brief This function will release the previously allocated memory block by
 *        rt_malloc. The released memory block is taken back to system heap.
 * @param rmem the address of memory which will be released.
 */
WEAK void rt_free(void *rmem)
{
    rt_base_t level;

    /* call 'rt_free' hook */
    RT_OBJECT_HOOK_CALL(rt_free_hook, (rmem));
    /* NULL check */
    if (rmem == NULL)
        return;
    /* Enter critical zone */
    level = _heap_lock();
    _MEM_FREE(rmem);
    /* Exit critical zone */
    _heap_unlock(level);
}
RTM_EXPORT(rt_free);

/**
 * @brief This function will caculate the total memory, the used memory, and
 *        the max used memory.
 * @param total is a pointer to get the total size of the memory.
 * @param used is a pointer to get the size of memory used.
 * @param max_used is a pointer to get the maximum memory used.
 */
WEAK void rt_memory_info(size_t *total, size_t *used, size_t *max_used)
{
    rt_base_t level;

    /* Enter critical zone */
    level = _heap_lock();
    _MEM_INFO(total, used, max_used);
    /* Exit critical zone */
    _heap_unlock(level);
}
RTM_EXPORT(rt_memory_info);

#if defined(RT_USING_SLAB) && defined(RT_USING_SLAB_AS_HEAP)
void *rt_page_alloc(size_t npages)
{
    rt_base_t level;
    void *ptr;

    /* Enter critical zone */
    level = _heap_lock();
    /* alloc page */
    ptr = rt_slab_page_alloc(system_heap, npages);
    /* Exit critical zone */
    _heap_unlock(level);
    return ptr;
}

void rt_page_free(void *addr, size_t npages)
{
    rt_base_t level;

    /* Enter critical zone */
    level = _heap_lock();
    /* free page */
    rt_slab_page_free(system_heap, addr, npages);
    /* Exit critical zone */
    _heap_unlock(level);
}
#endif

/**
 * This function allocates a memory block, which address is aligned to the
 * specified alignment size.
 * @param  size is the allocated memory block size.
 * @param  align is the alignment size.
 * @return The memory block address was returned successfully, otherwise it was
 *         returned empty NULL.
 */
WEAK void *rt_malloc_align(size_t size, size_t align)
{
    void *ptr;
    void *align_ptr;
    int uintptr_size;
    size_t align_size;

    /* sizeof pointer */
    uintptr_size = sizeof(void *);
    uintptr_size -= 1;

    /* align the alignment size to uintptr size byte */
    align = ((align + uintptr_size) & ~uintptr_size);

    /* get total aligned size */
    align_size = ((size + uintptr_size) & ~uintptr_size) + align;
    /* allocate memory block from heap */
    ptr = rt_malloc(align_size);
    if (ptr != NULL)
    {
        /* the allocated memory block is aligned */
        if (((rt_ubase_t)ptr & (align - 1)) == 0)
        {
            align_ptr = (void *)((rt_ubase_t)ptr + align);
        }
        else
        {
            align_ptr =
                (void *)(((rt_ubase_t)ptr + (align - 1)) & ~(align - 1));
        }

        /* set the pointer before alignment pointer to the real pointer */
        *((rt_ubase_t *)((rt_ubase_t)align_ptr - sizeof(void *))) =
            (rt_ubase_t)ptr;

        ptr = align_ptr;
    }

    return ptr;
}
RTM_EXPORT(rt_malloc_align);

/**
 * This function release the memory block, which is allocated by
 * rt_malloc_align function and address is aligned.
 * @param ptr is the memory block pointer.
 */
WEAK void rt_free_align(void *ptr)
{
    void *real_ptr;

    /* NULL check */
    if (ptr == NULL)
        return;
    real_ptr = (void *)*(rt_ubase_t *)((rt_ubase_t)ptr - sizeof(void *));
    rt_free(real_ptr);
}
RTM_EXPORT(rt_free_align);
#endif /* RT_USING_HEAP */

#ifndef RT_USING_CPU_FFS
#ifdef RT_USING_TINY_FFS
const uint8_t __lowest_bit_bitmap[] = {
    /*  0 - 7  */ 0,  1,  2,  27, 3,  24, 28, 32,
    /*  8 - 15 */ 4,  17, 25, 31, 29, 12, 32, 14,
    /* 16 - 23 */ 5,  8,  18, 32, 26, 23, 32, 16,
    /* 24 - 31 */ 30, 11, 13, 7,  32, 22, 15, 10,
    /* 32 - 36 */ 6,  21, 9,  20, 19};

/**
 * This function finds the first bit set (beginning with the least significant
 * bit) in value and return the index of that bit.
 *
 * Bits are numbered starting at 1 (the least significant bit).  A return value
 * of zero from any of these functions means that the argument was zero.
 * @return return the index of the first bit set. If value is 0, then this
 * function shall return 0.
 */
int __rt_ffs(int value)
{
    return __lowest_bit_bitmap[(uint32_t)(value & (value - 1) ^ value) % 37];
}
#else
const uint8_t __lowest_bit_bitmap[] = {
    /* 00 */ 0, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    /* 10 */ 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    /* 20 */ 5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    /* 30 */ 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    /* 40 */ 6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    /* 50 */ 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    /* 60 */ 5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    /* 70 */ 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    /* 80 */ 7, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    /* 90 */ 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    /* A0 */ 5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    /* B0 */ 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    /* C0 */ 6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    /* D0 */ 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    /* E0 */ 5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    /* F0 */ 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0};

/**
 * This function finds the first bit set (beginning with the least significant
 * bit) in value and return the index of that bit.
 *
 * Bits are numbered starting at 1 (the least significant bit).  A return value
 * of zero from any of these functions means that the argument was zero.
 * @return Return the index of the first bit set. If value is 0, then this
 * function shall return 0.
 */
int __rt_ffs(int value)
{
    if (value == 0)
        return 0;

    if (value & 0xff)
        return __lowest_bit_bitmap[value & 0xff] + 1;

    if (value & 0xff00)
        return __lowest_bit_bitmap[(value & 0xff00) >> 8] + 9;

    if (value & 0xff0000)
        return __lowest_bit_bitmap[(value & 0xff0000) >> 16] + 17;

    return __lowest_bit_bitmap[(value & 0xff000000) >> 24] + 25;
}
#endif /* RT_USING_TINY_FFS */
#endif /* RT_USING_CPU_FFS */

#ifndef __on_rt_assert_hook
#define __on_rt_assert_hook(ex, func, line)                                    \
    __ON_HOOK_ARGS(rt_assert_hook, (ex, func, line))
#endif

#ifdef RT_DEBUG
/* RT_ASSERT(EX)'s hook */

void (*rt_assert_hook)(const char *ex, const char *func, size_t line);

/**
 * This function will set a hook function to RT_ASSERT(EX). It will run when the
 * expression is false.
 * @param hook is the hook function.
 */
void rt_assert_set_hook(void (*hook)(const char *ex, const char *func,
                                     size_t line))
{
    rt_assert_hook = hook;
}

/**
 * The RT_ASSERT function.
 * @param ex_string is the assertion condition string.
 * @param func is the function name when assertion.
 * @param line is the file line number when assertion.
 */
void rt_assert_handler(const char *ex_string, const char *func, size_t line)
{
    volatile char dummy = 0;

    if (rt_assert_hook == NULL)
    {
#ifdef RT_USING_MODULE
        if (dlmodule_self())
        {
            /* close assertion module */
            dlmodule_exit(-1);
        }
        else
#endif /*RT_USING_MODULE*/
        {
            rt_kprintf(
                "(%s) assertion failed at function:%s, line number:%d \n",
                ex_string, func, line);
            while (dummy == 0)
                ;
        }
    }
    else
    {
        rt_assert_hook(ex_string, func, line);
    }
}
RTM_EXPORT(rt_assert_handler);
#endif /* RT_DEBUG */

/**@}*/
