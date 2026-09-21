#ifndef __BSP_SPI_AUDIO_H__
#define __BSP_SPI_AUDIO_H__

#define     SPI3WIREEN			1
#define     SPI2WIREEN			0
#define     SPI1DMAINT			1       //0-IRQ/CHK, 1-DMA
#define     SPI1AUXEN			0

#define     CIRCULAR_BUF_EN     0

#define     SPIEN               0       //SPI Enable bit 0:disable 1:enable
#define     SPISM               1       //Slave mode 0:master 1:slave
#define     SPI_BUSMODE_LOWBIT  2       //data bus width select low bit
#define     SPI_BUSMODE_HIGNBIT 3       //data bus width select high bit
#define     SPI_RXSEL           4       //TX:0 RX:1
#define     SPI_CLKIDS          5       //SPI clock state when idle 0:clock stay at 0 1:clk stay at 1
#define     SPI_SMPS            6       //SPI output edge select bit 0: falling edge 1:rising edge
#define     SPIIE               7       //SPI interrupt enable 0:disable 1:enable
#define     SPILF_EN            8       //SPI LFSR enable bit 0:disable 1:enable
#define     SPIMBEN             9       //SPI multiple bit bus enable bit
#define     SPIIOSS             10      //SPI sample data is at the same clock edge with output data 0:difference 1:same
#define     SPIPND              16      //SPI pending

#define     SPI_CMD_HEAD        4       //head + data len
#define     SPI_AUDIO_ESBC      10
#define     SPI_AUDIO_FRAME     576

#define     SPI_KICK_NUM        576   //576 //每次CS下拉收发的包大小,例如设2,就是tx-1byte -> rx-1byte -> tx-1byte -> rx-1byte,如此循环

#define     SPI_CHK_REV         0
#define     SPI_IRQ_REV         1
#define     SPI_DMA_REV         2

#define     AUBUFCON        	SFR_RW (SFR1_BASE + 0x02*4)
#define     NMICON              SFR_RW (SFR4_BASE + 0x02*4)

typedef struct{
    u32 spien:1;        /*bit0, SPI Enable bit 0:disable 1:enable*/
    u32 spism:1;        /*bit1, Slave mode 0:master 1:slave*/
    u32 busmode:2;      /*bit2-bit3, 00:3wire mode, 01:2wire mode, 10:2bit bidrectional data bus, 11:4bit bidrectional data bus*/
    u32 rxsel:1;        /*bit4, in DMA or 2-wire, 0:transmit, 1:receive*/
    u32 clkids:1;       /*bit5, SPI clock state when idle 0:clock stay at 0 1:clk stay at 1*/
    u32 smps:1;         /*bit6, SPI output edge select bit 0: falling edge 1:rising edge*/
    u32 spiie:1;        /*bit7, SPI interrupt enable 0:disable 1:enable*/
    u32 spilf_en:1;     /*bit8, SPI LFSR enable bit 0:disable 1:enable*/
    u32 spimben:1;      /*bit9, SPI multiple bit bus enable bit,0:disable 1:enable*/
    u32 spioss:1;       /*bit10,SPI sample data is at the same clock edge with output data 0:difference 1:same*/
    u32 holdenrx:1;     /*bit11,SPI hold enable when bt rx, 0:disable 1:enable*/
    u32 holdentx:1;     /*bit12,SPI hold enable when bt tx, 0:disable 1:enable*/
    u32 holdensw:1;     /*bit13,SPI software hold enable, 0:disable 1:enable*/
    u32 reserved1:2;    /*bit14-bit15, Unused*/
    u32 spipnd:1;       /*bit16,SPI pending, 0:nott finish SPI rx/tx, 1:finish SPI rx/tx*/
    u32 resetved2:15;   /*bit17-bit31, Unused*/
}SPI1_CTRL_REGISTER;

typedef enum{
    MASTER_SLAVE_MODE,  //主从模式
    SYNC_MODE,          //同步模式
    DATA_EXCHG_MODE     //数据交换模式
}E_SPI1_DATA_MODE;

typedef enum{
    SPI1_MODE_0,     //CPOL=0, CPHA=0
    SPI1_MODE_1,     //CPOL=0, CPHA=1
    SPI1_MODE_2,     //CPOL=1, CPHA=0
    SPI1_MODE_3      //CPOL=1, CPHA=1
}E_SPI1_CLK_MODE;

typedef enum{
    SPI1_G1 = 0x01, //SPI_CLK:PB0, SPI_DI:PB2, SPI_DO/SPI_DATA:PB1
    SPI1_G2,        //SPI_CLK:PA14, SPI_DI:PA13, SPI_DO/SPI_DATA:PA15
    SPI1_G3,        //SPI_CLK:PA6, SPI_DI:PA8, SPI_DO/SPI_DATA:PA7
    SPI1_G4,        //SPI_CLK:PB8, SPI_DI:PB7, SPI_DO/SPI_DATA:PB9
    SPI1_G5         //SPI_CLK:PB1, SPI_DI:PB2, SPI_DO/SPI_DATA:PB9
}E_SPI1_IO;

typedef enum{
    SPI1_S_CHK_PROC,
    SPI1_S_TX,
    SPI1_S_RX,
    SPI1_S_CMD,
    SPI1_S_IDLE,

    SPI1_M_CHK_PROC,
    SPI1_M_TX,
    SPI1_M_RX,
    SPI1_M_CMD,
    SPI1_M_IDLE
}E_SPI1_STA;

typedef struct{
    volatile u8 rx_cnt;         //计数是否收齐一帧SPI_KICK_NUM
    volatile u8 rxbuf_rear;     //循环队列尾指针
    volatile u8 rxbuf_front;    //循环队列头指针

    volatile u8 rx_m_succ;     //主机rx成功
    volatile u8 rx_s_succ;     //主机rx成功
}DMABUF_VARS;

typedef struct{

    volatile bool bo_irq_done;//中断是否完成
    volatile E_SPI1_STA spi1_main_sta;//SPI状态

    u32     main_proc_tick;
    u32     spi1_send_cnt;      //发送计数/s
    u32     spi1_recv_cnt;      //接收计数/s

    u32     wait_tick;
    bool    rx_data_kick;
    u32     slave_tx_interval;  //从机tx间隔


    u32     mic_frame;//mic帧长度_byte
    u8      cs_change;

    // u32     tx_dma_addr;
    // u32     rx_dma_addr;

    // u8      bits_per_word;
    // u16     delay_use_cs;
    // u32     speed_hz;
    // u16     word_delay;
}SPI1_VARS;

void spi1_master_init(u32 baud);            //发送端初始化SPI1
void spi1_salve_init(void);
void cs_edge_isr_init(void);
void spi1_register_init(u8 in_mode);

void cs_edge_isr(void);
void spi1_send_buf_do(u8 *txbuf, u8 *rxbuf, u32 len);

bool spk_write_to_cirbuf_process(void* buf, u16 length);
bool spk_read_by_cirbuf_process(u8 *output, u16 length);

void dma_buf_init(void);
u8 is_rx_dmabuf_empty(void);
u8 is_rx_dmabuf_full(void);

void spi1_irq_handler(void);
void spi1_kick(u8 mode, void *buf, u32 len);    //RX 接收启动接口

void dac_src0_dma_out_proc(void *buf, uint len);
void dac_src0_onebyte_out_proc(u32 aubufdata);
void dac_src0_drc_stereo_proc(s16 *left, s16 *right);

extern void spi1_main_sta_switch(E_SPI1_STA e_sta);
void spi1_main_process(void);
void spi_test_init(void);
void spi_audio_send_test(void *buf, u16 length);
#endif //__BSP_SPI_AUDIO_H__
