#include "include.h"

cbuf_t cbuf_spi1tx AT(.spi_buf);
AT(.com_text.cbufcsst)
const char spk_w [] = "buf_write1: end[%d], len[%d], wptr[%d]\n";

AT(.com_text.cir_read)
bool spk_read_by_cirbuf_process(u8 *output, u16 length)
{
    if(cbuf_uart2tx.data_len >= length){
        circular_buf_read(&cbuf_uart2tx, output, length);  //读cbuf数据
        return true;
    }else{
        #if EF_PRINTF
        printf(buf_e);
        #endif
        return false;
    }
}

AT(.com_text.cir_write)
bool spk_write_to_cirbuf_process(void* buf, u16 length)
{
    if(circular_buf_is_write_able(&cbuf_uart2tx, length)){
        //my_printf("write buf\n");
        circular_buf_write(&cbuf_uart2tx, (u8 *)buf, length);  //写mic数据
        return true;
    }else{
        #if EF_PRINTF
        printf(buf_f);
        #endif
        return false;
    }
}

#if SPI1_AUDIO_EN

#define TRACE_EN            1
#if     TRACE_EN
#define TRACE(...)          my_printf(__VA_ARGS__)
#define TRACE_R(...)        my_print_r(__VA_ARGS__)
#else
#define TRACE(...)
#define TRACE_R(...)
#endif

void spi1_irq_read(void);
void spi1_irq_tx(void);

void spi1_cs_en(void);
void spi1_cs_dis(void);
void spi1_cs_init(void);

u32 get_sysclk_nhz(void);
void obuf_put_samples_w(void *buf, uint len);//SPI1 中断入口函数

volatile bool flag_spk_data_kick = true;

#define SPI1_CS_INIT()          spi1_cs_init()
#define SPI1_CS_EN()            spi1_cs_en()
#define SPI1_CS_DIS()           spi1_cs_dis()

#define EDGE_BIT                            BIT(9)//PA9
#define EDGE_INT_PORT                       (IO_PA9 - 1)
#define EDGE_PIN_IRQ_DIS()                  {PORTINTEN &= ~BIT(EDGE_INT_PORT);}
#define EDGE_PIN_IRQ_EN()                   {PORTINTEN |=  BIT(EDGE_INT_PORT);}

uint8_t spi_audio_test_en = (SPI_AUDIO_TEST_EN && SPI_SLAVE_OR_MASTER_EN);

extern u32 dac_obuf[576 + 512];
#if CIRCULAR_BUF_EN
u8 spi1_tx_buf[SPI_TXBUF_SIZE];
u8 spi1_rx_buf[SPI_RX_MAX][SPI_RXBUF_SIZE];
u8 tmp_send_buf[576];
cbuf_t cbuf_spi1tx AT(.spi_buf);
#endif
u8 tmp_recv_buf[576] AT(.spi_recv_buf);
uint calc_crc(void *buf, uint len, uint seed);
DMABUF_VARS dbuf_spi1rx AT(.spi_buf);
SPI1_VARS spi1_trans_vars AT(.spi_buf);

/*----------------------------------------------------------------------------*/
//SPI主从机数据交互测试
//command01
//主机发送有效数据
#if 0 //简单数据测试
AT(.com_rodata.spi1_cmd)
static const u8 master_tx_cmd_01[] = {
    0xAA,0x88,  //data---
    0x44,0x22,  //data---
    0x11,0xFF,  //data---
    0x12,0x34,  //data---
    0x56,0x78,  //data---
    0x66,
};

//command08
//从机发送有效数据
AT(.com_rodata.spi1_cmd)
static const u8 slave_tx_cmd_08[] = {
    0x55,0x12,  //data---
    0x34,0x56,  //data---
    0x78,0xFF,  //data---
    0x88,0x44,  //data---
    0x22,0x11,  //data---
    0x67
};
#endif
/*----------------------------------------------------------------------------*/


/*!**************************************************************************************************
\brief          SPI1初始化
\SPI主机调用spi1_master_init，SPI从机调用spi1_salve_init
\param[in]      No param_in
\param[out]     No param_out
\return         No return
****************************************************************************************************/
AT(.text.spiflash1_drv)
void spi1_master_init(u32 baud)
{
#if SPI3WIREEN
    //SPI1_G1: CLK_PB0, DI_PB2, DO_PB1,   CS_PA9
    GPIOBFEN |= BIT(0)|BIT(1)|BIT(2);
    GPIOBDE |=  BIT(0)|BIT(1)|BIT(2);
    GPIOBDIR &= ~(BIT(0) | BIT(1));  //CLK,DO output
    GPIOBDIR |= BIT(2);  //DI input
    GPIOBPU |= BIT(2);   //DI pull up 10K
    FUNCMCON1 = (0x0F<<12);
    FUNCMCON1 = (SPI1_G1<<12);  //G5
#else
    //SPI1_G1: CLK_PB0, DIO_PB1   //CS_PA9
    GPIOBFEN |= BIT(0)|BIT(1);
    GPIOBDE |=  BIT(0)|BIT(1);
    GPIOBDIR &= ~(BIT(0) | BIT(1));  //CLK,DO output
    GPIOBDIR |= BIT(1);  //DI input
    GPIOBPU |= BIT(1);   //DI pull up 10K
    FUNCMCON1 = (0x0F<<12);
    FUNCMCON1 = (SPI1_G1<<12);  //G1
#endif
    memset(&spi1_trans_vars, 0, sizeof(spi1_trans_vars));
    spi1_trans_vars.spi1_main_sta   = SPI1_M_CHK_PROC;

    spi1_trans_vars.mic_frame       = SPI_AUDIO_FRAME;
    #if CIRCULAR_BUF_EN
    circular_buf_init(&cbuf_spi1tx, (void *)spi1_tx_buf, SPI_TXBUF_SIZE);
    #endif

    dma_buf_init();
    SPI1_CS_INIT();
    SPI1BAUD = get_sysclk_nhz()/baud - 1;//SPI主机初始化时钟
    spi1_register_init(SPI_CHK_REV);
    // spi1_kick(SPI_IRQ_REV, spi1_rx_buf, SPI_KICK_NUM);//SPI IRQ 接收设置
    // sys_irq_init(IRQ_SPI_VECTOR, 0, spi1_irq_handler);
    printf("[spi1_master_init SPI1CON: 0x%X]\n", SPI1CON);
}

AT(.text.spiflash1_drv)
void spi1_salve_init(void)
{
#if SPI3WIREEN
    //SPI1_G1: CLK_PB0, DI_PB2, DO_PB1,   CS_PA9

	GPIOBFEN |= BIT(1)|BIT(2)|BIT(0);
    GPIOBDE  |= BIT(1)|BIT(2)|BIT(0);
    GPIOBDIR &= ~BIT(1);                //DO output
    GPIOBDIR |= BIT(0)|BIT(2);
    GPIOBPU  |= BIT(0)|BIT(2);
    FUNCMCON1 = (0x0F<<12);
    FUNCMCON1 = (SPI1_G1<<12);
#else
    //SPI1_G1: CLK_PB0, DIO_PB1
	GPIOBFEN |= BIT(0)|BIT(1);
    GPIOBDE  |= BIT(0)|BIT(1);
    GPIOBDIR |= BIT(0)|BIT(1);
    GPIOBPU  |= BIT(0)|BIT(1);
    FUNCMCON1 = (0x0F<<12);
    FUNCMCON1 = (SPI1_G1<<12);
#endif
    memset(&spi1_trans_vars, 0, sizeof(spi1_trans_vars));
    spi1_trans_vars.spi1_main_sta   = SPI1_S_CHK_PROC;
    spi1_trans_vars.mic_frame       = SPI_AUDIO_FRAME;

    #if CIRCULAR_BUF_EN
    circular_buf_init(&cbuf_spi1tx, (void *)spi1_tx_buf, SPI_TXBUF_SIZE);
    #endif

    dma_buf_init();
    SPI1_CS_INIT();//cs input
    cs_edge_isr_init();

    spi1_register_init(SPI_CHK_REV);
    #if SPI1DMAINT
    SPI1CON |= BIT(SPI_RXSEL);
    #endif
    // sys_irq_init(IRQ_SPI_VECTOR, 0, spi1_irq_handler);
    // spi1_kick(SPI_CHK_REV, spi1_rx_buf[dbuf_spi1rx.rxbuf_rear], SPI_KICK_NUM);  //SPI IRQ 接收设置
    printf("[spi1_salve_init SPI1CON: 0x%X]\n", SPI1CON);
}

AT(.com_text.spi1_cs)
void cs_edge_isr_init(void)
{
    GPIOAFEN &= ~EDGE_BIT;//set PA9 to GPIO
    GPIOADE  |=  EDGE_BIT;//set PA9 to digital IO
    GPIOADIR |=  EDGE_BIT;//set PA9 to input

    PORTINTEDG |= BIT(EDGE_INT_PORT);//下降沿触发
    PORTINTEN |= BIT(EDGE_INT_PORT);//interrupt enable

    WKUPCPND = (BIT(22) | BIT(23));//clear pending
    WKUPEDG |= BIT(6);//ports falling edge
    // WKUPEDG &= ~BIT(7);//ports rising edge
    // WKUPCON |= BIT(6) | BIT(7) | BIT(16);//wakeup 6and7 enable, interrupt enable
    WKUPCON |= BIT(6) | BIT(16);//wakeup 6 enable, interrupt enable
    NMICON |= BIT(IRQ_PORT_VECTOR);
    sys_irq_init(IRQ_PORT_VECTOR, 1, cs_edge_isr);
}

AT(.text.spiflash1_drv)
void spi1_register_init(u8 in_mode)
{
    switch(in_mode){
        case SPI_DMA_REV://DMA
            //DMA接收SPI1控制寄存器设置：使能；从模式；中断使能；
            //DMA和两线模式下需要设置接收标记位
            if(SPI_SLAVE_OR_MASTER_EN){
                SPI1CON &= ~BIT(SPISM);
                // SPI1CON =  BIT(SPIEN);
                SPI1CON =  BIT(SPIEN);//| BIT(SPIIE) | BIT(SPI_RXSEL);
            }else{
                SPI1CON =  BIT(SPIEN) | BIT(SPIIE) | BIT(SPI_RXSEL) | BIT(SPISM);
            }
            #if SPI2WIREEN
            	SPI1CON |= BIT(SPI_BUSMODE_LOWBIT);				//两线模式;SPI_BUSMODE_LOWBIT
            #endif
            // sys_irq_init(IRQ_SPI_VECTOR, 0, spi1_irq_handler);
            break;
        case SPI_IRQ_REV://中断
            //中断接收SPI1控制寄存器设置：使能；从模式；中断使能;
            //三线模式下无需设置收发标记
            #if SPI3WIREEN
                if(SPI_SLAVE_OR_MASTER_EN){
                    // SPI1CPND = BIT(16);
                    SPI1CON &= ~BIT(SPISM);
                    SPI1CON =  BIT(SPIEN);
                }else{
                    SPI1CPND = BIT(16);
                    SPI1CON =  BIT(SPIEN) | BIT(SPIIE) | BIT(SPISM) ;
                }
            #elif SPI2WIREEN
            	SPI1CON =  BIT(SPI_RXSEL) |BIT(SPI_BUSMODE_LOWBIT);
            #endif
            // sys_irq_init(IRQ_SPI_VECTOR, 0,spi1_irq_handler);
            break;
        case SPI_CHK_REV://查询
            if(SPI_SLAVE_OR_MASTER_EN){
                SPI1CON =  BIT(SPIEN);
            }else{
                SPI1CON =  BIT(SPIEN) | BIT(SPISM);
            }
            break;
        default:
            break;
    }
}

//SPI1 cs
AT(.text.spiflash1_drv)
void spi1_cs_init(void)
{
    if(SPI_SLAVE_OR_MASTER_EN){
        GPIOAFEN &= ~BIT(9);
        GPIOADE |= BIT(9);
        GPIOADIR &= ~BIT(9);
        GPIOASET = BIT(9);
    }else{
        GPIOAFEN &= ~BIT(9);
        GPIOADE |= BIT(9);
        GPIOADIR |= BIT(9);
        // GPIOEPU |= BIT(4);
    }
}
AT(.com_text.spi1_audio)
void spi1_cs_en(void)
{
    if(SPI_SLAVE_OR_MASTER_EN){
        GPIOACLR = BIT(9);
    }
}
AT(.com_text.spi1_audio)
void spi1_cs_dis(void)
{
    if(SPI_SLAVE_OR_MASTER_EN){
        GPIOASET = BIT(9);
    }
}
/*
AT(.com_text.spi1_audio)
u8 spi1_cs_get(void)
{
    u8 status = GPIOA & BIT(3);
    return status;
}
*/

/*
//SPI1单字节发送
AT(.text.spiflash1_drv)
static void spi1_sendbyte(u8 data)
{
    SPI1CON &= ~BIT(4);                         //TX
    SPI1BUF = data;
    while (!(SPI1CON & BIT(SPIPND)));           //Wait pending
}
*/

// AT(.com_rodata.bat)
// const char spi_test[] = "%X ";
#if CIRCULAR_BUF_EN
//SPI主机: n字节发送, n字节接收(需要SPI从机配合进行n字节接收,n字节发送)
AT(.com_text.spi1_audio)
void spi1_send_buf_do(u8 *txbuf, u8 *rxbuf, u32 len)
{
    #if !SPI1DMAINT
    u32 i = 0;
    u32 tmp_kick_cnt = len;
    #endif
    u32 templen = len;
    u8 *tx_pt = txbuf;
    u8 *rx_pt = rxbuf;
    SPI1_CS_EN();
    delay_us(5);                                //配合SPI从机的响应速度进行调整延时
    // GLOBAL_INT_DISABLE();
    if(!is_rx_dmabuf_full()){
        dbuf_spi1rx.rxbuf_rear = (dbuf_spi1rx.rxbuf_rear + 1) % SPI_RX_MAX;
    #if !SPI1DMAINT
        // SPI1CON &= ~BIT(4);
        for(i = 0; i < templen; i++){           //TX
            SPI1BUF = *tx_pt;
            while (!(SPI1CON & BIT(SPIPND)));
            tx_pt++;
        }

        // SPI1CON |= BIT(4);
        SPI1BUF = 0xff;
        while(tmp_kick_cnt){                    //RX
            if(SPI1CON & BIT(SPIPND)){
                *rx_pt = SPI1BUF;
                // my_printf(spi_test, *rxbuf);
                SPI1BUF = 0xff;
                rx_pt++;
                tmp_kick_cnt--;
            }
            while (!(SPI1CON & BIT(SPIPND)));
        }
    #else
        SPI1DMAADR = DMA_ADR(tx_pt);
        SPI1DMACNT = templen;
        while (!(SPI1CON & BIT(SPIPND)));//TX

        SPI1CON |= BIT(SPI_RXSEL);
        delay_us(2);
        SPI1DMAADR = DMA_ADR(rx_pt);
        SPI1DMACNT = templen;
        while (!(SPI1CON & BIT(SPIPND)));//RX
        SPI1CON &= ~BIT(SPI_RXSEL);
    #endif
        dbuf_spi1rx.rx_m_succ = 1;
    }
    // GLOBAL_INT_RESTORE();
    SPI1_CS_DIS();
}
#endif

/*
//SPI DMA发送 调用之前需要确定函数spi1_tx_init已初始化
AT(.text.spiflash1_drv)
static void spi1_dma_sendbytes(void *data, u32 len)
{
    SPI1_CS_EN();
    if(0 == xcfg_cb.spi1_slave_master){
        // delay_us(50);
    }
    SPI1DMAADR = DMA_ADR(data);
    SPI1DMACNT = len;
    delay_us(100);//wait data to send
    SPI1_CS_DIS();
}
*/

/*!**************************************************************************************************
\brief          循环缓存buf数据接口(满后等待)
\param[in]      No param_in
\param[out]     No param_out
\return         No return
****************************************************************************************************/
#if CIRCULAR_BUF_EN
#define EF_PRINTF  0
#define WR_TIMES_100MS  0

#if WR_TIMES_100MS
AT(.com_text.iis_ext_cst)
const char spkr100[] = "spkrpro: [%d]  sams [%d] ----";
AT(.com_text.iis_ext_cst)
const char spkw100[] = " spkwpro: [%d]  sams [%d]  chmode[%d]\n";

u16  spkrtick = 0;
u16  spkwtick = 0;
u16  spkrcnt = 0;
u16  spkwcnt = 0;
#endif // WR_TIMES_100MS

#if EF_PRINTF
AT(.com_text.iis_ext_cst)
const char buf_f[] = "f ";
AT(.com_text.iis_ext_cst)
const char buf_e[] = "e ";
#endif

/*----------------------------------------------------------------------------*/
AT(.com_text.cir_write)
bool spk_write_to_cirbuf_process(void* buf, u16 length)
{
    if(circular_buf_is_write_able(&cbuf_spi1tx, length)){
        //my_printf("write buf\n");
        circular_buf_write(&cbuf_spi1tx, (u8 *)buf, length);  //写mic数据
        return true;
    }else{
        #if EF_PRINTF
        printf(buf_f);
        #endif
        return false;
    }
}

AT(.com_text.cir_read)
bool spk_read_by_cirbuf_process(u8 *output, u16 length)
{
    if(cbuf_spi1tx.data_len >= SPI_AUDIO_FRAME){
        circular_buf_read(&cbuf_spi1tx, output, length);  //读cbuf数据
        return true;
    }else{
        #if EF_PRINTF
        printf(buf_e);
        #endif
        return false;
    }
}

/*----------------------------------------------------------------------------*/
//循环队列缓冲判空
AT(.com_text.rx_read)
u8 is_rx_dmabuf_empty(void)
{
    if(dbuf_spi1rx.rxbuf_rear != dbuf_spi1rx.rxbuf_front){
        //my_printf("1y");
        return 0;
    }else{
        //my_printf("1n");        //缓冲区空
        return 1;
    }
}
#endif
//循环队列缓冲判满
AT(.com_text.rx_read)
u8 is_rx_dmabuf_full(void)
{
    if(dbuf_spi1rx.rxbuf_front != (dbuf_spi1rx.rxbuf_rear + 1) % SPI_RX_MAX){
        // my_printf("2y");
        return 0;
    }else{
        // my_printf("2n");         //缓冲区满
        return 1;
    }
}

AT(.text.spiflash1_drv)
void dma_buf_init(void)
{
    // u8 i;
    memset(&dbuf_spi1rx, 0, sizeof(dbuf_spi1rx));
    // for(i = 0; i < SPI_RX_MAX ; i++){
    //     memset(spi1_rx_buf[i], 0, SPI_RXBUF_SIZE);
    // }
}

/*----------------------------------------------------------------------------*/


/*!**************************************************************************************************
\brief          SPI1 irq\DMA\check 接收
\param[in]      No param_in
\param[out]     No param_out
\return         No return
****************************************************************************************************/
// AT(.com_text.spi_info)
// const char str_spi1[] = "+,%X\n";
#if CIRCULAR_BUF_EN
AT(.com_text.timer) //FIQ
void spi1_irq_handler(void)     //SPI1中断处理函数
{
    if(SPI1CON & BIT(SPIPND)){
        SPI1CPND = BIT(16);
        // my_printf(str_spi1, 0);
        //u8 rev;
        //rev = SPI1BUF;
        // my_printf(str_spi1, rev);
        //SPI1BUF = 0xFF;
        spi1_trans_vars.bo_irq_done = false;
    }
}

// AT(.text.spiflash1_drv)
AT(.com_text.spi1_kick)
void spi1_irq_read(void)    // SPI1三线中断接收
{
    SPI1CPND = BIT(16);
    // printf("WATI IRQ INT..\n");
    if(!SPI_SLAVE_OR_MASTER_EN){
        SPI1BUF = 0xFF;
    }
    // SPI1BUF = 0xff;
    //while(1);
}

// SPI1三线查询接收
AT(.com_text.spi1_kick)
void spi1_check_read(void)
{
    SPI1BUF = 0xff;
    while(!dbuf_spi1rx.rx_s_succ){
        if(SPI1CON & BIT(SPIPND)){
            //u8 r_dat = SPI1BUF;
            SPI1BUF = 0x55;
            dbuf_spi1rx.rx_cnt++;
            if(dbuf_spi1rx.rx_cnt >= SPI_KICK_NUM){
                dbuf_spi1rx.rx_cnt = 0;
                // dbuf_spi1rx.rx_s_succ = 1;
            }
            // printf("[0x%X,0x%X]",r_dat,SPI1CON);
        }
    }
}

AT(.com_text.spi1_kick)
void spi1_dma_read(void *buf, u32 len)  // SPI1三线DMA接收
{
	//printf("SPI1CON = 0x%X\n",SPI1CON);
    SPI1DMAADR = DMA_ADR(buf); //dac_obuf spi_recv_buf
    //printf("SPI1DMAADR = 0x%X,0x%X\n",SPI1DMAADR,spi_recv_buf);
    SPI1DMACNT = len;           //接收的时候需要kick start

    // printf("WATI DMA INT..\n");
    // SPI1BUF = 0xff;
    SPI1CPND = BIT(16);         //clear pending
}


//SPI1 接收接口函数
//mode:SPI_CHK_REV :查询接收
//SPI_IRQ_REV :中断单字节接收
//SPI_DMA_REV :中断DMA接收
// AT(.text.spiflash1_drv)
AT(.com_text.spi1_kick)
void spi1_kick(u8 mode, void *in_buf, u32 len)
{
    switch(mode){
        case SPI_DMA_REV: spi1_dma_read(in_buf,len);    //DMA
            break;
        case SPI_IRQ_REV: spi1_irq_read();              //中断
            break;
        case SPI_CHK_REV: spi1_check_read();            //查询
            break;
        default:
            break;
    }
}

//SPI主从机数据收发与命令解码主循环
AT(.text.spiflash1_drv)
void spi1_main_sta_switch(E_SPI1_STA e_sta)
{
    if(e_sta != spi1_trans_vars.spi1_main_sta){
        spi1_trans_vars.spi1_main_sta = e_sta;
    }
}

#define spi1_print_test     1
const char str_tx_info[] = "SPITX: M/S = %d, tx_speed: %dbyte/s, rx_speed: %dbyte/s\n";
const char str_rx_info[] = "SPIRX: M/S = %d, tx_speed: %dbyte/s, rx_speed: %dbyte/s\n";

AT(.text.spiflash1_drv)
void spi1_main_process(void)
{
#if (SPI_SLAVE_OR_MASTER_EN)
    switch(spi1_trans_vars.spi1_main_sta){
        case SPI1_M_CHK_PROC:
            if(flag_spk_data_kick){
                flag_spk_data_kick = false;
                if(spk_read_by_cirbuf_process(tmp_send_buf, SPI_AUDIO_FRAME)){
                    my_printf("spi master test\n");
                    #if 0//简单数据测试
                    memcpy(tmp_send_buf, master_tx_cmd_01, SPI_KICK_NUM);
                    memset(tmp_recv_buf, 0, sizeof(tmp_recv_buf));
                    #endif
                    spi1_send_buf_do(tmp_send_buf, tmp_recv_buf, SPI_KICK_NUM);
                    #if 0//SPI_TX_RX_TEST
                    my_print_r(tmp_send_buf, SPI_KICK_NUM);
                    #endif
                    spi1_trans_vars.spi1_send_cnt += (SPI_KICK_NUM/2);//test
                }else{
                    my_printf("E1 \n");
                    // spi1_send_buf_do((u8 *)master_fail_cmd_02, SPI_KICK_NUM);
                }
                spi1_trans_vars.wait_tick = tick_get();
            }
            break;
        case SPI1_M_RX:
            break;
        case SPI1_M_CMD:
            break;
        case SPI1_M_IDLE:
            break;
        default:
            my_printf("master error process\n");
            break;
    }
    if(1 == dbuf_spi1rx.rx_m_succ){
        dbuf_spi1rx.rx_m_succ = 0;
        dbuf_spi1rx.rxbuf_front = (dbuf_spi1rx.rxbuf_front + 1) % SPI_RX_MAX;
    }
    //printf test
    if(tick_check_expire(spi1_trans_vars.main_proc_tick, 1000)){
        spi1_trans_vars.main_proc_tick = tick_get();
        // my_printf(str_tx_info, xcfg_cb.spi1_slave_master, spi1_trans_vars.spi1_send_cnt, spi1_trans_vars.spi1_recv_cnt);
        spi1_trans_vars.spi1_send_cnt = 0;
        spi1_trans_vars.spi1_recv_cnt = 0;
    }

#else
    switch(spi1_trans_vars.spi1_main_sta){
        case SPI1_S_CHK_PROC://deafult RX
            if(flag_spk_data_kick){
                flag_spk_data_kick = false;
                if(spk_read_by_cirbuf_process(tmp_send_buf, SPI_AUDIO_FRAME)){
                    my_printf("spi slave test\n");
                    #if 0//简单数据测试
                    memcpy(tmp_send_buf, slave_tx_cmd_08, SPI_KICK_NUM);//test
                    #endif
                    spi1_trans_vars.spi1_send_cnt += (SPI_KICK_NUM/2);//test
                }else{
                    // my_printf("slave mic frame fail");
                }
            }
            break;
        case SPI1_S_RX:
            break;
        case SPI1_S_CMD:
            break;
        case SPI1_S_IDLE:
            break;
        default:
            my_printf("slave error process\n");
            break;
    }
    if(1 == dbuf_spi1rx.rx_s_succ){
        dbuf_spi1rx.rx_s_succ = 0;
        #if 0//SPI_TX_RX_TEST
        my_print_r(tmp_recv_buf,  SPI_KICK_NUM);
        memset(tmp_recv_buf, 0, sizeof(tmp_recv_buf));
        #endif
        dbuf_spi1rx.rxbuf_front = (dbuf_spi1rx.rxbuf_front + 1) % SPI_RX_MAX;
    }
    //printf test
    if(tick_check_expire(spi1_trans_vars.main_proc_tick, 1000)){
        spi1_trans_vars.main_proc_tick = tick_get();
        // my_printf(str_rx_info, xcfg_cb.spi1_slave_master, spi1_trans_vars.spi1_send_cnt, spi1_trans_vars.spi1_recv_cnt);
        spi1_trans_vars.spi1_send_cnt = 0;
        spi1_trans_vars.spi1_recv_cnt = 0;
    }
#endif
}
#endif

void spi_test_init(void)
{
    #if SPI_SLAVE_OR_MASTER_EN
    printf("spi_master_init\n");
    // spi1_master_init(1000000);//1M baud,freq is equal
    spi1_master_init(2000000);//2M also ok
    #else
        printf("spi_slave_init\n");
        spi1_salve_init();
    #endif
}

AT(.com_text.spi1_audio)
void spi_audio_send_test(void *buf, u16 length)
{
    #if !SPI1DMAINT
    u32 i = 0;
    #endif
    u32 templen = length;
    u8 *tx_pt = buf;
    SPI1_CS_EN();
    // delay_ms(30);                                //配合SPI从机的响应速度进行调整延时

    if(!is_rx_dmabuf_full()){
    #if !SPI1DMAINT
        for(i = 0; i < templen; i++){           //TX
            SPI1BUF = *tx_pt;
            while (!(SPI1CON & BIT(SPIPND)));
            tx_pt++;
        }
        SPI1BUF = 0xff;
    #else
        SPI1DMAADR = DMA_ADR(tx_pt);
        SPI1DMACNT = templen;
        while (!(SPI1CON & BIT(SPIPND)));//TX
        delay_us(2);
        SPI1CON &= ~BIT(SPI_RXSEL);
    #endif
        dbuf_spi1rx.rx_m_succ = 1;
    }
    SPI1_CS_DIS();
}

// AT(.com_text.isr_ch)
// const char isr_ch[] = "f \n";
// AT(.com_text.isr_ch1)
// const char isr_ch1[] = "g \n";
// AT(.com_text.isr_ch2)
// const char isr_ch2[] = "h \n";
//SPI1从机n字节接收
AT(.com_text.spi1_cs) FIQ
void cs_edge_isr(void)
{
    u32 templen = SPI_KICK_NUM;
    u32 tmp_kick_cnt = SPI_KICK_NUM;
    #if !SPI1DMAINT
    u32 i = 0;
    #endif

    if(WKUPEDG & BIT(22))//fall
    {
        WKUPCPND = BIT(22);

        if(!SPI_SLAVE_OR_MASTER_EN){     //主机cs拉低kick从机
        #if !SPI1DMAINT
            SPI1BUF = 0xff;
            while(tmp_kick_cnt){                //RX
                if(SPI1CON & BIT(SPIPND)){
                    tmp_recv_buf[i] = SPI1BUF;
                    SPI1BUF = 0xff;
                    i++;
                    tmp_kick_cnt--;
                }
            }
            dbuf_spi1rx.rx_s_succ = 1;

        #else
            SPI1DMAADR = DMA_ADR(tmp_recv_buf);
            SPI1DMACNT = tmp_kick_cnt;
            while (!(SPI1CON & BIT(SPIPND)));//RX
            dbuf_spi1rx.rx_s_succ = 1;
        #endif
            #if SPI_AUDIO_TEST_EN
                dac_gpdma_kick((s16*)tmp_recv_buf,templen);
            #endif
        }
    }
}


#endif

