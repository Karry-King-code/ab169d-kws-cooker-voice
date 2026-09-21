#ifndef _BSP_HUART_H
#define _BSP_HUART_H

#define BSP_HID_MSG_MAX_NUM         16  //Msg max num
#define HID_CMD_MAX_SIZE            10  //decoder buf len
#define HUART_RX_BUF_LEN            5  //huart isr buf
#define HUART_RX_LOOP_BUF_LEN       (HUART_RX_BUF_LEN*5)//huart rx loop buf

#define HUART_RX_KICK_NUM           HUART_RX_BUF_LEN  //huart触发中断的长度(使用DMA中断方式需要每次接收同样长度的包)

#define DECOED_PACKRT_EXPIRE_TICK   300

#define PLL_CLK             24000000
#define HUART_BAUD          4562
#define HUART_BAUD_VAL      (((PLL_CLK + (HUART_BAUD / 2)) / HUART_BAUD) - 1)

/*!*************************************************************************************************
\brief	HID CMD
****************************************************************************************************/
#define REPORT_ID_01    0x1
#define REPORT_ID_02    0x2
#define REPORT_ID_85    0x55
#define REPORT_ID_65    0x41

typedef enum
{
    H_HID_PLAYPAUSE,//01 04 00
    H_HID_VOLDOWN,//01 00 02
    H_HID_VOLUP,//01 00 01
}eHidCmd;

/*!*************************************************************************************************
\brief	Command Message
****************************************************************************************************/
typedef struct
{
    eHidCmd     eMsg;
    u32         uData;
}SHID_CMD_MSG;

/*!*************************************************************************************************
\brief	HID cmd msg VARS
****************************************************************************************************/
typedef struct
{
    SHID_CMD_MSG sOutputMsg[BSP_HID_MSG_MAX_NUM];
    u32     MsgRd;
    u32     MsgWr;
}SHID_CMD_VARS;

/*!*************************************************************************************************
\brief	command packet struct
****************************************************************************************************/
typedef enum
{
    CMD_HEADER_ONE = 0x55,
    CMD_HEADER_TWO = 0xAA,
    CMD_ID = 0xFE,
}eHidCmdHeader;
#if UART_AUDIO_EN
typedef enum
{
    IDEL_STEP,
    FIND_HEADER,
    CONFIRM_ID,
    GET_LENGTH,
    SAVE_DATA,
    CAL_CHECKSUM,
    CMD_SEND,
    SAVE_HID,
}eDecodeStep;
#else
typedef enum
{
    FIND_HEADER,
    CONFIRM_ID,
    GET_LENGTH,
    SAVE_HID,
    CAL_CHECKSUM,
    CMD_SEND,
}eDecodeStep;
#endif // UART_AUDIO_EN
typedef struct
{
    eDecodeStep decodeStep;

    bool getHeader1;//55
    bool getHeader2;//AA
    bool getIdentification;//FE
    u8 hidLength;
    volatile u8 w_cnt;
    volatile u8 r_cnt;
    u8 hidDataBuf[HID_CMD_MAX_SIZE + 2];//max is 64byte //+2避免越界
    u8 checkSum;//64byte checksum
}SHID_DECODE_PACKET;

/*!*************************************************************************************************
\brief	HS UART
****************************************************************************************************/
typedef struct
{
    bool dataChangeFlag;
    u8 buf[HUART_RX_BUF_LEN + 1];

    volatile u8 w_cnt;
    volatile u8 r_cnt;
    u8 decodeBuf[HUART_RX_LOOP_BUF_LEN];
} S_HSUART_BUF;

void u_huart_tx_set_dir(void);
void bsp_huart_init(void);
void bsp_config_huart(u8 *rx_buf, uint rx_len, u8 trx_oline);
void bsp_huart_irq_init(void);
void bsp_huart_isr(void);
void bsp_huart_rx_kick(void);

u8 bsp_huart_getchar(u8 *charData);
void bsp_huart_save_nbyte(u8 *inbuf, u8 length);
void bsp_huart_putchar_byte(char ch);
void bsp_huart_putchar_data(void *buf, uint len);
void huart_deal_process(void);

#endif
