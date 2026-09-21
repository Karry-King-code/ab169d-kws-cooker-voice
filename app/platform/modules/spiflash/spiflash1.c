#include "include.h"
#include "spiflash1.h"

#if EX_SPIFLASH_SUPPORT

#if 0
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif


void spi1_cs_en(void);
void spi1_cs_dis(void);
void spi1_cs_init(void);

#define SPI1_CS_INIT()           spi1_cs_init()
#define SPI1_CS_EN()             spi1_cs_en()
#define SPI1_CS_DIS()            spi1_cs_dis()

#define SF_PROGRAM              0x02                //SPIFlash编程
#define SF_READ                 0x03                //SPIFlash读取
#define SF_READSSR              0x05                //SPIFlash读状态寄存器
#define SF_WRITE_EN             0x06                //SPIFlash写使能
#define SF_ERASE                0x20                //SPIFlash擦除 //sector erase(4K擦除)
#if FLASH_GREATER_THAN_16MBYTE
#define Enter4byteAddrs         0xb7
#endif

#define SPIFLASH_ID_NUM         6                   //支持的flash id数量

gpio_t spi1_gpio AT(.buf.spi1buf);

AT(.rodata.spiflash1_id_tbl)
const u32 spiflash1_id_table[SPIFLASH_ID_NUM] = {
    0x40140000,
    0x40180B00,
	0x40170000,
    0x40182000,
    0x40155E00,
    0x40150000,
};

AT(.text.spiflash1_drv)
void spi1_cs_init(void)
{
    gpio_t *g = &spi1_gpio;
    gpio_cfg_init(g, SPIFLASH_CS_IO);
    if (g->sfr) {
        g->sfr[GPIOxFEN] &= ~g->pin;
        g->sfr[GPIOxDE]  |=  g->pin;
        g->sfr[GPIOxDIR] &= ~g->pin;
        g->sfr[GPIOxSET]  =  g->pin;
    }
}

AT(.text.spiflash1_drv)
void spi1_cs_en(void)
{
    gpio_t *g = &spi1_gpio;
    g->sfr[GPIOxCLR]  =  g->pin;
}

AT(.text.spiflash1_drv)
void spi1_cs_dis(void)
{
    gpio_t *g = &spi1_gpio;
    g->sfr[GPIOxSET]  =  g->pin;
}

///SPIFlash Delay
AT(.text.spiflash1_drv)
static void spi1_delay(void)
{
    uint cnt = 20;
    while (cnt--) {
        asm("nop");
    }
}

///SPI接口获取1Byte数据
AT(.text.spiflash1_drv)
static u8 spi1_getbyte(void)
{
    SPI1CON |= BIT(4);                              //RX
    SPI1BUF = 0xff;
    while (!(SPI1CON & BIT(16)));                   //Wait pending
    return SPI1BUF;
}

///SPI接口发送1Byte数据
AT(.text.spiflash1_drv)
static void spi1_sendbyte(u8 data)
{
    SPI1CON &= ~BIT(4);                             //TX
    SPI1BUF = data;
    while (!(SPI1CON & BIT(16)));                   //Wait pending
}

AT(.text.spiflash1_drv)
u32 spiflash1_id_read(void)
{
    u32 id = 0,i;
    SPI1_CS_EN();
    spi1_delay();
    spi1_sendbyte(0x9F);
    for (i = 0; i < 4; i++) {
        id |= spi1_getbyte();
        id <<= 8;
    }
    SPI1_CS_DIS();
    return id;
}

AT(.text.spiflash1_drv)
void spi1_init(u32 baud)
{
#if   SPIFLASH_MAP == SPI1MAP_G1
    //SPI1_G1: CLK_PA3, DIO_PA4,   //CS_PA6
    GPIOAFEN |=   BIT(3)|BIT(4);
    GPIOADE  |=   BIT(3)|BIT(4);
    GPIOADIR &= ~(BIT(3)|BIT(4));   //CLK,DO output
    GPIOADIR |=   BIT(4);           //DI input
    GPIOAPU  |=   BIT(4);           //DI pull up 10K
    FUNCMCON1 = (0x0F<<12);
    FUNCMCON1 = SPI1MAP_G1;
#elif SPIFLASH_MAP == SPI1MAP_G2
    //SPI1_G2: CLK_PA8, DIO_PA9,   //CS_PA6
    GPIOAFEN |=   BIT(8)|BIT(9);
    GPIOADE  |=   BIT(8)|BIT(9);
    GPIOADIR &= ~(BIT(8)|BIT(9));  //CLK,DO output
    GPIOADIR |=   BIT(9);          //DI input
    GPIOAPU  |=   BIT(9);          //DI pull up 10K
    FUNCMCON1 = (0x0F<<12);
    FUNCMCON1 = SPI1MAP_G2;
#elif SPIFLASH_MAP == SPI1MAP_G3
    //SPI1_G3: CLK_PA12, DIO_PA13,   //CS_PA6
    GPIOAFEN |=   BIT(12)|BIT(13);
    GPIOADE  |=   BIT(12)|BIT(13);
    GPIOADIR &= ~(BIT(12)|BIT(13));  //CLK,DO output
    GPIOADIR |=   BIT(13);           //DI input
    GPIOAPU  |=   BIT(13);           //DI pull up 10K
    FUNCMCON1 = (0x0F<<12);
    FUNCMCON1 = SPI1MAP_G3;
#elif SPIFLASH_MAP == SPI1MAP_G4
    //SPI1_G4: CLK_PA1, DIO_PA2,   //CS_PA6
    GPIOAFEN |=   BIT(1)|BIT(2);
    GPIOADE  |=   BIT(1)|BIT(2);
    GPIOADIR &= ~(BIT(1)|BIT(2));   //CLK,DO output
    GPIOADIR |=   BIT(2);           //DI input
    GPIOAPU  |=   BIT(2);           //DI pull up 10K
    FUNCMCON1 = (0x0F<<12);
    FUNCMCON1 = SPI1MAP_G4;
#endif
    SPI1_CS_INIT();
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    SPI1BAUD = 24000000/baud - 1;   //sys 24M.
    SPI1CON =  0x01 | (1<<2);
#if FLASH_GREATER_THAN_16MBYTE
    delay_5ms(10);
    enter_4byte_addr();
    delay_5ms(10);
#endif
}

///SPIFlash写使能
AT(.text.spiflash1_drv)
void spiflash1_init(void)
{
    spi1_init(SPIFLASH_BAUD);
    spiflash1_release_pd();
    //while(1) {WDT_CLR();printf("Flash ID = 0x%X\n", spiflash1_id_read());delay_ms(300);}
}

///SPIFlash写使能
AT(.text.spiflash1_drv)
static void spiflash1_write_enable(void)
{
    SPI1_CS_EN();
    spi1_sendbyte(SF_WRITE_EN);
    SPI1_CS_DIS();
    spi1_delay();
}

///SPIFlash读状态寄存器
AT(.text.spiflash1_drv)
static uint spiflash1_readssr(void)
{
    uint ssr;
    SPI1_CS_EN();
    spi1_sendbyte(SF_READSSR);
    ssr = spi1_getbyte();
    SPI1_CS_DIS();
    spi1_delay();
    return ssr;
}

///SPIFlash等待BUSY
AT(.text.spiflash1_drv)
static void spiflash1_waitbusy(void)
{
    do {
        spi1_delay();
    } while (spiflash1_readssr() & 0x01);
}

///发送SPIFlash的地址
AT(.text.spiflash1_drv)
static void spiflash1_sendaddr(u32 addr)
{
#if FLASH_GREATER_THAN_16MBYTE
    spi1_sendbyte(addr >> 24);
#endif
    spi1_sendbyte(addr >> 16);
    spi1_sendbyte(addr >> 8);
    spi1_sendbyte(addr);
}

///SPIFlash读取
AT(.text.spiflash1_drv)
void spiflash1_read(void *buf, u32 addr, u32 len)
{
    TRACE("[r:0x%X,%d]",addr, len);
    u32 i;
    u8 *read_buf = (u8*)buf;
    SPI1_CS_EN();
    spi1_sendbyte(SF_READ);
    spiflash1_sendaddr(addr);
    for (i = 0; i < len; i++) {
       read_buf[i] = spi1_getbyte();
    }
    SPI1_CS_DIS();
}

#if FLASH_GREATER_THAN_16MBYTE
void enter_4byte_addr(void)
{
    spiflash1_write_enable();
    SPI1_CS_EN();
    spi1_sendbyte(Enter4byteAddrs);
    SPI1_CS_DIS();
    spiflash1_waitbusy();
}
#endif

///SPIFlash编程
AT(.text.spiflash1_drv)
void spiflash1_write(void *buf, u32 addr, u32 len)
{
    TRACE("[w:0x%X,%d]",addr, len);
    u8 *write_buf = (u8*)buf;
    spiflash1_write_enable();
    SPI1_CS_EN();
    spi1_sendbyte(SF_PROGRAM);
    spiflash1_sendaddr(addr);
#if 0
    SPI1CON &= ~BIT(4);                             //TX
    SPI1DMAADR = DMA_ADR(write_buf);
    SPI1DMACNT = len;
    while (!(SPI1CON & BIT(16)));                   //Wait pending
#else
   u32 i;
   for (i = 0; i < len; i++) {
       spi1_sendbyte(write_buf[i]);
   }
#endif
    SPI1_CS_DIS();
    spiflash1_waitbusy();
}

///SPIFlash擦除
AT(.text.spiflash1_drv)
void spiflash1_erase(u32 addr)
{
    TRACE("spi erase: %x\n", addr);
    spiflash1_write_enable();

    SPI1_CS_EN();
    spi1_sendbyte(SF_ERASE);
    spiflash1_sendaddr(addr);
    SPI1_CS_DIS();

    spiflash1_waitbusy();
}

////#define SF_ERASE_BLOCK          0xD8              //SPIFlash擦除  //block erase   //64K
//AT(.text.spiflash1_drv)
//void spiflash1_erase_block(u32 addr)
//{
//    TRACE("spi erase: %x\n", addr);
//    spiflash1_write_enable();
//
//    SPI1_CS_EN();
//    spi1_sendbyte(SF_ERASE_BLOCK);
//    spiflash1_sendaddr(addr);
//    SPI1_CS_DIS();
//
//    spiflash1_waitbusy();
//}

///进入power_down模式
AT(.text.spiflash1_drv)
void spiflash1_power_down(void)
{
    SPI1_CS_EN();
    spi1_sendbyte(0xB9);
    SPI1_CS_DIS();
    spi1_delay();
}

///解除power_down模式
AT(.text.spiflash1_drv)
void spiflash1_release_pd(void)
{
    SPI1_CS_EN();
    spi1_sendbyte(0xAB);
    SPI1_CS_DIS();
    spi1_delay();
}

AT(.text.spiflash1_drv)
bool is_exspiflash_online(void)
{
    #if INTERNAL_FLASH_REC
    return true;
    #endif
    u32 flash_id;
    u32 i = 5;
    while(i--) {
        WDT_CLR();
        flash_id = spiflash1_id_read();
        printf("flash id = 0x%X\n",flash_id);
        for(u16 k = 0; k < SPIFLASH_ID_NUM; k++){
            if(spiflash1_id_table[k] == flash_id){
                return true;
            }
        }
        printf("--->!!!Read Flash ID Err\n",flash_id);
        delay_ms(1);
    }
    return false;
}

#if SPIFLASH_MUSIC_BIN_WRITE_TEST
extern u8 const exspiflash_music_bin[36864];
void write_music_bin_to_spiflash(void)
{
    spiflash1_init();
    u8 rx_buf[256];
    int block_total,block_ok, block_fail, i;
    block_total = sizeof(exspiflash_music_bin)/256;
    for (i = 0; i < sizeof(exspiflash_music_bin); ) {
        spiflash1_erase(i);
        printf("erase addr = 0x%X\n",i);
        i += 4096;
    }
    printf("write exspiflash_music_bin,len = %d, block_total = %d\n", sizeof(exspiflash_music_bin) ,block_total);
    //write
    for (i = 0; i < block_total; i++) {
        WDT_CLR();
        spiflash1_write((void*)&exspiflash_music_bin[i*256], i*256, 256);
        printf("[0x%X]",i*256);
    }
    //read_back and compare
    printf("\nread_back and compare\n");
    block_ok = 0;
    block_fail = 0;
    for (i = 0; i < block_total; i++) {
        spiflash1_read(rx_buf,i*256,256);
        if (memcmp(rx_buf,&exspiflash_music_bin[i*256],256)) {
            printf("[fail:0x%X]",i*256);
            block_fail++;
        } else {
            printf("[ok:0x%X]",i*256);
            block_ok++;
        }
    }
    printf("block_ok = %d  block_fail = %d\n", block_ok, block_fail);
}
#endif  //SPIFLASH_MUSIC_BIN_WRITE_TEST

#endif //(EX_SPIFLASH_SUPPORT & EXSPI_REC)

#if SPIFLASH0_RW_EN

#define SPIFLASH_WRITE              0x02
#define SPIFLASH_READ               0x03
#define SPIFLASH_RDSSR              0x05
#define SPIFLASH_WREN               0x06
#define SPIFLASH_ERASE              0x20
#define SPIFLASH_RDID               0X9F
#define SPIFLASH_RMID               0X90
#define SPIFLASH_READ_FAST          0x0b
#define SPIFLASH_RUID               0x4B
#define SPIFLASH_Security_Write     0x42
#define SPIFLASH_Security_Erase     0x44
#define SPIFLASH_Security_Read      0x48
#define SPIFLASH_Write_Status       0x01

#define SPIFLASH0_BAUD              0x1
#define FLASH0_STADR                0
#define PAGE_LEN                    512
#define SPI_MUL_BIT_EN              0           //SPI multiple bit bus en

void spiflash0_init(void)
{
    //底层已经初始化过，不需要再初始化
}

void spiflash_sendbyte(u8 date)
{
    SPI0CON &= ~BIT(4);                             //TX
    SPI0BUF  = date;
    while (!(SPI0CON & BIT(16)));                   //Wait pending
}

u8 spiflash_getbyte(void)
{
    SPI0CON |= BIT(4);                              //RX
    SPI0BUF  = 0xff;
    while (!(SPI0CON & BIT(16)));                   //Wait pending
    return SPI0BUF;
}

void flashcs_dis(void)
{
    GPIOGSET = BIT(2);      //cs
}

void flashcs_en(void)
{
    GPIOGCLR = BIT(2);      //cs
}

void spiflash_write_en(void)
{
    flashcs_en();
    spiflash_sendbyte(SPIFLASH_WREN);        //write_en
    flashcs_dis();
    // delay(20);
    delay_us(20);
}

u8 spiflash_readssr(void)
{
    u8 ssr;
    u8 ssr1;
    flashcs_en();
    spiflash_sendbyte(SPIFLASH_RDSSR);        //read ssr
    ssr = spiflash_getbyte();
    ssr1 = spiflash_getbyte();
    flashcs_dis();
    printf("ssr00 = %X\n",ssr);
    printf("ssr01 = %X\n",ssr1);
    return ssr;
}


void spiflash0_sendaddr(u32 addr)
{
    spiflash_sendbyte(addr >> 16);
    spiflash_sendbyte(addr >> 8);
    spiflash_sendbyte(addr);
}

void spiflash0_id_read(void)
{
    u32 id = 0;
    flashcs_en();
    // delay(20);
    delay_us(20);
    spiflash_sendbyte(SPIFLASH_RDID);
    for (u8 i = 0; i < 4; i++) {
        id |= spiflash_getbyte();
        if(i<3){id<<=8;}
//        printf("ID = %X\n",SPI0BUF);
//        printf("ID = %X\n",id);
    }
     printf("ID = %X\n",id);
     flashcs_dis();
}

void spiflash_security_erase()
{
    spiflash_write_en();
    /////////////////
    flashcs_en();
    spiflash_sendbyte(SPIFLASH_Security_Erase);            //erase(4k)
    spiflash0_sendaddr(0x00);
    flashcs_dis();
    while(spiflash_readssr() & 0x01);   //wait_busy;
}

void spiflash_security_read(u8 *buf, u32 addr, u32 len)
{
//    //SPI0_G1: CLK_PG4, DO_PG5, DI_PG1, CS_PG2

    flashcs_en();
    spiflash_sendbyte(SPIFLASH_Security_Read);        //read_flash

    spiflash0_sendaddr(addr);
    spiflash_sendbyte(0x00);

    if(SPI_MUL_BIT_EN == 1){
        SPI0CON |= BIT(9);
    }

    delay_us(20);
    for (u32 i = 0; i < len; i++)
    {
       buf[i] = spiflash_getbyte();
    }

    if(SPI_MUL_BIT_EN == 1){
        SPI0CON &= ~BIT(9);
    }
    flashcs_dis();
}

void spiflash_security_write(u8 *buf, u32 addr, u32 len)
{
    spiflash_write_en();
    /////////////////
    flashcs_en();
    spiflash_sendbyte(SPIFLASH_Security_Write);        //program
    spiflash0_sendaddr(addr);

    SPI0CON &= ~BIT(4);
    SPI0DMAADR = ((u32)buf);
    SPI0DMACNT = len;
    while (!(SPI0CON & BIT(16)));                   //Wait pending
    flashcs_dis();
    while(spiflash_readssr() & 0x01);
}

void spiflash0_read_test(void)
{
    u8 data_test_read[2];
    u8 data_test[16] = {0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4a,0x4b,0x4c,0x4d,0x4e,0x4f};
    memset(data_test_read,0,2);
    // spiflash0_init();
    spiflash0_id_read();
    printf("1111\n");
    spiflash_security_erase();
    printf("2222\n");
    spiflash_security_write(data_test,0x00,16);
    printf("3333\n");
    spiflash_security_read(data_test_read,0x00,16);
    print_r(data_test_read,16);
}

///读取SPIFlash的UID
void spiflash_readuid(void *idbuf, uint param)
{
    u8 *buf = idbuf;
    flashcs_en();
//    SPI0CON &= ~BIT(4);
    spiflash_sendbyte(0x4B);
    while (param--) {
        spiflash_sendbyte(0x00);
    }
//    SPI0CON |= BIT(4);
    for(int i=0;i<16;i++) {
        buf[i] = spiflash_getbyte();
    }
    flashcs_dis();
}

//flash uid读取示例
void flash_uid_read_proc(void)
{
    uint8_t uid_data[16];	        // 芯片的UID数据
    //--- 读取芯片的UID数据
	spiflash_readuid(uid_data,4);
    print_r(uid_data,16);
}
#endif