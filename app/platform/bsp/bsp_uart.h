#ifndef _BSP_UART_H
#define _BSP_UART_H

#define SYS_CLK                     24000000

#define UART_CMD_MAX_SIZE           64
#define UART_RX_BUF_LEN             240 //128

#define UART_TXBUF_SIZE             340
#define UART_RXBUF_SIZE             960
/*!*************************************************************************************************
\brief	command packet struct
****************************************************************************************************/
typedef enum
{
    CMD_UHEAD_ONE = 0x55,
    CMD_UHEAD_TWO = 0xAA,
    CMD_UID = 0xFE,
}eUartHeader;

typedef enum
{
    FIND_UHEAD,
    CONFIRM_UID,
    GET_ULENGTH,
    SAVE_UDATA,
    CAL_UCHECKSUM,
    CMD_USEND,
}eUartStep;

typedef enum
{
    SYS_VOLUME_CTRL = 0x01,     //系统总音量
}eAdcDacCmd;

typedef struct
{
    u8 u_set_sys_vol;           //当前系统总音量

    eUartStep decodeStep;

    bool getHeader1;//55
    bool getHeader2;//AA
    bool getIdentification;//FE
    u8 uLength;
    volatile u8 w_cnt;
    volatile u8 r_cnt;
    u8 DataBuf[UART_CMD_MAX_SIZE + 2];//max is 64byte //+2避免越界
    u8 checkSum;//64byte checksum
}SUART_DECODE_PACKET;

/*!*************************************************************************************************
\brief	UART
****************************************************************************************************/
typedef struct
{
    u8 buf[UART_RX_BUF_LEN];
    volatile u32 rx_cnt;         //计数是否收齐一帧SPI_KICK_NUM
    volatile u8 w_cnt;
    volatile u8 r_cnt;
    u8 decodeBuf[UART_RX_BUF_LEN];
} S_UART_BUF;


void bsp_uart_init(void);
void udet_init(u32 baudrate);
void register_uart0_isr_init(isr_t isr);
void uart0_isr(void);
u8 bsp_uart0_get_data(u8 *charData);
void bsp_uart0_putchar(char charData);
bool bsp_send_uart0_data(u8 *buf, u16 length);
void bsp_uart_clear_decodeBuf(void);
bool parse_uart_cmd_data(u8 uartData);
void uart_deal_process(void);
void uart_main_process(void);
#endif
