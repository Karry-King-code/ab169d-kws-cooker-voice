#include "include.h"

///0x100 - hid命令数据之和(u8类型),作为hid命令的一个校验位
uint8_t checksum_calculation(uint8_t *ptr, uint16_t start, uint16_t len)
{
    uint16_t i ;
    uint8_t sum = 0;

    for(i = start; i < (start + len); i++){
        sum += ptr[i];
    }
    return (uint8_t)(0x100 - sum);
}

#if HUART_EN

#define TRACE_EN                0
#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

isr_t register_isr(int vector, isr_t isr);

// SHID_CMD_VARS sHCVars AT(.huart_buf);//huart msg
S_HSUART_BUF sHsUartbuf AT(.huart_buf);//TX/RX
SHID_DECODE_PACKET sHidDecodePacket AT(.huart_buf);//

AT(.com_text.huart)
void u_huart_rx_set_dir(void)
{
#if   (HUART_RX_MAP == HSRX_PA5)
    GPIOADIR |= BIT(5);
#elif (HUART_RX_MAP == HSRX_PA6)
    GPIOADIR |= BIT(6);
#elif (HUART_RX_MAP == HSRX_PA2)
    GPIOADIR |= BIT(2);
#elif (HUART_RX_MAP == HSRX_PA10)
    GPIOADIR |= BIT(10);
#elif (HUART_RX_MAP == HSRX_PA11)
    GPIOADIR |= BIT(11);
#endif
}

AT(.text.huart)
void u_huart_tx_set_dir(void)
{
#if   (HUART_TX_MAP == HSTX_PA5)
    GPIOADIR &= ~BIT(5);
#elif (HUART_TX_MAP == HSTX_PA6)
    GPIOADIR &= ~BIT(6);
#elif (HUART_TX_MAP == HSTX_PA2)
    GPIOADIR &= ~BIT(2);
#elif (HUART_TX_MAP == HSTX_PA10)
    GPIOADIR &= ~BIT(10);
#elif (HUART_TX_MAP == HSTX_PA11)
    GPIOADIR &= ~BIT(11);
#endif
}

AT(.text.bsp.uart)
void bsp_huart_init(void)
{
#if (HUART_TX_MAP || HUART_RX_MAP)
    u8 tx_map, rx_map;
#endif
    // memset(&sHCVars, 0, sizeof(sHCVars));
    memset(&sHsUartbuf, 0, sizeof(sHsUartbuf));
    memset(&sHidDecodePacket, 0, sizeof(sHidDecodePacket));
//tx
#if   (HUART_TX_MAP == HSTX_PA5)
    tx_map = 1;
    GPIOADE  |=  BIT(5);
    GPIOASET |=  BIT(5);
    GPIOADIR &= ~BIT(5);
    GPIOAFEN |=  BIT(5);
#elif (HUART_TX_MAP == HSTX_PA6)
    tx_map = 2;
    GPIOADE  |=  BIT(6);
    GPIOASET |=  BIT(6);
    GPIOADIR &= ~BIT(6);
    GPIOAFEN |=  BIT(6);
#elif (HUART_TX_MAP == HSTX_PA2)
    tx_map = 3;
    GPIOADE  |=  BIT(2);
    GPIOASET |=  BIT(2);
    GPIOADIR &= ~BIT(2);
    GPIOAFEN |=  BIT(2);
#elif (HUART_TX_MAP == HSTX_PA10)
    tx_map = 4;
    GPIOADE  |=  BIT(10);
    GPIOASET |=  BIT(10);
    GPIOADIR &= ~BIT(10);
    GPIOAFEN |=  BIT(10);
#elif (HUART_TX_MAP == HSTX_PA11)
    tx_map = 5;
    GPIOADE  |=  BIT(11);
    GPIOASET |=  BIT(11);
    GPIOADIR &= ~BIT(11);
    GPIOAFEN |=  BIT(11);
#endif

//rx
#if   (HUART_RX_MAP == HSRX_PA5)
    rx_map = 1;
    GPIOAPU  |= BIT(5);
    GPIOADE  |= BIT(5);
    GPIOASET |= BIT(5);
    GPIOADIR |= BIT(5);
    GPIOAFEN |= BIT(5);
#elif (HUART_RX_MAP == HSRX_PA6)
    rx_map = 2;
    GPIOAPU  |= BIT(6);
    GPIOADE  |= BIT(6);
    GPIOASET |= BIT(6);
    GPIOADIR |= BIT(6);
    GPIOAFEN |= BIT(6);
#elif (HUART_RX_MAP == HSRX_PA2)
    rx_map = 3;
    GPIOAPU  |= BIT(2);
    GPIOADE  |= BIT(2);
    GPIOASET |= BIT(2);
    GPIOADIR |= BIT(2);
    GPIOAFEN |= BIT(2);
#elif (HUART_RX_MAP == HSRX_PA10)
    rx_map = 4;
    GPIOAPU  |= BIT(10);
    GPIOADE  |= BIT(10);
    GPIOASET |= BIT(10);
    GPIOADIR |= BIT(10);
    GPIOAFEN |= BIT(10);
#elif (HUART_RX_MAP == HSRX_PA11)
    rx_map = 5;
    GPIOAPU  |= BIT(11);
    GPIOADE  |= BIT(11);
    GPIOASET |= BIT(11);
    GPIOADIR |= BIT(11);
    GPIOAFEN |= BIT(11);
#endif
    FUNCMCON0 = 0x0f << 20 | 0x0f << 16;

#if HUART_CROSSBAR
    //如使用其他IO,需要手动修改
    GPIOADE  |=  BIT(7);
    GPIOASET |=  BIT(7);
    GPIOADIR |=  BIT(7);
    GPIOAPU  |=  BIT(7);
    GPIOAFEN |=  BIT(7);

    FUNCMCON0 = ((FUNCMCON0 & ~(0x7 << 20))  | (6 << 20));
    FUNCINCON = ((FUNCINCON & ~(0x3f << 24)) | ((HUART_CROSSBAR_IO - 1) << 24)); //channel3 select input  io

    FUNCOUTMCON = ((FUNCOUTMCON & ~(0x1f << 0)) | (HUART_CROSSBAR_IO << 0));     //channel0 select output io
    FUNCOUTCON  = ((FUNCOUTCON & ~(0xf << 0)) | FUNCO_HSUTTX);                   //function select hsuart_tx
    bsp_config_huart(sHsUartbuf.buf, UART_RX_BUF_LEN, 1);                        //only support online mode
#else
    if(tx_map == rx_map){
        FUNCMCON0 = (0x7 << 20) | HUART_TX_MAP;
        bsp_config_huart(sHsUartbuf.buf, UART_RX_BUF_LEN, 1);
    }else{
        FUNCMCON0 = HUART_RX_MAP | HUART_TX_MAP;
        bsp_config_huart(sHsUartbuf.buf, UART_RX_BUF_LEN, 0);
    }
#endif
#if HUART_DMA_EN && (!HUART_PRINTF_EN)
    bsp_huart_rx_kick();    /* printf模式(2026-08-31)下RX路径被裁剪(bsp_huart.c:!HUART_PRINTF_EN区), 此调用须同步裁剪, 否则链接报undefined */
#endif
}

AT(.text.bsp.uart)
void bsp_config_huart(u8 *rx_buf, uint rx_len, u8 trx_oline)
{
    CLKGAT0 &= ~BIT(11);
    asm("nop");
    asm("nop");
    CLKGAT0 |= BIT(11);
    CLKCON0 &= ~(BIT(17)|BIT(18)|BIT(19));                  //HUART 24M CLK

    bsp_huart_irq_init();
    HSUT0BAUD = (HUART_BAUD_VAL << 16) | HUART_BAUD_VAL;

    HSUT0CON = BIT(9);                              //2 bit stop bit
    if(trx_oline){
        HSUT0CON |= BIT(18);
    }
    HSUT0CON |= BIT(2);                             //RX IE enable

#if HUART_DMA_EN
    HSUT0CON |= BIT(7) | BIT(4);                    //TX/RX DMA mode
#endif
    HSUT0CON |= BIT(1) | BIT(0);                    //TX/RX EN

    HSUT0TMRCNT = 32;                               //32个时钟没收完，则结束
    HSUT0CON |= BIT(10);                            //Enable TMRCNT
    HSUT0CPND = 0xffff;
}

AT(.text.bsp.uart)
void bsp_huart_irq_init(void)
{
    #if (!HUART_PRINTF_EN)
    register_isr(IRQ_HUART_VECTOR, bsp_huart_isr);
    PICPR &= ~BIT(IRQ_HUART_VECTOR);
	PICEN |= BIT(IRQ_HUART_VECTOR);
    #endif
}

#if (!HUART_PRINTF_EN)
// AT(.com_text.batstr)
// const char batstrhuart[] = "%d \n";
AT(.com_text.huart.isr)
void bsp_huart_isr(void)
{
//     my_printf(batstrhuart, 1);
    if ((HSUT0CON & BIT(2)) && (HSUT0CON & (BIT(12) | BIT(15)))) {         //RX Pending or TimeOUT
        if ((HSUT0CON & BIT(3)) == 0) {                                    //没有发送数据，直接继续接收
            #if HUART_RX_TEST_EN
            // my_printf(batstrhuart, 2);
            my_print_r(sHsUartbuf.buf, HSUT0FIFOCNT);
            #endif
            char rx_form[] = "%d \n";
            my_printf(rx_form, HSUT0FIFOCNT);
            bsp_huart_save_nbyte(sHsUartbuf.buf, HSUT0FIFOCNT);           //保存每次接收到的数据，支持不定长
            bsp_huart_rx_kick();
        }
    }
    if ((HSUT0CON & BIT(3)) && (HSUT0CON & BIT(13))) {  //TX Done
        // my_printf(batstrhuart, 3);
        HSUT0CPND = BIT(13);
        HSUT0CON &= ~BIT(3);                            //TX IE Dis
        HSUT0CON |= BIT(2);                             //RX IE En

        u_huart_rx_set_dir();
        bsp_huart_rx_kick();
    }
}

AT(.com_text.bsp.huart)
void bsp_huart_rx_kick(void)
{
    // my_printf(batstrhuart, 1);
    HSUT0CPND = BIT(0);                             //Clear RX to idle
    HSUT0CPND = BIT(12);                            //Clear RX Pending
    HSUT0CPND = BIT(15);
    HSUT0RXADR = DMA_ADR(sHsUartbuf.buf);
    HSUT0RXCNT = HUART_RX_BUF_LEN;             //buf len--rx max len
}
#endif

//***************************************************************************************//
#if 0
///HUART 获取RXFIFO中的数据个数
AT(.com_text.bsp.huart)
uint bsp_huart_get_rxcnt(void)
{
    return HSUT0FIFOCNT;
}

///HUART RXFIFO清空
AT(.com_text.bsp.huart)
void bsp_huart_rxfifo_clear(void)
{
    // HSUT0CPND = BIT(11);
    HSUT0CPND = BIT(12);
}
#endif

//huart获取1字节数据接口
AT(.text.huart)
u8 bsp_huart_getchar(u8 *charData)
{
    if(sHsUartbuf.r_cnt != sHsUartbuf.w_cnt)
    {
//        char rx_form[] = "%d \n";
//        my_printf(rx_form, sHsUartbuf.r_cnt);
        *charData = sHsUartbuf.decodeBuf[sHsUartbuf.r_cnt];
        if((sHsUartbuf.r_cnt + 1) >= HUART_RX_LOOP_BUF_LEN){
            sHsUartbuf.r_cnt = 0;
        }else{
            sHsUartbuf.r_cnt++;
        }
        return 1;
    }
    return 0;
}

#if (!HUART_PRINTF_EN)
//huart存放n字节
AT(.com_text.uart)
void bsp_huart_save_nbyte(u8 *inbuf, u8 length)
{
    u32 tempCnt = 0;
    while(length--)
    {
        sHsUartbuf.decodeBuf[sHsUartbuf.w_cnt] = inbuf[tempCnt];
//        char test_format[] = "decodeBuf[%d]: 0x%x\n";
//         my_printf(test_format,sHsUartbuf.w_cnt, sHsUartbuf.decodeBuf[sHsUartbuf.w_cnt]);
        if((sHsUartbuf.w_cnt + 1) >= HUART_RX_LOOP_BUF_LEN){
            sHsUartbuf.w_cnt = 0;
        }else{
            sHsUartbuf.w_cnt++;
        }
        tempCnt++;
    }
}
#endif

#if 0
//huart获取字节数据并存放接口
AT(.com_text.uart)
void bsp_huart_getchar_data2_save(void)
{
    u32 huartCnt = bsp_huart_get_rxcnt();
    // my_printf("huartCnt: %d\n", huartCnt);
    u32 tempCnt = 0;

    if(huartCnt != 0){
        my_printf("huartCnt: %d\n", huartCnt);
        my_printf("sHsUartbuf.buf[]:\n");
        my_print_r(sHsUartbuf.buf, 10);
    }
    while(huartCnt--)
    {
        sHsUartbuf.decodeBuf[sHsUartbuf.w_cnt] = sHsUartbuf.buf[tempCnt];
        // my_printf("decodeBuf[%d]: 0x%x\n",sHsUartbuf.w_cnt, sHsUartbuf.decodeBuf[sHsUartbuf.w_cnt]);
        if((sHsUartbuf.w_cnt + 1) >= HUART_RX_LOOP_BUF_LEN){
            sHsUartbuf.w_cnt = 0;
        }else{
            sHsUartbuf.w_cnt++;
        }
        tempCnt++;
    }
    bsp_huart_rxfifo_clear();
}
#endif

///HUART 发送1Byte数据
#if (!HUART_PRINTF_EN)
AT(.text.uart)
void bsp_huart_putchar_byte(char ch)
{
    HSUT0DATA = ch;
    while (!(HSUT0CON & BIT(13)));
}
#else
AT(.com_text.huart)
void bsp_huart_putchar_byte(char ch)
{
    HSUT0DATA = ch;
    while (!(HSUT0CON & BIT(13)));
}
#endif

//huart发送数据接口
AT(.text.huart)
void bsp_huart_putchar_data(void *buf, uint len)
{
    HSUT0CON &= ~BIT(0);        //DIS RX_EN
    u_huart_tx_set_dir();

    if ((u32)buf >= 0x10000000) {
        const char *txbuf = buf;
        while (len--) {
            bsp_huart_putchar_byte(*txbuf++);
        }
    } else {
        // while (!(HSUT0CON & BIT(13)));
        HSUT0CON |= BIT(7);                 //DMA Mode
        HSUT0TXADR = DMA_ADR(buf);
        HSUT0TXCNT = len;
    }

    while (!(HSUT0CON & BIT(13)));
    u_huart_rx_set_dir();
    HSUT0CON |= BIT(0);

    // HSUT0RXADR = DMA_ADR(sHsUartbuf.buf);
    // HSUT0RXCNT = UART_RX_BUF_LEN;
    // house_huart_rx_kick();
}

//***************************************************************************************//
AT(.text.bsp.huart)
void bsp_huart_clear_decodeBuf(void)
{
    memset(&sHsUartbuf, 0, sizeof(sHsUartbuf));
}

#if 0
AT(.text.bsp.huart)
void bsp_huart_hid_msg_out(eHidCmd eMsg, u32 uData)
{
    SHID_CMD_MSG *psOutputMsg;

    psOutputMsg = &sHCVars.sOutputMsg[sHCVars.MsgWr];
    psOutputMsg->eMsg = eMsg;
    psOutputMsg->uData = uData;

    sHCVars.MsgWr++;

    if(sHCVars.MsgWr >= BSP_HID_MSG_MAX_NUM)
    {
        sHCVars.MsgWr = 0;
    }
    return;
}
AT(.text.bsp.huart)
SHID_CMD_MSG *bsp_huart_hid_msg_read()
{
    SHID_CMD_MSG *psOutputMsg = NULL;

    if(sHCVars.MsgRd != sHCVars.MsgWr)
    {
        psOutputMsg = &sHCVars.sOutputMsg[sHCVars.MsgRd++];

        if(sHCVars.MsgRd >= BSP_HID_MSG_MAX_NUM)
        {
            sHCVars.MsgRd = 0;
        }
    }
    return psOutputMsg;
}
#endif


AT(.text.huart)
bool parse_huart_hid_cmd_data(u8 uartData)
{
    bool boDecodeSuccess = false;
    if(CMD_HEADER_ONE == uartData){
        sHidDecodePacket.getHeader1 = true;
        sHidDecodePacket.decodeStep = FIND_HEADER;
    }else{
        switch(sHidDecodePacket.decodeStep)
        {
            case FIND_HEADER:
            {
                TRACE("FIND_HEADER\n");
                if(sHidDecodePacket.getHeader1){
                    if(CMD_HEADER_TWO == uartData){
                        sHidDecodePacket.decodeStep = CONFIRM_ID;
                    }else{
                        sHidDecodePacket.getHeader1 = false;
                        sHidDecodePacket.decodeStep = SAVE_HID;
                        sHidDecodePacket.hidDataBuf[sHidDecodePacket.w_cnt++] = CMD_HEADER_ONE;//55 is hid data
                        sHidDecodePacket.hidDataBuf[sHidDecodePacket.w_cnt++] = uartData;
                    }
                }else{
                    TRACE("header error\n");
                    memset(&sHidDecodePacket, 0, sizeof(sHidDecodePacket));
                }
            }break;
            case CONFIRM_ID:
            {
                if(CMD_ID == uartData){
                    sHidDecodePacket.getIdentification = true;
                    sHidDecodePacket.decodeStep = GET_LENGTH;
                }else{
                    TRACE("identification error\n");
                    memset(&sHidDecodePacket, 0, sizeof(sHidDecodePacket));
                }
            }break;
            case GET_LENGTH:
            {
                TRACE("GET_LENGTH uartData: 0x%X\n", uartData);
                if(uartData <= HID_CMD_MAX_SIZE){
                    sHidDecodePacket.hidLength = uartData;
                    sHidDecodePacket.decodeStep = SAVE_HID;
                }else{
                    TRACE("cmd length oversize\n");
                    memset(&sHidDecodePacket, 0, sizeof(sHidDecodePacket));
                }
            }break;
            case SAVE_HID:
            {
                sHidDecodePacket.hidDataBuf[sHidDecodePacket.w_cnt & 0x3f] = uartData;
                sHidDecodePacket.w_cnt++;
                if(sHidDecodePacket.w_cnt >= sHidDecodePacket.hidLength){
                    TRACE("SAVE_HID uartData: 0x%X\n", uartData);
                    sHidDecodePacket.decodeStep = CAL_CHECKSUM;
                }else{
                    TRACE("SAVE_HID uartData: 0x%X\n", uartData);
                }
            }break;
            case CAL_CHECKSUM:
            {
                TRACE("CAL_CHECKSUM\n");
                sHidDecodePacket.checkSum = uartData;
                u8 tempCheckSum = checksum_calculation(sHidDecodePacket.hidDataBuf, 0, sHidDecodePacket.hidLength);
                if(tempCheckSum == sHidDecodePacket.checkSum){
                    TRACE("checksum right\n");
                    sHidDecodePacket.decodeStep = CMD_SEND;
                }else{
                    TRACE("checksum error\n");
                    memset(&sHidDecodePacket, 0, sizeof(sHidDecodePacket));
                }
            }break;
            case CMD_SEND:
            {
                TRACE("cmd step error\n");
                memset(&sHidDecodePacket, 0, sizeof(sHidDecodePacket));
            }break;
            default:
                break;
        }
    }
    if(CMD_SEND == sHidDecodePacket.decodeStep){
        TRACE("CMD_SEND\n");
        boDecodeSuccess = true;
    }
    if(boDecodeSuccess)
    {
        memset(&sHidDecodePacket, 0, sizeof(sHidDecodePacket));
    }
    return boDecodeSuccess;
}
//***************************************************************************************//

#if HUART_TX_TEST_EN
static u32 huart_tx_tick = 0;
AT(.rodata.huart_cmd)
static const u8 huart_tx_cmd[] = {
    0x55,0x12,  //data---
    0x34,0x56,  //data---
    0x78,0xFF,  //data---
    0x88,0x44,  //data---
    0x22,0x11,  //data---
};
//huart发送测试
AT(.text.bsp.huart)
void huart_send_test(void)
{
    u8 huart_tx_buf[10];
    memcpy(huart_tx_buf, huart_tx_cmd, 10);
    bsp_huart_putchar_data(huart_tx_buf, 10);
}
#endif

//huart收到的数据处理
AT(.text.huart)
void huart_deal_process(void)
{
    u8 hidCharData;
    while(bsp_huart_getchar(&hidCharData)){
        WDT_CLR();
        parse_huart_hid_cmd_data(hidCharData);
    }
    #if HUART_TX_TEST_EN
        if(tick_check_expire(huart_tx_tick, 100)){
            huart_tx_tick = tick_get();
            huart_send_test();
        }
    #endif
}

#endif
