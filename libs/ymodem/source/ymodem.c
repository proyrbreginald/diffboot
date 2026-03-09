#include <boot/section.h>
#include <main.h>
#include <ymodem.h>

// 配置调试日志
#define DBG_TAG __FILE_NAME__
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

#define UART_RX_BUF_SIZE (1024 * 2) // 定义uart接收缓冲区大小

// 定义结构体
SHARE static uint8_t uart_rx_buf[UART_RX_BUF_SIZE]; // uart接收缓冲区
static uint32_t last_read_end_pos = 0;              // 记录上次读取结束的位置
static struct rt_semaphore uart_rx_sem;             // 用于通知线程处理数据

/**
 * @brief 初始化结构体。
 *
 * @param mgr 结构体指针。
 */
static void ymodem_manager_init(void)
{
    // 初始化信号量
    rt_sem_init(&uart_rx_sem, "uart_rx_sem", 0, RT_IPC_FLAG_FIFO);

    // 配置DMA
    LL_DMA_ConfigAddresses(
        DMA1, LL_DMA_STREAM_0,
        LL_USART_DMA_GetRegAddr(UART4, LL_USART_DMA_REG_DATA_RECEIVE),
        (uint32_t)uart_rx_buf, LL_DMA_DIRECTION_PERIPH_TO_MEMORY);
    LL_DMA_SetDataLength(DMA1, LL_DMA_STREAM_0, UART_RX_BUF_SIZE);

    // 开启DMA中断(HT和TC)
    LL_DMA_EnableIT_HT(DMA1, LL_DMA_STREAM_0);
    LL_DMA_EnableIT_TC(DMA1, LL_DMA_STREAM_0);

    // 开启UART空闲中断
    LL_USART_EnableIT_IDLE(UART4);

    // 启动DMA和UART接收
    LL_DMA_EnableStream(DMA1, LL_DMA_STREAM_0);
    LL_USART_EnableDMAReq_RX(UART4);
}

/**
 * @brief uart4中断处理函数。
 *
 */
void UART4_IRQHandler(void)
{
    rt_interrupt_enter(); /* RT-Thread中断进入 */
    LOG_D("enter %s", __FUNCTION__);

    if (LL_USART_IsActiveFlag_IDLE(UART4))
    {
        LL_USART_ClearFlag_IDLE(UART4);
        rt_sem_release(&uart_rx_sem); // 通知线程一帧结束
    }

    LOG_D("exit %s", __FUNCTION__);
    rt_interrupt_leave(); /* RT-Thread中断退出 */
}

/**
 * @brief dma中断处理函数。
 *
 */
void DMA1_Stream0_IRQHandler(void)
{
    rt_interrupt_enter(); /* RT-Thread中断进入 */
    LOG_D("enter %s", __FUNCTION__);

    if (LL_DMA_IsActiveFlag_HT0(DMA1))
    {
        LL_DMA_ClearFlag_HT0(DMA1);
        rt_sem_release(&uart_rx_sem);
    }
    if (LL_DMA_IsActiveFlag_TC0(DMA1))
    {
        LL_DMA_ClearFlag_TC0(DMA1);
        rt_sem_release(&uart_rx_sem);
    }

    LOG_D("exit %s", __FUNCTION__);
    rt_interrupt_leave(); /* RT-Thread中断退出 */
}

/**
 * @brief 接收数据的处理回调。
 *
 * @param data 接收到的数据。
 * @param len 数据的字节长度。
 */
void ymodem_receive_handle(const uint8_t *data, uint32_t len)
{
    // 这里的data指向原始缓冲区，注意不要阻塞太久
    // 如果处理逻辑复杂，建议在此进行软件FIFO压入
    LOG_D("data: %.*s", len, data);
    rt_kprintf("%.*s", len, data);
}

#undef THREAD_NAME
#define THREAD_NAME "ymodem"
/**
 * @brief 文件传输线程。
 *
 * @param parameter 结构体指针。
 */
static void ymodem_thread_entry(void *parameter)
{
    while (1)
    {
        // 等待信号量，设置超时确保兜底（可选）
        if (rt_sem_take(&uart_rx_sem, RT_WAITING_FOREVER) == RT_EOK)
        {
            // 获取DMA当前待传输的剩余数据量
            // LL_DMA_GetDataLength返回的是还剩多少没存入内存
            uint32_t data_len = LL_DMA_GetDataLength(DMA1, LL_DMA_STREAM_0);
            uint32_t curr_pos = UART_RX_BUF_SIZE - data_len;
            LOG_D("receive data len: %u, current pos: %u, last pos: %u",
                  data_len, curr_pos, last_read_end_pos);

            if (curr_pos != last_read_end_pos)
            {
                if (curr_pos > last_read_end_pos)
                {
                    /* 情况A: 数据连续，没有回环 */
                    // 数据区间: [last_read_end_pos, curr_pos)
                    ymodem_receive_handle(&uart_rx_buf[last_read_end_pos],
                                          curr_pos - last_read_end_pos);
                }
                else
                {
                    /* 情况B: DMA已经翻转 (回环) */
                    // 第一部分: 从上次位置到Buf末尾
                    ymodem_receive_handle(&uart_rx_buf[last_read_end_pos],
                                          UART_RX_BUF_SIZE - last_read_end_pos);
                    // 第二部分: 从Buf开头到当前位置
                    if (curr_pos > 0)
                    {
                        ymodem_receive_handle(&uart_rx_buf[0], curr_pos);
                    }
                }
                last_read_end_pos = curr_pos;
            }
        }
    }
}

/**
 * @brief 启动文件传输线程。
 *
 * @return int 返回是否启动成功。
 */
static int ymodem_thread_init(void)
{
    ymodem_manager_init();

    rt_err_t result = RT_EOK;
    rt_thread_t tid =
        rt_thread_create(THREAD_NAME, ymodem_thread_entry, RT_NULL, 1024, 1, 0);
    if (tid != RT_NULL)
    {
        LOG_I(THREAD_NAME " thread create success");
        result = rt_thread_startup(tid);
        if (result == RT_EOK)
        {
            LOG_I(THREAD_NAME " thread startup success");
        }
        else
        {
            LOG_E(THREAD_NAME " thread startup fail with %d", result);
        }
    }
    else
    {
        result = RT_ENOMEM;
        LOG_E(THREAD_NAME " thread create fail");
    }
    return result;
}
INIT_APP_EXPORT(ymodem_thread_init);