#ifndef __SECTION_H__
#define __SECTION_H__

// 将函数放到ITCM中运行
#define FAST __attribute__((section(".fast"), noinline))

// 将数据放到共享RAM中，允许几乎所有外设访问
#define SHARE __attribute__((section(".share")))

// 将数据放到备份RAM中，主要用于boot和app之间的切换
#define BACKUP __attribute__((section(".backup")))

#endif // __SECTION_H__