#ifndef __SECTION_H__
#define __SECTION_H__

#define FAST   __attribute__((section(".fast"), noinline))
#define RETAIN __attribute__((section(".retain"), noinline))

#endif // __SECTION_H__