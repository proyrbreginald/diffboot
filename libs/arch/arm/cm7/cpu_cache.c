/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-04-02     tanek        first implementation
 * 2019-04-27     misonyo      update to cortex-m7 series
 */

#include <board.h>
#include <rtdebug.h>
#include <rthw.h>

/* The L1-caches on all CortexÂ®-M7s are divided into lines of 32 bytes. */
#define L1CACHE_LINESIZE_BYTE (32)

void rt_hw_cpu_icache_enable(void)
{
    SCB_EnableICache();
}

void rt_hw_cpu_icache_disable(void)
{
    SCB_DisableICache();
}

rt_base_t rt_hw_cpu_icache_status(void)
{
    return 0;
}

void rt_hw_cpu_icache_ops(int ops, void *addr, int size)
{
    uint32_t address = (uint32_t)addr & (uint32_t)~(L1CACHE_LINESIZE_BYTE - 1);
    int32_t size_byte = size + address - (uint32_t)addr;
    uint32_t linesize = 32U;
    if (ops & RT_HW_CACHE_INVALIDATE)
    {
        __DSB();
        while (size_byte > 0)
        {
            SCB->ICIMVAU = address;
            address += linesize;
            size_byte -= linesize;
        }
        __DSB();
        __ISB();
    }
}

void rt_hw_cpu_dcache_enable(void)
{
    SCB_EnableDCache();
}

void rt_hw_cpu_dcache_disable(void)
{
    SCB_DisableDCache();
}

rt_base_t rt_hw_cpu_dcache_status(void)
{
    return 0;
}

void rt_hw_cpu_dcache_ops(int ops, void *addr, int size)
{
    uint32_t startAddr =
        (uint32_t)addr & (uint32_t)~(L1CACHE_LINESIZE_BYTE - 1);
    uint32_t size_byte = size + (uint32_t)addr - startAddr;
    uint32_t clean_invalid = RT_HW_CACHE_FLUSH | RT_HW_CACHE_INVALIDATE;

    if ((ops & clean_invalid) == clean_invalid)
    {
        SCB_CleanInvalidateDCache_by_Addr((uint32_t *)startAddr, size_byte);
    }
    else if (ops & RT_HW_CACHE_FLUSH)
    {
        SCB_CleanDCache_by_Addr((uint32_t *)startAddr, size_byte);
    }
    else if (ops & RT_HW_CACHE_INVALIDATE)
    {
        SCB_InvalidateDCache_by_Addr((uint32_t *)startAddr, size_byte);
    }
    else
    {
        RT_ASSERT(0);
    }
}
