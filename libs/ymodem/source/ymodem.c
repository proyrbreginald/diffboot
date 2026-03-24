#include <algo/algo.h>
#include <main.h>
#include <rthw.h>
#include <rtthread.h>
#include <stdlib.h>
#include <string.h>
#include <ymodem.h>

// 配置调试日志
#define DBG_TAG __FILE_NAME__
#define DBG_LVL DBG_VERBOSE
#include <rtdebug.h>

/* 控制字符 */
#define SOH (0x01)   /* 128字节数据包头 */
#define STX (0x02)   /* 1024字节数据包头 */
#define EOT (0x04)   /* 结束传输 */
#define ACK (0x06)   /* 响应 */
#define NAK (0x15)   /* 重传 */
#define CAN (0x18)   /* 取消 */
#define CRC_C (0x43) /* 'C' 字符 */

#define UART_RX_BUF_SIZE (1024 * 2)
#define YMODEM_RB_SIZE (1024 * 4)

SHARE_UNINIT static uint8_t uart_rx_buf[UART_RX_BUF_SIZE];
static uint32_t last_read_end_pos = 0;
static struct rt_semaphore uart_rx_sem;
static struct rt_ringbuffer ymodem_rb;
static uint8_t rb_mem[YMODEM_RB_SIZE];
static ymodem_ops_t *ymodem_cb = NULL;

void ymodem_set_ops(ymodem_ops_t *const ops)
{
    ymodem_cb = ops;
}

/**
 * @brief 串口底层发送。
 * @param ch 发送的字符。
 */
static void ymodem_putchar(const uint8_t ch)
{
    while (!LL_USART_IsActiveFlag_TXE_TXFNF(UART4))
        ;
    LL_USART_TransmitData8(UART4, ch);
}

/**
 * @brief 搬运dma数据到环形缓冲区。
 */
static void ymodem_dma_process(void)
{
    uint32_t data_len = LL_DMA_GetDataLength(DMA1, LL_DMA_STREAM_0);
    uint32_t curr_pos = UART_RX_BUF_SIZE - data_len;

    if (curr_pos != last_read_end_pos)
    {
        if (curr_pos > last_read_end_pos)
        {
            rt_ringbuffer_put(&ymodem_rb, &uart_rx_buf[last_read_end_pos],
                              curr_pos - last_read_end_pos);
        }
        else
        {
            rt_ringbuffer_put(&ymodem_rb, &uart_rx_buf[last_read_end_pos],
                              UART_RX_BUF_SIZE - last_read_end_pos);
            if (curr_pos > 0)
                rt_ringbuffer_put(&ymodem_rb, &uart_rx_buf[0], curr_pos);
        }
        last_read_end_pos = curr_pos;
    }
}

/**
 * @brief 阻塞式读取。
 * @param dest 数据去向。
 * @param len 读取指定长度数据。
 * @param timeout_ms 超时时间。
 * @return size_t
 */
static size_t rb_read_wait(uint8_t *dest, size_t len, uint32_t timeout_ms)
{
    rt_tick_t start = rt_tick_get();
    size_t total = 0;
    while (total < len)
    {
        total += rt_ringbuffer_get(&ymodem_rb, dest + total, len - total);
        if (total >= len)
            break;

        rt_sem_take(&uart_rx_sem, rt_tick_from_millisecond(20));
        ymodem_dma_process();

        if ((rt_tick_get() - start) > rt_tick_from_millisecond(timeout_ms))
            break;
    }
    return total;
}

/**
 * @brief ymodem协议主循环。
 */
void ymodem_receive_loop(void)
{
    uint8_t *pkt = rt_malloc(1024 + 12);
    if (!pkt)
        return;

    uint8_t head;
    int error_occurred = 0;
    LOG_I("wait for sender");

    // 会话层循环
    while (1)
    {
        rt_ringbuffer_reset(&ymodem_rb);
        last_read_end_pos =
            UART_RX_BUF_SIZE - LL_DMA_GetDataLength(DMA1, LL_DMA_STREAM_0);

        // 请求起始包(文件名包)
        ymodem_putchar(CRC_C);

        if (rb_read_wait(&head, 1, 3000) != 1)
            continue;

        if (head == SOH || head == STX)
        {
            // 读取文件名包
            uint32_t d_len = (head == SOH) ? 128 : 1024;
            if (rb_read_wait(&pkt[1], d_len + 4 - 1, 1000) != (d_len + 3))
                continue;

            char f_name[64] = {0};
            strncpy(f_name, (char *)&pkt[3], 63);

            // 收到空文件名包，会话真正结束
            if (strlen(f_name) == 0)
            {
                ymodem_putchar(ACK);
                LOG_I("all transfers completed");
                break;
            }

            uint32_t f_size = atoi((char *)&pkt[3] + strlen(f_name) + 1);
            LOG_I("receiving file: %s, size: %u", f_name, f_size);

            // 开始接收
            if (ymodem_cb && ymodem_cb->on_begin)
            {
                if (ymodem_cb->on_begin(f_name, f_size) != 0)
                {
                    // 拒绝接收(例如磁盘空间不足)
                    ymodem_putchar(CAN);
                    ymodem_putchar(CAN);
                    error_occurred = -1;
                    goto exit_session;
                }
            }

            ymodem_putchar(ACK);
            ymodem_putchar(CRC_C); // 准备接收正式数据

            // 进入数据接收循环
            uint32_t curr = 0;
            uint8_t seq = 1;

            while (1)
            {
                if (rb_read_wait(&head, 1, 5000) != 1)
                {
                    error_occurred = -2;
                    break;
                }

                // 只有收到EOT才说明数据传完了
                if (head == EOT)
                {
                    LOG_D("first EOT received");
                    ymodem_putchar(NAK);
                    if (rb_read_wait(&head, 1, 2000) == 1 && head == EOT)
                    {
                        LOG_D("second EOT received");
                        ymodem_putchar(ACK);
                        LOG_I("file %s received", f_name);
                    }
                    break; // 跳出数据循环，回到会话循环去发'C'
                }

                if (head == SOH || head == STX)
                {
                    uint32_t block_len = (head == SOH) ? 128 : 1024;
                    if (rb_read_wait(&pkt[1], block_len + 4, 1000) ==
                        (block_len + 4))
                    {
                        // 校验SEQ
                        if (pkt[1] + pkt[2] == 0xFF)
                        {
                            // 校验CRC
                            uint16_t c_crc = algo_crc16(&pkt[3], block_len);
                            uint16_t r_crc =
                                (uint16_t)(pkt[3 + block_len] << 8) |
                                pkt[3 + block_len + 1];

                            if (c_crc == r_crc)
                            {
                                if (pkt[1] == seq)
                                {
                                    uint32_t write_sz =
                                        (f_size - curr < block_len)
                                            ? (f_size - curr)
                                            : block_len;

                                    // 处理数据块
                                    if (ymodem_cb && ymodem_cb->on_data)
                                    {
                                        if (ymodem_cb->on_data(
                                                &pkt[3], write_sz, curr) != 0)
                                        {
                                            ymodem_putchar(CAN);
                                            ymodem_putchar(CAN);
                                            error_occurred = -3;
                                            goto exit_session;
                                        }
                                    }

                                    curr += write_sz;
                                    seq++;
                                }
                                ymodem_putchar(ACK);
                                continue;
                            }
                        }
                    }
                    ymodem_putchar(NAK); // 校验失败
                }

                if (head == CAN)
                {
                    error_occurred = -4;
                    goto exit_session;
                }
            }
        }
        else if (head == CAN)
            break;
    }

exit_session:
    // 数据传输循环结束，传递可能的错误信息通知上层
    if (ymodem_cb && ymodem_cb->on_end)
        ymodem_cb->on_end(error_occurred);

    rt_free(pkt);
}

/**
 * @brief 初始化ymodem。
 */
void ymodem_init(void)
{
    rt_sem_init(&uart_rx_sem, "uart_rx_sem", 0, RT_IPC_FLAG_FIFO);
    rt_ringbuffer_init(&ymodem_rb, rb_mem, YMODEM_RB_SIZE);

    LL_DMA_ConfigAddresses(
        DMA1, LL_DMA_STREAM_0,
        LL_USART_DMA_GetRegAddr(UART4, LL_USART_DMA_REG_DATA_RECEIVE),
        (uint32_t)uart_rx_buf, LL_DMA_DIRECTION_PERIPH_TO_MEMORY);
    LL_DMA_SetDataLength(DMA1, LL_DMA_STREAM_0, UART_RX_BUF_SIZE);
    LL_DMA_EnableIT_HT(DMA1, LL_DMA_STREAM_0);
    LL_DMA_EnableIT_TC(DMA1, LL_DMA_STREAM_0);
    LL_USART_EnableIT_IDLE(UART4);
    LL_DMA_EnableStream(DMA1, LL_DMA_STREAM_0);
    LL_USART_EnableDMAReq_RX(UART4);
}

/**
 * @brief uart4中断处理函数。
 */
void UART4_IRQHandler(void)
{
    rt_interrupt_enter();
    if (LL_USART_IsActiveFlag_IDLE(UART4))
    {
        LL_USART_ClearFlag_IDLE(UART4);
        rt_sem_release(&uart_rx_sem);
    }
    rt_interrupt_leave();
}

/**
 * @brief dma1中断处理函数。
 */
void DMA1_Stream0_IRQHandler(void)
{
    rt_interrupt_enter();
    if (LL_DMA_IsActiveFlag_HT0(DMA1))
    {
        LL_DMA_ClearFlag_HT0(DMA1);
    }
    if (LL_DMA_IsActiveFlag_TC0(DMA1))
    {
        LL_DMA_ClearFlag_TC0(DMA1);
    }
    rt_sem_release(&uart_rx_sem);
    rt_interrupt_leave();
}