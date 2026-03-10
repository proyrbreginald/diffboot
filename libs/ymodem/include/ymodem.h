#ifndef _YMODEM_H_
#define _YMODEM_H_

#include <rtthread.h>

/* 控制字符 */
#define SOH             (0x01) /* 128字节数据包头 */
#define STX             (0x02) /* 1024字节数据包头 */
#define EOT             (0x04) /* 结束传输 */
#define ACK             (0x06) /* 响应 */
#define NAK             (0x15) /* 重传 */
#define CAN             (0x18) /* 取消 */
#define CRC_C           (0x43) /* 'C' 字符 */

#endif