#ifndef _ESBC_TRAN_H
#define _ESBC_TRAN_H
//***************************************************************************************//
#define     ESBC_CAL_TIME           4//((UART_AUDIO_ESBC * 16) / 2 / 160)
#define     UART_CMD_ESBC_SIZE      10
#define     UART_CMD_HEAD           5       //head + data len + mic num
#define     UART_KICK_NUM           (UART_AUDIO_ESBC + UART_CMD_HEAD)
typedef enum
{
    CMD_HEADER_ONE_UART0 = 0xEF,
    CMD_HEADER_TWO_UART0 = 0xFA,

    CMD_MTX_01_UART0  = 0x01,
    CMD_MTX_02_UART0  = 0x02,
    CMD_SYS_VOL_UART0 = 0x03,
    CMD_MIC_ALG_UART0 = 0x04,
    CMD_MIC_DIG_UART0 = 0x05,
    CMD_MIC_POST_GAIN_UART0 = 0x06,
    CMD_STX_08_UART0  = 0x08,
    CMD_STX_09_UART0  = 0x09,
}eHidCmdHeader_uart0;

typedef enum
{
    IDEL_STEP_UART0,
    FIND_HEADER_UART0,
    CONFIRM_ID_UART0,
    GET_LENGTH_UART0,
    SAVE_DATA_UART0,
    CAL_CHECKSUM_UART0,
    CMD_SEND_UART0,
}eDecodeStep_uart0;

typedef struct
{
    u8 u_set_sys_vol;//当前系统总音量
    u8 u_set_mic_anl;//当前MIC模拟增益
    u8 u_set_mic_dig;//当前MIC数字增益
    u8 u_set_mic_post_gain;//当前MIC后置数字增益
}SYS_VOL_VARS;

typedef struct
{
    eDecodeStep_uart0 decodeStep;

    bool getHeader1;//EF
    bool getHeader2;//FA
    u8 getIdentification;//cmd head
    u16 uLength;
    volatile u32 w_cnt;
    volatile u32 r_cnt;
    u8 DataBuf[UART_CMD_MAX_SIZE + 4];
    u8 checkSum;//64byte checksum
}UART_DECODE_PACKET;

typedef struct{
    u32     rx_err_cnt;     //解码错误累加
    bool    bo_uart_rst;

    u32     sys_run_time;
    u32     main_proc_tick;
    u32     send_cnt;       //发送计数/s
    volatile u32 recv_cnt;  //接收计数/s

    u32     wait_tick;
    bool    rx_data_kick;
}UART_VARS;

extern cbuf_t cbuf_uart2tx AT(.uart_buf.utx);
extern cbuf_t cbuf_uart2rx AT(.uart_buf.urx);
extern SYS_VOL_VARS        sSysVolVars AT(.uart_buf.ucmd);
extern UART_DECODE_PACKET  sDecodePacket AT(.uart_buf.ucmd);
extern UART_VARS           suart2_vars AT(.uart_buf.ucmd);

bool far_mic1_r_cirbuf_process(u8 *output, u16 length);
void uart_decode_process(void);

#endif // _ESBC_TRAN_H
//***************************************************************************************//
