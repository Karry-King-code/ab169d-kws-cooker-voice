#include "include.h"

#if 1   //DEBUG
#define dump_printf    my_printf
#else
#define dump_printf(...)
#endif
//dump使用示例请查看 bsp_ext_dump_buf_test.c中的dump_buf_test()函数
//----dump  api-------------------------------------------------------------
//buf:        dump模块需要的buf
//buf_len:    长度为 (14 * (file_toal + 1) + DMA_LEN), 其中DMA_LEN表示putbuf一次发送的数据长度
//file_toal:  dump模块同时支持导出的文件个数. 目前PC端软件UartDump暂时只支持同时接收2路并保存.
//putbuf:     底层dump函数,这里一般为高速串口DMA发送函数
//wait:       dump等待函数,下一次putbuf时,需要等待上一次putbuf结束
void dump_buf_init(u8 *buf, int buf_len, int file_toal,void (*putbuf)(void*buf,u32 len), void (*wait)(void));

//buf:      需要导出数据的BUF
//buf_len:  导出数据的长度
//file_idx: 文件序号,0表示UartDump收到数据后保存到文件0, 1表示保存到文件1
void dump_putbuf(u8 *buf,  int buf_len, u8 file_idx);
//-----user config---------------------------------------------------------
#define DUMP_HEAD_NUM       2               // 支持同时导出几路数据(目前PC端工具UartDump最大只支持接收两路)
#define HEAD_TAG            0x37365455      //UT67
#define HEAD_LEN            14              //sizeof(DUMP_HEAD)
//dump数据包结构:
//4byte(tag) + 4byte(pkt_cnt) + 2byte(pkt_len) + 2byte(crc)+1byte(type)+1byte(head_sum) + dump实际数据
typedef struct _DUMP_HEAD {
    u32 tag;
    u32 pkt_cnt;
    u16 pkt_len;
    u16 crc;          //crc16
    u8  type;         //bit7 crc_en //bit[3:0]  file_idx
    u8  head_sum;
}PACKED DUMP_HEAD  ;  //#define PACKED  __attribute__((packed))

typedef struct _DUMP_SPI_PROTOCOL {
    DUMP_HEAD *head[DUMP_HEAD_NUM];
    u8 *head_buf;           //head buf (14byte)
    u8 *data_buf;           //data_buf = head_buf + sizeof(DUMP_HEAD)
    u16 data_buf_len;       //not include head
    u16 tx_len;             //一次发送的长度
    void (*putbuf)(void* buf,u32 buf_len);
    void (*wait_put_finish)(void);
}DUMP_PROTOCOL;
static DUMP_PROTOCOL dump;
//-------------------------------------------------------------------------
//dump_buf_init中的buf数据结构
//head[0]
//head[...]
//head[DUMP_HEAD_NUM-1]
//head_buf  //buf tx_start addr
//data_buf
//####### dump_buf_init中,buf长度建议为: 14*(file_toal+1) + DataDmaLen
AT(.text.dumpbuf)
void dump_buf_init(u8 *buf, int buf_len, int file_toal,void (*putbuf)(void*buf,u32 len), void (*wait)(void))
{
    int i, len_min;
    if (NULL == buf ||  file_toal < 1) {
        dump_printf("dump_var_init param ERR\n");
        return;
    }
    len_min = (file_toal + 1) * HEAD_LEN + 4;
    if(buf_len <  len_min) {
        dump_printf("!!!Error, dump_var_init , buf_len = %d, is to small\n",len_min);
        return;
    }
    memset(buf,0x00,buf_len);
    memset(&dump,0x00,sizeof(DUMP_PROTOCOL));
    dump.head_buf =   buf +  file_toal * HEAD_LEN;
    dump.data_buf =   buf + (file_toal + 1)  * HEAD_LEN;
    dump.data_buf_len = buf_len - (file_toal + 1) * HEAD_LEN;
    dump.tx_len = dump.data_buf_len +HEAD_LEN;
    //head
    for(i = 0; i < file_toal; i++) {
        dump.head[i] = (DUMP_HEAD*)(buf + i * HEAD_LEN);
        dump.head[i]->tag = HEAD_TAG;  //UT67
        dump.head[i]->pkt_len = dump.data_buf_len;
        dump.head[i]->type = (u8)i & 0x0F;
    }
    dump.putbuf = putbuf;
    dump.wait_put_finish = wait;
    dump_printf("sizeof(DUMP_HEAD) = %d, dump.data_buf_len = %d\n",HEAD_LEN,dump.data_buf_len);
    dump_printf("-->dump_buf_init ok\n");
}

AT(.com_text.dumpbuf.crc)
static u16 calc_sum(void *buf, uint len)
{
    u16 sum = 0;
    u8* buf8 = (u8*)buf;
    for(uint i = 0; i < len; i++) {
        sum += buf8[i];
    }
    return sum;
}

AT(.com_text.dumpbuf.cststr)
const char dump_str_buf_err[] = "dump_buf len err(%d,%d)\n";

AT(.com_text.dumpbuf)
void dump_putbuf(u8 *buf,  int buf_len, u8 file_idx)
{
    if(buf_len > dump.data_buf_len) {
        dump_printf(dump_str_buf_err,buf_len,dump.data_buf_len);
        return;
    }
    dump.wait_put_finish();  //等待上一次发送完成
    memcpy(dump.data_buf, buf, buf_len);
    dump.head[file_idx]->pkt_cnt++;
    dump.head[file_idx]->pkt_len = buf_len;
    dump.head[file_idx]->crc = calc_sum(dump.data_buf, buf_len);
    dump.head[file_idx]->head_sum = calc_sum(&(dump.head[file_idx]->pkt_cnt),9);
    //dump_printf("pkt_cnt = %d, head_sum = 0x%X, pkt_crc = 0x%X\n",dump.head[file_idx]->pkt_cnt, dump.head[file_idx]->head_sum, dump.head[file_idx]->crc);
    memcpy(dump.head_buf, dump.head[file_idx], HEAD_LEN);
    dump.putbuf(dump.head_buf, buf_len + HEAD_LEN);
}

//DUMP_BUF 测试程序,使用高速串口DMA方式导出数据,以下程序在530X上测试OK
#define HUART_DUMP_BAUD          6000000 // 9600
#define HUART_DUMP_BAUD_VAL      (((24000000 + (HUART_DUMP_BAUD / 2)) / HUART_DUMP_BAUD) - 1)
void huart_module_init(void)   //huart DMA输出测试  //测试发现huart暂时只支持26M时钟
{
    FUNCMCON0 = 0x0f << 20 | 0x0f << 16;
#if 0   //PA5,注意芯片引脚如果是多IO复用的,会影响dump
    FUNCMCON0 = 0x01 << 20 | HSTX_PA5;
    GPIOADE  |=  BIT(5);
    GPIOADIR &= ~BIT(5);
    GPIOAFEN |=  BIT(5);
#endif
#if 1   //PA10
    FUNCMCON0 = 0x01 << 20 | HSTX_PA10;
    GPIOADE  |=  BIT(10);
    GPIOADIR &= ~BIT(10);
    GPIOAFEN |=  BIT(10);
#endif

    CLKGAT0 |= BIT(11);                             //HUART CLK enable
    CLKCON0 &= ~(BIT(17)|BIT(18)|BIT(19));          //HUART 24M CLK

    HSUT0BAUD = (HUART_DUMP_BAUD_VAL << 16) | HUART_DUMP_BAUD_VAL;
    HSUT0CON = 0;
    //HSUT0CON |= BIT(9);        //BIT9 = 1, 2 bit stop bit  //0 : 1BIT
    HSUT0CON |= BIT(7);       //TX DMA MODE
    HSUT0CON |= BIT(17) | BIT(16);   //cfg sync
    HSUT0CPND = 0xffff;
    HSUT0CON |= BIT(1);       //TX EN
    static u8 buf_first_kick[2] = {0x55,0xAA};  //kick for first Pending of DMA finish.
    HSUT0TXADR = (u32)buf_first_kick;
    HSUT0TXCNT = 2;
    printf("hsut0 init,HSUT0CON = 0x%X,HUART_BAUD_VAL = %d\n",HSUT0CON,HUART_DUMP_BAUD_VAL);
}

AT(.com_text.dumpbuf_test)
void huart_wait_tx_finish(void)
{
    while(!(HSUT0CON&BIT(13))); //1 DMA finish  //0 dma not finish
}

AT(.com_text.dumpbuf_test)
void huart_putbuf(void *buf, u32 len)
{
    if (0 == len) {
        return;
    }
    huart_wait_tx_finish();     //等待上一次发送完成
    HSUT0TXADR = DMA_ADR(buf);
    HSUT0TXCNT = len;           //kick start auto clear tx pending
}



//测试 huart_putbuf 函数能否发送成功, dump_buf_init初始化前需要确认发送函数是否OK.
void dumphuart_io_test(void)
{
    huart_module_init();
    WDT_DIS();
    u8 dma_test_buf[] = {0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88};
    while(1) {
        huart_putbuf(dma_test_buf,8);
        delay_5ms(10);
        printf("->");
    }
}

//循环发送正弦波数组进行测试
static u8 sinetbl_test[256] = {   //32 samples  //sine_16k_500hz_dual
	0x00, 0x00, 0x00, 0x00, 0x84, 0x0C, 0x84, 0x0C, 0x8C, 0x18, 0x8C, 0x18, 0xA4, 0x23, 0xA4, 0x23,
	0x5C, 0x2D, 0x5C, 0x2D, 0x56, 0x35, 0x56, 0x35, 0x44, 0x3B, 0x44, 0x3B, 0xEB, 0x3E, 0xEB, 0x3E,
	0x26, 0x40, 0x26, 0x40, 0xEB, 0x3E, 0xEB, 0x3E, 0x43, 0x3B, 0x43, 0x3B, 0x57, 0x35, 0x57, 0x35,
	0x5D, 0x2D, 0x5D, 0x2D, 0xA4, 0x23, 0xA4, 0x23, 0x8D, 0x18, 0x8D, 0x18, 0x83, 0x0C, 0x83, 0x0C,
	0x00, 0x00, 0x00, 0x00, 0x7C, 0xF3, 0x7C, 0xF3, 0x74, 0xE7, 0x74, 0xE7, 0x5D, 0xDC, 0x5D, 0xDC,
	0xA4, 0xD2, 0xA4, 0xD2, 0xA9, 0xCA, 0xA9, 0xCA, 0xBC, 0xC4, 0xBC, 0xC4, 0x16, 0xC1, 0x16, 0xC1,
	0xD9, 0xBF, 0xD9, 0xBF, 0x15, 0xC1, 0x15, 0xC1, 0xBC, 0xC4, 0xBC, 0xC4, 0xA8, 0xCA, 0xA8, 0xCA,
	0xA4, 0xD2, 0xA4, 0xD2, 0x5C, 0xDC, 0x5C, 0xDC, 0x74, 0xE7, 0x74, 0xE7, 0x7B, 0xF3, 0x7B, 0xF3,

    0x00, 0x00, 0x00, 0x00, 0x84, 0x0C, 0x84, 0x0C, 0x8C, 0x18, 0x8C, 0x18, 0xA4, 0x23, 0xA4, 0x23,
	0x5C, 0x2D, 0x5C, 0x2D, 0x56, 0x35, 0x56, 0x35, 0x44, 0x3B, 0x44, 0x3B, 0xEB, 0x3E, 0xEB, 0x3E,
	0x26, 0x40, 0x26, 0x40, 0xEB, 0x3E, 0xEB, 0x3E, 0x43, 0x3B, 0x43, 0x3B, 0x57, 0x35, 0x57, 0x35,
	0x5D, 0x2D, 0x5D, 0x2D, 0xA4, 0x23, 0xA4, 0x23, 0x8D, 0x18, 0x8D, 0x18, 0x83, 0x0C, 0x83, 0x0C,
	0x00, 0x00, 0x00, 0x00, 0x7C, 0xF3, 0x7C, 0xF3, 0x74, 0xE7, 0x74, 0xE7, 0x5D, 0xDC, 0x5D, 0xDC,
	0xA4, 0xD2, 0xA4, 0xD2, 0xA9, 0xCA, 0xA9, 0xCA, 0xBC, 0xC4, 0xBC, 0xC4, 0x16, 0xC1, 0x16, 0xC1,
	0xD9, 0xBF, 0xD9, 0xBF, 0x15, 0xC1, 0x15, 0xC1, 0xBC, 0xC4, 0xBC, 0xC4, 0xA8, 0xCA, 0xA8, 0xCA,
	0xA4, 0xD2, 0xA4, 0xD2, 0x5C, 0xDC, 0x5C, 0xDC, 0x74, 0xE7, 0x74, 0xE7, 0x7B, 0xF3, 0x7B, 0xF3
};

//dump使用示例
#define DUMP_BUF_LEN  (14 * (2 + 1) + 1280)  //长度为 (14 * (file_toal + 1) + DMA_LEN), 其中DMA_LEN表示putbuf一次发送的数据长度
static u8 dump_cache_buf[DUMP_BUF_LEN] AT(.huart_dump_buf);
void dump_buf_test(void)
{
    //dumphuart_io_test();  //测试高速串口DMA发送是否正常
    huart_module_init();    //初始化高速串口模块
    dump_buf_init(dump_cache_buf,DUMP_BUF_LEN,2,huart_putbuf,huart_wait_tx_finish);  //初始化dump模块
    printf("dump run...\n");
    int cnt = 65536;  //65536*128 = 8M (8388608 byte)
    WDT_DIS();
    while(cnt--) {  //同时导出两个文件测试
        dump_putbuf(sinetbl_test,256,0);  //导出的数据会保存到文件0
        dump_putbuf(sinetbl_test,256,1);  //导出的数据会保存到文件1
        if(cnt % 1024 == 0) {
            printf(".");
        }
    }
    printf("dump_buf_test finish\n"); while(1);
}

void huart_dump_init(void)
{
    huart_module_init();
    dump_buf_init(dump_cache_buf,DUMP_BUF_LEN,2,huart_putbuf,huart_wait_tx_finish);
}

#if SPK_SPI_DUMP
///SPI DEBUG
#define CONST_CAT(x, y)         x ## y
#define SET_MACRO(x, y)         CONST_CAT(x, y)

#define SCLK_GP             A
#define SDAT_GP             A
#define SCLK_BIT            BIT(7)
#define SDAT_BIT            BIT(4)

#define SCLK_GPIODE         SET_MACRO(GPIO, SET_MACRO(SCLK_GP, DE))
#define SCLK_GPIOFEN        SET_MACRO(GPIO, SET_MACRO(SCLK_GP, FEN))
#define SCLK_GPIODIR        SET_MACRO(GPIO, SET_MACRO(SCLK_GP, DIR))
#define SCLK_GPIOSET        SET_MACRO(GPIO, SET_MACRO(SCLK_GP, SET))
#define SCLK_GPIOCLR        SET_MACRO(GPIO, SET_MACRO(SCLK_GP, CLR))
#define SDAT_GPIODE         SET_MACRO(GPIO, SET_MACRO(SDAT_GP, DE))
#define SDAT_GPIOFEN        SET_MACRO(GPIO, SET_MACRO(SDAT_GP, FEN))
#define SDAT_GPIODIR        SET_MACRO(GPIO, SET_MACRO(SDAT_GP, DIR))
#define SDAT_GPIOSET        SET_MACRO(GPIO, SET_MACRO(SDAT_GP, SET))
#define SDAT_GPIOCLR        SET_MACRO(GPIO, SET_MACRO(SDAT_GP, CLR))

void my_spi_init(void)
{
    SCLK_GPIODE |= SCLK_BIT;
    SCLK_GPIOFEN &= ~SCLK_BIT;
    SCLK_GPIODIR &= ~SCLK_BIT;
    SCLK_GPIOSET = SCLK_BIT;

    SDAT_GPIODE |= SDAT_BIT;
    SDAT_GPIOFEN &= ~SDAT_BIT;
    SDAT_GPIODIR &= ~SDAT_BIT;
    SDAT_GPIOSET = SDAT_BIT;
}

AT(.com_text.sys)
void my_spi_putc(u8 ch)
{
   GLOBAL_INT_DISABLE();
   u8 i;
   for(i=0; i<8; i++) {
       SCLK_GPIOCLR = SCLK_BIT;
       if(ch & 0x80) {
           SDAT_GPIOSET = SDAT_BIT;
       } else {
           SDAT_GPIOCLR = SDAT_BIT;
       }
       ch <<= 1;
       SCLK_GPIOSET = SCLK_BIT;
   }
   GLOBAL_INT_RESTORE();
}
#endif
