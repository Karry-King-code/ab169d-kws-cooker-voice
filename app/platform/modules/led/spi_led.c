#include "include.h"
#include "spi_led.h"

#if RGB_SERIAL_EN
//LED level = led_encode_low*n + led_encode_mid*8*n + led_encode_high*4*8*n
AT(.rodata.spi_led)
const u8 led_encode_low[8] = {
    0x24, 0x26, 0x34, 0x36,
    0xA4, 0xA6, 0xB4, 0xB6,
};

AT(.rodata.spi_led)
const u8 led_encode_mid[4] = {
    0x49, 0x4D, 0x69, 0x6D,
};

AT(.rodata.spi_led)
const u8 led_encode_high[8] = {
    0x92, 0x93, 0x9A, 0x9B,
    0xD2, 0xD3, 0xDA, 0xDB,
};

/*
AT(.rodata.spi_led)
const u8 led_encode_table[9] = {
    //100 100 100 100       100 100 100 100     0Level
    0x92, 0x49, 0x24,
    //100 100 100 100       100 100 100 110     1Level
    0x92, 0x49, 0x26,
    //110 110 110 110       110 110 110 110     255Level
    0xDB, 0x6D, 0xB6,
};
*/

led_data_t led_data_0 AT(.spi_led_buf);

AT(.text.bsp.spi_led)
void spi1_led_init(void)
{
#if !RGB_SPI_CROSSBAR
#if   RGB_SPI_PORT == SPI1MAP_G1
    GPIOAFEN |=   BIT(4);
    GPIOADE  |=   BIT(4);
    GPIOADIR &=  ~BIT(4);           //DO output
    FUNCMCON1 = (0x0F<<12);
    FUNCMCON1 = SPI1MAP_G1;
#elif RGB_SPI_PORT == SPI1MAP_G2
    GPIOAFEN |=   BIT(9);
    GPIOADE  |=   BIT(9);
    GPIOADIR &=  ~BIT(9);          //DO output
    FUNCMCON1 = (0x0F<<12);
    FUNCMCON1 = SPI1MAP_G2;
#elif RGB_SPI_PORT == SPI1MAP_G3
    GPIOAFEN |=   BIT(13);
    GPIOADE  |=   BIT(13);
    GPIOADIR &=  ~BIT(13);           //DO output
    FUNCMCON1 = (0x0F<<12);
    FUNCMCON1 = SPI1MAP_G3;
#elif RGB_SPI_PORT == SPI1MAP_G4
    GPIOAFEN |=   BIT(2);
    GPIOADE  |=   BIT(2);
    GPIOADIR &=  ~BIT(2);           //DO output
    FUNCMCON1 = (0x0F<<12);
    FUNCMCON1 = SPI1MAP_G4;
#endif
#else
    GPIOAFEN |=   BIT(RGB_CROSSBAR_IO-1);
    GPIOADE  |=   BIT(RGB_CROSSBAR_IO-1);
    GPIOADIR &=  ~BIT(RGB_CROSSBAR_IO-1);

    FUNCMCON0   = (FUNCMCON0  & ~(0xF  << 12))| (6 << 12);
    FUNCOUTMCON = (FUNCOUTCON & ~(0x1F << 8)) | (RGB_CROSSBAR_IO << 8);
    FUNCOUTCON  = (FUNCOUTCON & ~(0xF  << 8)) | (FUNCO_SPI1_DO << 8);
#endif
    memset(&led_data_0, 0, sizeof(led_data_0));
    SPI1BAUD = get_sysclk_nhz()/SPI_LED_BAUD - 1;//SPI CLK
    SPI1CON = 0x01 | (1<<2);

    spi1_test_set();
}

AT(.text.bsp.spi_led)
static void spi1_dma_sendbytes(void *data, u32 len)
{
    // SPI1_CS_EN();
    SPI1DMAADR = DMA_ADR(data);
    SPI1DMACNT = len;
    // SPI1_CS_DIS();
}

//LED control level-> 0-255
AT(.text.bsp.spi_led)
void spi1_led_ctrl(led_id_t rgb_id, u8 rgb_level)
{
    u32 i = 0;
    u32 tmp_level, en_data;
    u8 *pmap, *pspi_data;
    pmap = led_data_0.led_map;
    pspi_data = led_data_0.led_data_buf;

    if(rgb_id >= LED_TOTAL_NUM){
        return;
    }

    *(pmap + rgb_id) = rgb_level;
    while(i < LED_MAP_BUF_SIZE){
        tmp_level = *(pmap + i);
        en_data = level_2_encode(tmp_level);

        *(pspi_data++) = (u8)(en_data >> 16);
        *(pspi_data++) = (u8)(en_data >> 8);
        *(pspi_data++) = (u8)en_data;
        i++;
    }
    set_led_chg(true);
}

AT(.text.bsp.spi_led)
u32 level_2_encode(u8 rgb_level)
{
    u8 hcnt, mcnt, lcnt;
    u32 rgb_encode = 0;
    hcnt = rgb_level/32;
    mcnt = rgb_level/8 - 4*hcnt;
    lcnt = rgb_level%8;
    rgb_encode = (led_encode_high[hcnt]<<16) + (led_encode_mid[mcnt]<<8) + led_encode_low[lcnt];
    // my_printf("hcnt, mcnt, lcnt: %d, %d, %d\n", hcnt, mcnt, lcnt);
    return rgb_encode;
}

AT(.text.bsp.spi_led)
void set_led_chg(bool boset)
{
    if(boset != led_data_0.led_chg_flag){
        led_data_0.led_chg_flag = boset;
    }
}

AT(.text.bsp.spi_led)
bool get_led_chg(void)
{
    return led_data_0.led_chg_flag;
}

AT(.text.bsp.spi_led)
void spi1_test_set(void)
{
    #if 1   //test
    spi1_led_ctrl(LED_G_01, 50);
    spi1_led_ctrl(LED_R_01, 50);
    spi1_led_ctrl(LED_B_01, 50);
    // my_printf("led_data_buf:\n");
    // my_print_r(led_data_0.led_data_buf, LED_DATA_BUF_SIZE);
    #endif
}

AT(.text.bsp.spi_led)
void spi1_led_data_send_proc(void)
{
    if(get_led_chg()){
        set_led_chg(false);
        spi1_dma_sendbytes(led_data_0.led_data_buf, LED_DATA_BUF_SIZE);
    }
}
#endif
