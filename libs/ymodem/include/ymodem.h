#ifndef _YMODEM_H_
#define _YMODEM_H_

#include <rtthread.h>

#define YMODEM_PROMISE_SOH 0x01 // 128字节数据包头
#define YMODEM_PROMISE_STX 0x02 // 1024字节数据包头
#define YMODEM_PROMISE_EOT 0x04 // 结束传输
#define YMODEM_PROMISE_ACK 0x06 // 确认
#define YMODEM_PROMISE_NAK 0x15 // 重发
#define YMODEM_PROMISE_CAN 0x18 // 取消
#define YMODEM_PROMISE_C   0x43 // 字符'C'

typedef struct ymodem_ctx ymodem_ctx_t;
typedef struct ymodem_ops ymodem_ops_t;

struct ymodem_ops {
    rt_err_t (*on_begin)(ymodem_ctx_t *ctx, const char *file_name,
                         rt_size_t file_size);
    rt_err_t (*on_data)(ymodem_ctx_t *ctx, rt_uint8_t *data, rt_size_t len);
    void (*on_end)(ymodem_ctx_t *ctx, rt_err_t reason);
};

struct ymodem_ctx {
    ymodem_ops_t ops;              // 应用层回调
    rt_uint8_t *buffer;            // 数据缓冲区
    rt_uint32_t file_byte_size;    // 文件总字节数
    rt_uint32_t receive_byte_size; // 已接收字节数
    rt_uint8_t packet_id;          // 当前正在传输的包序号
    void *user_data;               // 用户自定义指针
};

#endif