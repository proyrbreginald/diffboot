#ifndef _START_H_
#define _START_H_

typedef void (*void_fn_void_t)(void);

// 复位后加载user程序时需要校验的值
// user: 0x75 0x73 0x65 0x72 -> 0x72657375
#define LOAD_USER_CHECKSUM 0x72657375

// 复位后加载oem程序时需要校验的值
// oem: 0x6f 0x65 0x6d 0x00 -> 0x6f656d00
#define LOAD_OEM_CHECKSUM 0x6f656d00

#endif