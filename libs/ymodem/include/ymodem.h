#ifndef _YMODEM_H_
#define _YMODEM_H_

/**
 * @brief YModem过程回调接口。
 */
typedef struct {
    /**
     * @brief 收到文件名和大小，准备开始接收。
     * @param filename 文件名。
     * @param size 文件大小。
     * @return 0: 准备就绪，可以接收; 非0: 拒绝接收 (例如磁盘空间不足)。
     */
    int (*on_begin)(const char *filename, uint32_t size);

    /**
     * @brief 收到有效数据块。
     * @param data 数据指针。
     * @param len 数据长度。
     * @param curr_pos 当前文件偏移量。
     * @return 0: 处理成功; 非0: 错误 (例如写入失败)，将终止传输。
     */
    int (*on_data)(const uint8_t *data, uint32_t len, uint32_t curr_pos);

    /**
     * @brief 传输结束。
     * @param status 0: 成功; 非0: 异常终止 (超时、校验失败、被取消等)。
     */
    void (*on_end)(int status);
} ymodem_ops_t;

// 注册函数声明
void ymodem_set_ops(ymodem_ops_t *const ops);

void ymodem_receive_loop(void);

void ymodem_init(void);

#endif