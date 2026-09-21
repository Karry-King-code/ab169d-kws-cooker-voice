//#include "bsp_huart.h"
#include "uart_com.h"
#include "typedef.h"
#include "config.h"

// 宏定义仅做查找用
#define HUART_ISR_BUF_LEN       HUART_RX_BUF_LEN   // 一次dma中断的长度

// 用此文件，要在sdk config.h中把 HUART_EN置1， 并且注释掉 huart_deal_process的调用
// 把 HSUT0RXCNT = HUART_RX_LOOP_BUF_LEN; 改为 HSUT0RXCNT = HUART_RX_BUF_LEN;

// 宏定义仅做查找用
#define HUART_TX_PIN        HUART_TX_MAP     // 发送引脚
#define HUART_RX_PIN        HUART_RX_MAP     // 接收引脚
#define HUART_BARDRATE      HUART_BAUD       // 波特率


void uart_com_init(int baud)
{
#if !HUART_EN
    my_printf("error! please set HUART_EN to 1 !\n");
#endif
}

int uart_com_write(uint8_t* data, int len)
{
#if HUART_EN
    bsp_huart_putchar_data(data, len);
#endif // HUART_EN
    return 0;
}

int uart_com_read(uint8_t* data, int len)
{
#if HUART_EN
    int read_bytes;
    for (read_bytes = 0; read_bytes < len; read_bytes++)
    {
        if (bsp_huart_getchar(data+read_bytes) == 0)
        {
            break;
        }
    }
    return read_bytes;
#else
    return 0;
#endif
}
