#include "include.h"

xcfg_cb_t xcfg_cb;

uint8_t cfg_spiflash_speed_up_en = SPIFLASH_SPEED_UP_EN;

extern uint8_t pin_sop_sel;

void sd_detect(void);
void tbox_uart_isr(void);
void testbox_init(void);
bool exspiflash_init(void);
void ledseg_6c6s_clr(void);
void bsp_stretch_init(void);
u32 asr_alg_process(short *ptr);
void tkey_init(void);
void bsp_irtx_init(u8 method);
bool drc_v3_init(const void *bin, int bin_size);
void ir_rx_110ms_isr(void);

AT(.text.bsp.sys)
void bsp_clr_mute_sta(void)
{
    if (sys_cb.mute) {
        sys_cb.mute = 0;
// #if DAC_DNR_EN
//         dac_dnr_set_sta(sys_cb.dnr_sta);
// #endif
    }
}

AT(.text.bsp.sys)
bool bsp_get_mute_sta(void)
{
    return sys_cb.mute;
}

#if MUSIC_SDCARD_EN
AT(.com_text.detect)
void sd_detect(void)
{
    if ((!is_sd_support()) || (IS_DET_SD_BUSY())) {
        return;
    }
    if (SD_IS_ONLINE()) {
        if (dev_online_filter(DEV_SDCARD)) {
            sd_insert();
            msg_enqueue(EVT_SD_INSERT);
//            printf("sd insert\n");
        }
    } else {
        if (dev_offline_filter(DEV_SDCARD)) {
            sd_remove();
            msg_enqueue(EVT_SD_REMOVE);
//            printf("sd remove\n");
        }
    }
}
#endif // MUSIC_SDCARD_EN

void sys_ram_info_dump(void)
{
    extern u32 __comm_size;
    extern u32 __bss_size;

    extern u32 __data_vma;
    extern u32 __data_ram_size;
    extern u32 __comm_vma;
    extern u32 __comm_ram_size;

    u32 comm_free  = (u32)&__comm_ram_size - (u32)&__comm_size;
    u32 data_free = (u32)&__data_ram_size - (u32)&__bss_size;
    u32 ram_total = comm_free + data_free;

    printf("ram_info:\n");
    printf("data:  remain:%6d(Byte), total:%6d(Byte), range:[0x%x - 0x%x]\n", data_free,  (u32)&__data_ram_size,  (u32)&__data_vma, (u32)&__data_vma + (u32)&__data_ram_size);
    printf("comm:  remain:%6d(Byte), total:%6d(Byte), range:[0x%x - 0x%x]\n", comm_free, (u32)&__comm_ram_size, (u32)&__comm_vma, (u32)&__comm_vma + (u32)&__comm_ram_size);
    printf("remain_total: %dK\n",ram_total / 1024);
}

AT(.text.bsp.sys.init)
static void rtc_32k_configure(void)
{
    u32 temp = RTCCON0;
    temp &= ~BIT(6);                            //use RTC internal 32K osc
    temp |=  BIT(2);                            //RTC output clock to core enable
    temp = (temp & ~(0x3 << 8)) | BIT(8);       //CLK2M in RTC power domain source select RC2M_RTC
#if ALARM_WAKEUP_EN
    temp |= BIT(0);                             //RC enable
#endif
    RTCCON0 = temp;
}

AT(.text.bsp.sys.init)
bool rtc_init(void)
{
    u32 temp;
    rtc_32k_configure();
    sys_cb.rtc_first_pwron = 0;
    temp = RTCCON0;
    if (temp & BIT(7)) {
        temp &= ~BIT(7);                        //clear first poweron flag
        RTCCON0 = temp;
        sys_cb.rtc_first_pwron = 1;
        printf("rtc 1st pwrup\n");
        return false;
    }
    return true;
}

//UART0打印信息输出GPIO选择，UART0默认G1(PA7)
void uart0_mapping_sel(void)
{
    //等待uart0发送完成
    if(UDETCON & BIT(0)) {
        while (!(UDETCON & BIT(8)));
    }
    GPIOADE  &= ~BIT(2);
    GPIOAFEN &= ~BIT(2);
    GPIOAPU  &= ~BIT(2);
    FUNCMCON0 = (0xf << 12) | (0xf << 8);           //clear uart0 mapping

#if (UART0_PRINTF_SEL == PRINTF_PA3)
    GPIOADE  |= BIT(3);
    GPIOAPU  |= BIT(3);
    GPIOADIR |= BIT(3);
    GPIOAFEN |= BIT(3);
    FUNCMCON0 = URX0MAP_TX | UTX0MAP_PA3;
#elif (UART0_PRINTF_SEL == PRINTF_PA8)
    GPIOADE  |= BIT(8);
    GPIOAPU  |= BIT(8);
    GPIOADIR |= BIT(8);
    GPIOAFEN |= BIT(8);
    FUNCMCON0 = URX0MAP_TX | UTX0MAP_PA8;
#elif (UART0_PRINTF_SEL == PRINTF_PA2)
    GPIOADE  |= BIT(2);
    GPIOAPU  |= BIT(2);
    GPIOADIR |= BIT(2);
    GPIOAFEN |= BIT(2);
    FUNCMCON0 = URX0MAP_TX | UTX0MAP_PA2;
#elif (UART0_PRINTF_SEL == PRINTF_PA12)
    GPIOADE  |= BIT(12);
    GPIOAPU  |= BIT(12);
    GPIOADIR |= BIT(12);
    GPIOAFEN |= BIT(12);
    FUNCMCON0 = URX0MAP_TX | UTX0MAP_PA12;
#elif (UART0_PRINTF_SEL == PRINTF_PA14)
    GPIOADE  |= BIT(14);
    GPIOAPU  |= BIT(14);
    GPIOADIR |= BIT(14);
    GPIOAFEN |= BIT(14);
    FUNCMCON0 = URX0MAP_TX | UTX0MAP_PA14;
#endif
#if HUART_EN && HUART_PRINTF_EN
    bsp_huart_init();
    HSUT0CON &= ~BIT(0);                    //DIS RX_EN
    u_huart_tx_set_dir();
    my_printf_init(bsp_huart_putchar_byte); //将所有打印printf映射到huart
#endif
}

AT(.text.bsp.sys.init)
void bsp_compiler(void)
{

}

void mclr_l_init(void)
{
    CLKGAT0 |=  BIT(15);                    //lp clken
    RTCCON0 |=  BIT(0);                     //RC enable
    WKUPCON  =  BIT(29) | ((GPIO_10S_SEL-1) << 24);      //选中IO引脚 这里选中PA10
//    RTCCON3 |=  BIT(3);                     //切换到vlcore供电
//    RTCCON3 &= ~BIT(5);

    RTCCON12 |=  (3 << 2);                  //rtc wdt disable
    RTCCON12 |=  (3 << 6);                  //PA11 reset disable
    RTCCON12 &= ~(3 << 4);                  //GPIO 10s reset disable
}

//系统启动最初的位置
void platform_main_insert(void)
{
#if SYS_BRAM_EN
    #if (SYS_BRAM_LEN == 0x1000)
    ICLOCK = 0xff;          //cache预留4K空间用作bram
    #elif (SYS_BRAM_LEN == 0x2000)
    ICLOCK = 0xffff;        //cache预留8K空间用作bram
    #endif
#endif

    if(GPIO_10S_RESET) {  //10S复位和mclr复位不要同时开，会有冲突
        GPIOADE  |= BIT(GPIO_10S_SEL-1);
        GPIOAPU  |= BIT(GPIO_10S_SEL-1);
        GPIOADIR |= BIT(GPIO_10S_SEL-1);
        if(RTCCON10 & BIT(13)) {
            RTCCON12 |= (3 << 4);                   //GPIO 10s reset disable
            RTCCON10 |= BIT(13);                    //clear pnd
            CRSTPND  |= BIT(19);

            mclr_l_init();                          //10s reset init
        } else {
            mclr_l_init();                          //10s reset init
        }
    } else {
        RTCCON12 |=  ((3 << 4) | (3 << 6));
    }
    if(MCLR_S_RESET) {
        GPIOADE  |= BIT(11);
        GPIOAPU  |= BIT(11);
        GPIOADIR |= BIT(11);
        if(RTCCON10 & BIT(9)) {
            RTCCON10 |= BIT(9);                     //clear pnd
        } else {
//            RTCCON3  |=  BIT(3);                    //切换到vlcore供电
//            RTCCON3  &= ~BIT(5);
            RTCCON12 |=  (3 << 6);                  //rtc wdt disable
            RTCCON12 |=  (3 << 4);                  //GPIO 10s reset disable
            RTCCON12 &= ~(3 << 2);                  //PA11 reset enable
        }
    } else {
        RTCCON12 |=  ((3 << 2) | (3 << 6));
    }
}
