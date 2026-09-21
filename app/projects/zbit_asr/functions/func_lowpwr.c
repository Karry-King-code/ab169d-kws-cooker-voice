#include "include.h"
#include "func.h"

#define TRACE_EN                0
#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#define TRACE_R(...)            print_r(__VA_ARGS__)
#else
#define TRACE(...)
#define TRACE_R(...)
#endif // TRACE_EN

extern u32 __comm_vma, __comm_lma, __comm_size;
extern u32 __pdn_comm_vma, __pdn_comm_lma, __pdn_comm_size;
extern u32 rc2m_freq_cal;

bool power_off_check(void);
void lock_code_pwrsave(void);
void unlock_code_pwrsave(void);
void lp_sleep(void (*sleep_cb)(void), void (*wakeup_cb)(void));
void boot_uart_init(void);
void spiflash_init(u8 sf_read, u8 dummy);
void timer0_init(void);
void sdadc_analog_mic_exit(u8 channel);

#define VRTC_SEL                4       //All Mode: VRTC档位
#define VDDIO_AON_SEL           2       //All Mode: 关机后VDDIO电压档位,0-2.4V, 1-2.8V, 2-3.1V, 3-3.3V
#define RI_LVCORE_SEL           6       //L3M模式: 档位6不要改动关机后保持mic bias电压使用
#define SPIFLASH_DP_PDN4_EN     0       //L4M模式: flash深度睡眠使能宏,能降低一些功耗
#define PWR_RTC1S_WK_MODE       1       //RTC1S模式: 1-wakeup form boot, 0-wakeup next address
#define PWR_RTC1S_WK_TIME       0xB7D7  //RTC1S模式: CLK来源-->rtc_rc2m/64
#define PWR_ALARM_WK_TIME       10      //ALARM模式: 唤醒时间设置,单位:秒

#define SPI_CS1_PDIR            GPIOGDIR
#define SPI_CS1_PORTSET         GPIOGSET
#define SPI_CS1_PORTCLR         GPIOGCLR
#define SPI_CS1                 2
#define SPI_CS1_EN()            SPI_CS1_PORTCLR = BIT(SPI_CS1)
#define SPI_CS1_DIS()           SPI_CS1_PORTSET = BIT(SPI_CS1)

#define SPIFLASH_DP             0xB9
#define SPIFLASH_RDP            0xAB

pwroff_wk_t wk_cfg AT(.pdncom_buf.wakeup);

#if (POWER_DOWN_MODE == PWR_L4M) || (POWER_DOWN_MODE == PWR_L4M_ALM) || (!SOFT_POWER_ON_OFF) || (POWER_DOWN_MODE == PWR_RTC1S) || MIX_PWR_DOWN_EN
static sleep_sfr_t sp_sfr AT(.pdncom_buf.sleep);
#endif

//软开机唤醒源打印输出
u32 lowpwr_get_wakeup_source(void)
{
    u32 reason = RTCCON9;
    if(RTCCON9){
        if(RTCCON9 & WK_PWROFF_BGUV){
            printf("BGUV wakeup\n");
        }
        if(RTCCON9 & WK_PWROFF_GPIO){
            printf("GPIO wakeup\n");
        }
        if(RTCCON9 & WK_PWROFF_RTC1S){
            printf("RTC 1s wakeup\n");
        }
        if(RTCCON9 & WK_PWROFF_ALARM){
            printf("alarm wakeup\n");
        }
    }
    return reason;
}

//软关机PMU电源配置
AT(.pdncom_text.pmu)
void pwroff_pmu_voltage_config(void)
{
    //配置关机下，VRTC数字的工作电压；RTCCON BIT0为0则选择RTCCON1 BIT8:6; 为1则选择BIT 11:9
    RTCCON1 = (RTCCON1 & ~(7<<6)) | (VRTC_SEL<<6) | (7 << 9);

    //VDDIO LDO从正常LDO切换到VDDIO_AON
    PWRCON0 = (PWRCON0 & ~(0x1f<<5)) | (0x13<<5);       //VDDIO=2.925v,正常运行时的vddio稍微降低点再切换来源
    RTCCON1 = (RTCCON1 & ~(3<<2)) | (VDDIO_AON_SEL<<2); //VDDIO_AON/V,关机后的vddio电压
    RTCCON3 |= BIT(7);                                  //VDDIO_AON enable
}

//软关机,关机前RTC配置
AT(.pdncom_text.rtc)
void pwroff_rtc_core_config(pwroff_wk_t *wkcfg)
{
    if(MCLR_S_RESET){
        RTCCON12 |= 0xf3;       //disable GPIO reset
        RTCCON12 &= ~0xc;       //PB10 reset
    }else{
        RTCCON12 |= 0xff;       //disable GPIO reset & PB10 reset
    }
}

//红外关机进入sleep mode  需要开启freqdetRC2M的频率
ALIGNED(512)
AT(.text.lowpwr.sleep)
void func_sleepmode(void)
{
    printf("%s\n", __func__);
#if (IRRX_SW_EN && FREQ_DET_EN)
    u16 gpioa_fen, gpiog_fen;
    u16 gpioa_dir, gpiog_dir;
    u16 gpioa_de, gpiog_de;
    u16 gpioa_drv, gpiog_drv;
    u16 gpioa_pu, gpiog_pu;
    u16 gpioa_pd, gpiog_pd;

    //u16 adc_ch;
    u8 sys_clk;
    u32 cpu_ie;
    u32 sddaccon;
    u32 clkdivcon0;
    u32 pll0con;

    //PLL0CON = ~BIT(18);                //pll10 sdm enable disable

    //io save
    gpioa_fen = GPIOAFEN;
    gpiog_fen = GPIOGFEN;
    gpioa_dir = GPIOADIR;
    gpiog_dir = GPIOGDIR;
    gpioa_de  = GPIOADE;
    gpiog_de  = GPIOGDE;
    gpioa_drv = GPIOADRV;
    gpiog_drv = GPIOGDRV;
    gpioa_pu  = GPIOAPU;
    gpiog_pu  = GPIOGPU;
    gpioa_pd  = GPIOAPD;
    gpiog_pd  = GPIOGPD;

    GPIOAFEN = 0;
    GPIOADIR = 0;
    GPIOADE = 0;
    GPIOADRV = 0;
    GPIOAPU = 0;
    GPIOAPD = 0;

    GPIOGDE = 0x3F;     //MCP FLASH
    // GPIOGFEN = 0;
    // GPIOGDIR = 0;
    // GPIOGDRV = 0;
    // GPIOGPU = 0;
    // GPIOGPD = 0;
    // wakeup_disable();

#if IRRX_SW_EN
    IR_CAPTURE_PORT();
#endif // IRRX_SW_EN
//
    CLKGAT0 &= ~(BIT(31) | BIT(29) | BIT(30) | BIT(25) | BIT(18) | BIT(16) | BIT(14) | BIT(13) | BIT(11) | BIT(10) | BIT(9) | BIT(6) | BIT(5) | BIT(4));
    CLKGAT1 &= ~(BIT(30) | BIT(28) | BIT(27) | BIT(17)| BIT(15) | BIT(14) | BIT(13)  |  BIT(8) | BIT(4) | BIT(2) | BIT(0));

    cpu_ie = PICCON & BIT(0);
//    PICCONCLR = BIT(0);     //关总中断

    gui_display(DISP_OFF);
#if DAC_DNR_EN
    u8 sta = dac_dnr_get_sta();
    dac_dnr_set_sta(0);
#endif

    sddaccon = SDDACCON;

    sys_clk = get_cur_sysclk();
    set_sys_clk(SYS_RC2M);
    TMR0PR  = (rc2m_freq_cal/2) - 1;                        //timer 1ms
	TMR0CON = (4 << 1) | (0 << 4) | BIT(7) | BIT(0);    //选择rc2m时钟源

    TMR1CON = (0 << 1) | (3 << 4) | (2 << 14) | BIT(7) | BIT(0);
    sys_set_tmr_enable(0);
    //PWRCON0 = (PWRCON0 & ~(0x1f<<0)) | (0x10<<0);           //DI_VCORES 0x10 = 1.2
    dac_power_off();                    //dac power down
    CLKGAT0 &= ~BIT(12);
    //sdadc_analog_mic_exit(CH_MIC_PA0);
    saradc_exit(adc_cb.channel);        //close saradc及相关通路模拟
    led_off();
    rled_off();

#if 0
    PLL0CON &= ~BIT(18);                //pll10 sdm enable disable
    // PLL0CON &= ~BIT(6);                 //pll0 analog diable
    // PLL0CON &= ~BIT(12);                //pll0 ldo disable

    adc_ch = adc_cb.channel;
    adc_cb.channel = ADCCH_VBAT;

#endif // 0
    WDT_DIS();

    clkdivcon0 = CLKDIVCON0;
    pll0con = PLL0CON;
    PLL0CON &= ~(BIT(15)|BIT(16));
    CLKDIVCON0 = 0;

    while(1) {
        asm("nop");asm("nop");asm("nop");
        if(get_irkey() == KEY_IR_POWER) {
            ir_key_clr();
            break;
        }
    }

    PLL0CON = pll0con;
    CLKDIVCON0 = clkdivcon0;

    WDT_EN();
    PICCON |= cpu_ie;

    GPIOAFEN = gpioa_fen;
    GPIOGFEN = gpiog_fen;
    GPIOADIR = gpioa_dir;
    GPIOGDIR = gpiog_dir;
    GPIOADE  = gpioa_de;
    GPIOGDE  = gpiog_de;
    GPIOADRV = gpioa_drv;
    GPIOGDRV = gpiog_drv;
    GPIOAPU  = gpioa_pu;
    GPIOGPU  = gpiog_pu;
    GPIOAPD  = gpioa_pd;
    GPIOGPD  = gpiog_pd;

    //PWRCON0 = (PWRCON0 & ~(0x1f<<0)) | (0x10<<0);           //DI_VCORES 0x10 = 1.2

    CLKGAT0 |= (BIT(31) | BIT(29) | BIT(30) | BIT(25) | BIT(18) | BIT(16) | BIT(14) | BIT(13) | BIT(11) | BIT(10) | BIT(9) | BIT(6) | BIT(5) | BIT(4));
    CLKGAT1 |= (BIT(30) | BIT(28) | BIT(27) | BIT(17)| BIT(15) | BIT(14) | BIT(13)  |  BIT(8) | BIT(4) | BIT(2) | BIT(0));

    TMR0PR  = 3000 - 1;                                 //timer 1ms
	TMR0CON = (2 << 1) | (3 << 4) | BIT(7) | BIT(0);    //timer0 clk = xosc24m / 8
    set_sys_clk(sys_clk);              //还原系统时钟
    TMR1CON = (2 << 1) | (3 << 4) | (2 << 14) | BIT(7) | BIT(0);
    sys_set_tmr_enable(1);

    //adc_cb.channel = adc_ch;
    //dac_restart();
    //dac_power_on(DAC_DRV_SEL);

#if DAC_DNR_EN
    dac_dnr_set_sta(sta);
#endif
    CLKGAT0 |= BIT(12);
    SDDACCON = sddaccon;
    bsp_saradc_init(adc_cb.channel);

     //gui_init();
    gui_display(DISP_POWERON);

#if 0
    delay_us(100);
    PLL0CON |= BIT(18);                 //PLL1 SDM enable
    // PLL0CON |= BIT(6);                 //PLL1 Analog enable
    // wakeup_disable();
    //enable adpll
    // PLL0CON |= BIT(12);                //PLL1 LDO enable
    dac_restart();
    bsp_change_volume(sys_cb.vol);
#endif // 0
#endif // IRRX_SW_EN && FREQ_DET_EN
    bsp_change_volume(sys_cb.vol);
    func_cb.sta = func_cb.last;
    func_cb.last = FUNC_NULL;
}

#if SPIFLASH_DP_PDN4_EN
AT(.pdncom_text.lp.sleep)
void spiflash_sendbyte_pdn4(u8 date)
{
    SPI0CON &= ~BIT(4);                             //TX
    SPI0BUF  = date;
    while (!(SPI0CON & BIT(16)));                   //Wait pending
}

AT(.pdncom_text.lp.sleep)
void flashcs_dis_pdn4(void)
{
    GPIOGSET = BIT(2);      //cs
}

AT(.pdncom_text.lp.sleep)
void flashcs_en_pdn4(void)
{
    GPIOGCLR = BIT(2);      //cs
}

AT(.pdncom_text.lp.sleep)
void spiflash0_enter_dp(void)
{
    flashcs_en_pdn4();
    spiflash_sendbyte_pdn4(SPIFLASH_DP);     //enter deep power down
    flashcs_dis_pdn4();
    delay_us(20);
}

AT(.pdncom_text.lp.sleep)
void spiflash0_release_dp(void)
{
    flashcs_en_pdn4();
    spiflash_sendbyte_pdn4(SPIFLASH_RDP);    //release deep power down
    flashcs_dis_pdn4();
    delay_us(20);
}
#endif // SPIFLASH_DP_PDN4_EN

#if (PWR_L3M == POWER_DOWN_MODE) || (PWR_L3M_ALM == POWER_DOWN_MODE)
AT(.com_text.pwroff)//该函数不可放在cache ram上
void pwroff_save_to_cache(u8 *src_addr, u32 length)
{
    //需要把RAM0的程序copy到ICACHE RAM上面跑
    u32 i;
    u8 *cp_dst_ptr, *cp_src_ptr;
    cp_src_ptr = (u8*) src_addr;
    cp_dst_ptr = (u8*) 0x71000;
    for(i = 0; i < length; i++){
        *cp_dst_ptr++ = *cp_src_ptr++;
    }
    asm("j 0x71000");
    while(1);
}
#endif

#if (POWER_DOWN_MODE == PWR_RTC1S)
AT(.text.lowpwr.pdn)
void pdn_rtc_sta_save(void)
{
    sp_sfr.piccon   = PICCON;
    sp_sfr.picen    = PICEN;
    sp_sfr.picpr    = PICPR;
    sp_sfr.nmicon   = NMICON;
    sp_sfr.clkgat0  = CLKGAT0;
    sp_sfr.clkgat1  = CLKGAT1;

    sp_sfr.pwrcon0  = PWRCON0;
    sp_sfr.pwrcon1  = PWRCON1;

    sp_sfr.pll0con  = PLL0CON;
    sp_sfr.pll0div  = PLL0DIV;

    sp_sfr.clkcon0  = CLKCON0;
    sp_sfr.clkcon1  = CLKCON1;
    sp_sfr.clkdivcon0 = CLKDIVCON0;

    sp_sfr.funcoutmcon  = FUNCOUTMCON;
    sp_sfr.funcoutcon   = FUNCOUTCON;
    sp_sfr.funcincon    = FUNCINCON;
    sp_sfr.funcomcon[0] = FUNCMCON0;
    sp_sfr.funcomcon[1] = FUNCMCON1;

    sp_sfr.udetbaud = UDETBAUD;
    sp_sfr.udetcon  = UDETCON;
    sp_sfr.adc_ch   = adc_cb.channel;

    sp_sfr.gpioa_fen = GPIOAFEN;
    sp_sfr.gpiog_fen = GPIOGFEN;
    sp_sfr.gpioa_dir = GPIOADIR;
    sp_sfr.gpiog_dir = GPIOGDIR;
    sp_sfr.gpioa_de  = GPIOADE;
    sp_sfr.gpiog_de  = GPIOGDE;
    sp_sfr.gpioa_drv = GPIOADRV;
    sp_sfr.gpiog_drv = GPIOGDRV;
    sp_sfr.gpioa_pu  = GPIOAPU;
    sp_sfr.gpiog_pu  = GPIOGPU;
    sp_sfr.gpioa_pd  = GPIOAPD;
    sp_sfr.gpiog_pd  = GPIOGPD;
#if 1   //使能会导致mic bias抖动一下
    dac_power_off();                                //dac power down
    sdadc_analog_mic_exit(CH_MIC_PA0);
    saradc_exit(adc_cb.channel);                    //close saradc及相关通路模拟
#endif
}

AT(.text.lowpwr.pdn)
void pdn_rtc_sta_restore(void)
{
    PWRCON0     = sp_sfr.pwrcon0;

    PWRCON1 &= ~(0xf << 0);                         //软开机时，关掉flash power gate
    PWRCON1    |= BIT(4);                           //VDDF pulldown
    asm("nop"); asm("nop"); asm("nop"); asm("nop"); //至少4us
    asm("nop"); asm("nop"); asm("nop"); asm("nop");
    asm("nop"); asm("nop"); asm("nop"); asm("nop");
    asm("nop"); asm("nop"); asm("nop"); asm("nop");
    PWRCON1 &= ~BIT(4);                             //flash power gate pull down disable
    PWRCON1 |= BIT(0) | BIT(1) | BIT(2);
    asm("nop"); asm("nop"); asm("nop"); asm("nop");
    PWRCON1 |= BIT(3);

    //第一时间拉高MCP Flash的CS脚，以免CLK翻转影响SPIFlash
    SPI_CS1_DIS();
    SPI_CS1_PDIR &= ~BIT(SPI_CS1);

    CLKGAT0     = sp_sfr.clkgat0;
    CLKGAT1     = sp_sfr.clkgat1;

    PLL0DIV     = sp_sfr.pll0div;
    PLL0CON     = sp_sfr.pll0con;
    PLL0CON    |= BIT(20);                          //update pll0div to pll_clk
    PLL0CON    |= BIT(18);                          //pll sdm enable

    GPIOAFEN    = sp_sfr.gpioa_fen;
    GPIOADIR    = sp_sfr.gpioa_dir;
    GPIOADE     = sp_sfr.gpioa_de;
    GPIOADRV    = sp_sfr.gpioa_drv;
    GPIOAPU     = sp_sfr.gpioa_pu;
    GPIOAPD     = sp_sfr.gpioa_pd;

    GPIOGFEN    = sp_sfr.gpiog_fen;
    GPIOGDIR    = sp_sfr.gpiog_dir;
    GPIOGDE     = sp_sfr.gpiog_de;
    GPIOGDRV    = sp_sfr.gpiog_drv;
    GPIOGPU     = sp_sfr.gpiog_pu;
    GPIOGPD     = sp_sfr.gpiog_pd;

    CLKDIVCON0  = sp_sfr.clkdivcon0;
    CLKCON1     = sp_sfr.clkcon1;
    asm("nop");
    asm("nop");
    asm("nop");
    CLKCON0     = (sp_sfr.clkcon0 & (~0x0c));
    asm("nop");
    asm("nop");
    asm("nop");
    CLKCON0 = sp_sfr.clkcon0;

    LVDCON = (LVDCON & ~0x07) | 1;                  //LVD=2.2V,  LVDCON[2:0]
    PICADR = (u32)&__comm_vma;
#if SYS_BRAM_EN
    #if (SYS_BRAM_LEN == 0x1000)
    ICLOCK = 0xff;          //cache预留4K空间用作bram
    #elif (SYS_BRAM_LEN == 0x2000)
    ICLOCK = 0xffff;        //cache预留8K空间用作bram
    #endif
#endif
    CACHCON0 = BIT(16) | BIT(0);                    //dCache Enable, iCache Enable

    EFCON0 |= BIT(3) | BIT(2);                      //EF WP
    MEMCON |= BIT(16);                              //MROM Protect

    SPI0BAUD = 1;                                   //RC CLK / (1 + 1) = 1M
    SPI0CON = BIT(0);                               //SPI EN

    FUNCOUTMCON = sp_sfr.funcoutmcon;
    FUNCOUTCON  = sp_sfr.funcoutcon;
    FUNCINCON   = sp_sfr.funcincon;
    FUNCMCON0   = sp_sfr.funcomcon[0];
    FUNCMCON1   = sp_sfr.funcomcon[1];

    UDETBAUD    = sp_sfr.udetbaud;
    UDETCON     = sp_sfr.udetcon;
    UDETCPND = BIT(10) | BIT(15) | BIT(18);          //CLR Pending
    UDETCON = (UDETCON & ~(0xfff << 16)) | (0x5aa << 16);  //KEY MODE RESET

    PICCON      = sp_sfr.piccon;
    PICPR       = sp_sfr.picpr;
    PICEN       = sp_sfr.picen;
    NMICON      = sp_sfr.nmicon;

    spiflash_init(0x0b, 1);
    timer0_init();

    adc_cb.channel = sp_sfr.adc_ch;
    bsp_saradc_init(adc_cb.channel);
    sys_set_tmr_enable(1);
}
#endif

#if ((PWR_L1M == POWER_DOWN_MODE) || (PWR_L2M == POWER_DOWN_MODE))
void pdn_port_state(void)
{
    //PA11配置为复位引脚，使能上拉电阻
    //关机下不需要用到的IO，都配置为模拟IO，避免漏电
    GPIOADE  = BIT(11);
    GPIOGDE  = 0x0;
    GPIOAFEN = 0x0;
    GPIOGFEN = 0x0;

    GPIOGPU  = 0;
    GPIOAPU  = BIT(11);
#if 0
    CLKGAT0 &= ~BIT(15);    //disable lp_top clock
    RTCCON0 &= ~BIT(26);    //IO interface disable, io控制寄存器无法写
#else
    CLKGAT0 |= BIT(15);      //enable lp_top clock
    RTCCON0 |= BIT(25);     // enable lpsw_rstn
    RTCCON0 |= BIT(26);     //IO interface enable, io控制寄存器可以写
#endif
}
#endif

#if(PWR_L1M == POWER_DOWN_MODE)
void pdn1(void)
{
#ifdef SNF_RC_EN
    RTCCON0 &= ~(0xff<<8);
    RTCCON0 |= BIT(0);     //enable SNIFF RC
#endif // SNF_RC_EN
    printf("%s poweroff\n", __func__);
    set_all_pin_off();
    // printf("pdn1 test, RTCCON0: %x, RTCCON3: %x, RTCCON4: %x, RTCCON12: %x\n", RTCCON0, RTCCON3, RTCCON4, RTCCON12);
    // printf("GPIOAPU: %x, GPIOBPU: %x, GPIOGPU: %x\n", GPIOAPU, GPIOBPU, GPIOGPU);

    pwroff_pmu_voltage_config();
    pdn_port_state();

    RTCCON12 |= 0x3<<4;     //disable GPIO reset

    RTCCON &= ~BIT(0);      //关机选择RTCCON1 BIT8~6

    RTCCON3  = //BIT(19)    //pd_core3
              BIT(7)        //en vddio_aon
            | BIT(6)        //pd_core
            | BIT(4)        //pd_core2
            //| BIT(3)      //rl2vlc
            ;

    LPMCON |= BIT(0);                           //sleep mode
    LPMCON |= BIT(1);                           //idle mode
    while(1);
}
#endif

#if(PWR_L2M == POWER_DOWN_MODE)
void pdn2(void)
{
    printf("%s poweroff\n", __func__);
    set_all_pin_off();
    PICCONCLR = BIT(0);     //关总中断
    WDT_DIS();
#ifdef SNF_RC_EN
    RTCCON0 &= ~(0xff<<8);
    RTCCON0 |= BIT(0);     //enable SNIFF RC
#endif // SNF_RC_EN

    RTCCON3 &= ~BIT(6);     // dis PD_CORE
    // RTCCON3 |= (1<<2);      // sw vddcore enable
    CLKGAT0 |= (1<<15);     // LP en clock

    // printf("pdn2 down\n");
    RTCCON9 = 0xffffffff;   //clear pending
    RTCCON10 = 0xffffffff;

    // printf("RTCCON0: %x, RTCCON3: %x, RTCCON4: %x, RTCCON12: %x\n", RTCCON0, RTCCON3, RTCCON4, RTCCON12);
    // printf("GPIOAPU: %x, GPIOBPU: %x, GPIOGPU: %x\n", GPIOAPU, GPIOBPU, GPIOGPU);

    pwroff_pmu_voltage_config();

    //唤醒源选择 RTC1S RWDT ALM WKO PORT
    memset(&wk_cfg, 0, sizeof(pwroff_wk_t));
    wk_cfg.source = WK_PWROFF_GPIO;
    if(POWER_ON_FALL_NUM != 0){
        u8 fall_io_temp[POWER_ON_FALL_NUM] = POWER_ON_FALL_IO;
        wk_cfg.wk_fall_en = 1;
        wk_cfg.fall_io_num = POWER_ON_FALL_NUM;
        memcpy(wk_cfg.fall_io_group, fall_io_temp, POWER_ON_FALL_NUM);
    }
    if(POWER_ON_RISE_NUM != 0){
        u8 rise_io_temp[POWER_ON_RISE_NUM] = POWER_ON_RISE_IO;
        wk_cfg.wk_rise_en = 1;
        wk_cfg.rise_io_num = POWER_ON_RISE_NUM;
        memcpy(wk_cfg.rise_io_group, rise_io_temp, POWER_ON_RISE_NUM);
    }
    pwroff_wk_source_config(&wk_cfg);
    RTCCON0 |=  BIT(31);        //power_flag
    CLKGAT0 &= ~BIT(15);        //disable lp_top clock
    RTCCON0 &= ~BIT(26);        //IO interface disable, io控制寄存器无法写
    if(MCLR_S_RESET){
        RTCCON12 |= 0xf3;       //disable GPIO reset
        RTCCON12 &= ~0xc;       //PA11 reset
    }else{
        RTCCON12 |= 0xff;       //disable GPIO reset & PA11 reset
    }

    RTCCON &= ~BIT(0);      //关机选择RTCCON1 BIT8~6
    RTCCON3 = //BIT(19)     //pd_core3
              BIT(7)        //en vddio_aon
            //| BIT(6)      //pd_core
            | BIT(4)        //pd_core2
            | BIT(3)        //rl2vlc
            | BIT(17)       //port wakeup enable
            ;

    LPMCON |= BIT(0);                           //sleep mode
    LPMCON |= BIT(1);                           //idle mode
    while(1);
}
#endif

#if(PWR_L3M == POWER_DOWN_MODE)
AT(.com_text.pdn_shut)
void pdn3_shutdown(void)
{
    CLKGAT0 &= ~BIT(2);     //disable ram0 clock
    RTCCON0 &= ~BIT(27);    //sram0 interface disable

    LPMCON |= BIT(0);       //sleep mode
    LPMCON |= BIT(1);       //idle mode
    while(1);
}

AT(.text.lowpwr.pdn)
void pdn3_do(void)
{
    u32 rtccon3;
    //唤醒源选择 RTC1S RWDT ALM WKO PORT
    memset(&wk_cfg, 0, sizeof(pwroff_wk_t));
    wk_cfg.pdn_io_interface_flag = 1;
    wk_cfg.lp_type = LP_DEEP_SLEEP;
    wk_cfg.source  = WK_PWROFF_GPIO;
#if MIX_PWR_DOWN_EN
        wk_cfg.wk_rise_en = 1;
        wk_cfg.rise_io_num = 1;
        wk_cfg.rise_io_group[0] = MIX_PWR_DOWN_UP_IO;
#else
    #if (WKU_IO_FALL_NUM != 0 && WKU_IO_PWRON_EN)
        u8 wku_fall_io_temp[WKU_IO_FALL_NUM] = WKU_IO_FALL_GROUP;
        wk_cfg.wk_io_fall_en = 1;
        wk_cfg.wk_io_fall_num = WKU_IO_FALL_NUM;
        memcpy(wk_cfg.wk_io_fall_group, wku_fall_io_temp, WKU_IO_FALL_NUM);
    #endif
    #if (WKU_IO_RISE_NUM != 0 && WKU_IO_PWRON_EN)
        u8 wku_rise_io_temp[WKU_IO_RISE_NUM] = WKU_IO_RISE_GROUP;
        wk_cfg.wk_io_rise_en = 1;
        wk_cfg.wk_io_rise_num = WKU_IO_RISE_NUM;
        memcpy(wk_cfg.wk_io_rise_group, wku_rise_io_temp, WKU_IO_RISE_NUM);
    #endif
    #if (POWER_ON_FALL_NUM != 0)
        u8 fall_io_temp[POWER_ON_FALL_NUM] = POWER_ON_FALL_IO;
        wk_cfg.wk_fall_en = 1;
        wk_cfg.fall_io_num = POWER_ON_FALL_NUM;
        memcpy(wk_cfg.fall_io_group, fall_io_temp, POWER_ON_FALL_NUM);
    #endif
    #if (POWER_ON_RISE_NUM != 0)
        u8 rise_io_temp[POWER_ON_RISE_NUM] = POWER_ON_RISE_IO;
        wk_cfg.wk_rise_en = 1;
        wk_cfg.rise_io_num = POWER_ON_RISE_NUM;
        memcpy(wk_cfg.rise_io_group, rise_io_temp, POWER_ON_RISE_NUM);
    #endif
#endif
    pwroff_wk_source_config(&wk_cfg);

    rtccon3  =
              BIT(17)       //port wakeup enable
#if PWROFF_MIC_BIAS_EN
            | BIT(18)       //block porb
            | BIT(22)       //Lvcore en
            | BIT(23)       //pdn mode en
#endif
            | BIT(7)        //en vddio_aon
#if !PWROFF_MIC_BIAS_EN
            | BIT(4)        //pd_core2
#endif
            | BIT(3)        //rl2vlc
            ;
    rtccon3 &= ~BIT(6);     //dis PD_CORE
    RTCCON3 = rtccon3;
    pwroff_rtc_core_config(&wk_cfg);
    pwroff_save_to_cache((u8 *)pdn3_shutdown, 100);
}

AT(.text.lowpwr.pdn)
void pdn3(void)
{
    printf("%s poweroff\n", __func__);
    set_all_pin_off();
    PICCONCLR = BIT(0);     //关总中断
    WDT_DIS();

    CLKGAT0 |= (1<<15);     // LP en clock

    // printf("pdn3 down\n");
    RTCCON9 = 0xffffffff;   //clear pending
    RTCCON10 = 0xffffffff;

#if PWROFF_MIC_BIAS_EN
    pwroff_mic_bias_on();
    RTCCON1 = (RTCCON1 & ~(0x3f<<17)) | (RI_LVCORE_SEL<<20) | (RI_LVCORE_SEL<<17);//PDM MODE ENABLE SEL
    PWRCON0 = (PWRCON0 & ~(0x1f<<0))  | (0x10);//DI_VCORES
#endif
    // printf("RTCCON0: %x, RTCCON3: %x, RTCCON4: %x, RTCCON12: %x\n", RTCCON0, RTCCON3, RTCCON4, RTCCON12);
    // printf("GPIOAPU: %x, GPIOBPU: %x, GPIOGPU: %x\n", GPIOAPU, GPIOBPU, GPIOGPU);

    pwroff_pmu_voltage_config();
    pdn3_do();
}
#endif

#if (POWER_DOWN_MODE == PWR_L4M)
AT(.pdncom_text.pdn_shut)
void pdn4_shutdown(void)
{
    u32 rtccon3;
    CLKGAT0 &= ~BIT(2);     //disable ram0 clock
    RTCCON0 &= ~BIT(27);    //sram0 interface disable

    rtccon3 =
              BIT(20)       //vdd11
            | BIT(17)       //port wakeup enable
            | BIT(7)        //en vddio_aon
            | BIT(4)        //pd_core2
            | BIT(3)        //rl2vlc
            ;
    rtccon3 &= ~BIT(6);     //dis PD_CORE
    RTCCON3 = rtccon3;

    LPMCON |= BIT(0);                           //sleep mode
    LPMCON |= BIT(1);                           //idle mode
    while(1);
}

AT(.pdncom_text.lp.sleep)
void lp_sleep_pdn4_proc(void)
{
    //需要把RAM0的程序copy到ICACHE RAM上面跑
    u32 i;
    u8 *cp_dst_ptr, *cp_src_ptr;
    cp_src_ptr = (u8*) &pdn4_shutdown;
    cp_dst_ptr = (u8*) 0x71000;
    for(i = 0; i < 0x100; i++){
        *cp_dst_ptr++ = *cp_src_ptr++;
    }

    // 1. save some thing to stack
    RTCCON0 |= BIT(25);    //LPRST

    // 2. analog disable
    // PG OFF
    // PWRCON1 |= BIT(4) ;           // FS PG PD
    // PWRCON1  = PWRCON1 & ~((0xf << 0)) ; // PG off

    // 3. select sniff clock and disable pll/xosc
    // CLKGAT0 =0xffffffff;
    // CLKGAT1 =0xffffffff;
    // CLKCON0  = (CLKCON0 & ~(3<<2)) | (0<<2);  //SEL RC2M
    // PLL0CON = 0;

    asm("j 0x71000");
    while(1);
}

AT(.pdncom_text.lp.sleep)
void lp_wakeup_pdn4_proc(void)
{
    //唤醒后的处理
    RTCCON |= BIT(1);                               //vrtc voltage select
    RTCCON  = (RTCCON  & ~(0x3<<1))  | (0x3<<1);    //RTC clk
    PWRCON0 = (PWRCON0 & ~(0x1f<<5)) | (0x18<<5);   //VDDIO=2.925v

    RTCCON3 |= 0x7;                                 //ldo enable
    PWRCON0 = (PWRCON0 & ~0x1F) | 0x10;             //VDDCORE is 1.2V
    RTCCON3 &= ~(BIT(18) | BIT(23));

    RTCCON3 |= BIT(5) | BIT(21);
    RTCCON3 &= ~(BIT(3) | BIT(20));

    RTCCON0 |= BIT(26);                             //LP if enable
    RTCCON0 |= BIT(27);                             //SRAM0 enable

    CLKGAT0 |= (3<<2) | BIT(15);                    //SRAM0 & LP_TOP clock enable

    RSTCON0 = 0x1f;
    RSTCON0 |= BIT(7);                              // adda clk rstn
    RSTCON0 |= 1<<4;                                //pllsdm reset release

    RTCCON12 |= (0x3<<6);                           //DIS RTC_WDT

    PWRCON0     = sp_sfr.pwrcon0;
    // PWRCON1     = sp_sfr.pwrcon1;
    // PWRCON0    |= BIT(20);

    PWRCON1 &= ~(0xf << 0);                         //软开机时，关掉flash power gate
    PWRCON1    |= BIT(4);                           //VDDF pulldown
    asm("nop"); asm("nop"); asm("nop"); asm("nop"); //至少4us
    asm("nop"); asm("nop"); asm("nop"); asm("nop");
    asm("nop"); asm("nop"); asm("nop"); asm("nop");
    asm("nop"); asm("nop"); asm("nop"); asm("nop");
    PWRCON1 &= ~BIT(4);                             //flash power gate pull down disable
    PWRCON1 |= BIT(0) | BIT(1) | BIT(2);
    asm("nop"); asm("nop"); asm("nop"); asm("nop");
    PWRCON1 |= BIT(3);

    //第一时间拉高MCP Flash的CS脚，以免CLK翻转影响SPIFlash
    SPI_CS1_DIS();
    SPI_CS1_PDIR &= ~BIT(SPI_CS1);

    CLKGAT0     = sp_sfr.clkgat0;
    CLKGAT1     = sp_sfr.clkgat1;

    PLL0DIV     = sp_sfr.pll0div;
    PLL0CON     = sp_sfr.pll0con;
    PLL0CON    |= BIT(20);                          //update pll0div to pll_clk

    PLL0CON |= BIT(18);                             //pll sdm enable

    GPIOAFEN    = sp_sfr.gpioa_fen;
    GPIOGFEN    = sp_sfr.gpiog_fen;

    GPIOADIR    = sp_sfr.gpioa_dir;
    GPIOGDIR    = sp_sfr.gpiog_dir;

    GPIOADE     = sp_sfr.gpioa_de;
    GPIOGDE     = sp_sfr.gpiog_de;

    GPIOADRV    = sp_sfr.gpioa_drv;
    GPIOGDRV    = sp_sfr.gpiog_drv;

    GPIOAPU     = sp_sfr.gpioa_pu;
    GPIOGPU     = sp_sfr.gpiog_pu;

    GPIOAPD     = sp_sfr.gpioa_pd;
    GPIOGPD     = sp_sfr.gpiog_pd;

    CLKDIVCON0  = sp_sfr.clkdivcon0;
    CLKCON1     = sp_sfr.clkcon1;
    asm("nop");
    asm("nop");
    asm("nop");
    CLKCON0     = (sp_sfr.clkcon0 & (~0x0c));
    asm("nop");
    asm("nop");
    asm("nop");
    CLKCON0 = sp_sfr.clkcon0;

    LVDCON = (LVDCON & ~0x07) | 1;                  //LVD=2.2V,  LVDCON[2:0]
    PICADR = (u32)&__comm_vma;
#if SYS_BRAM_EN
    #if (SYS_BRAM_LEN == 0x1000)
    ICLOCK = 0xff;          //cache预留4K空间用作bram
    #elif (SYS_BRAM_LEN == 0x2000)
    ICLOCK = 0xffff;        //cache预留8K空间用作bram
    #endif
#endif
    CACHCON0 = BIT(16) | BIT(0);                    //dCache Enable, iCache Enable

    EFCON0 |= BIT(3) | BIT(2);                      //EF WP
    MEMCON |= BIT(16);                              //MROM Protect

    SPI0BAUD = 1;                                   //RC CLK / (1 + 1) = 1M
    SPI0CON = BIT(0);                               //SPI EN

    FUNCOUTMCON = sp_sfr.funcoutmcon;
    FUNCOUTCON  = sp_sfr.funcoutcon;
    FUNCINCON   = sp_sfr.funcincon;
    FUNCMCON0   = sp_sfr.funcomcon[0];
    FUNCMCON1   = sp_sfr.funcomcon[1];

    UDETBAUD    = sp_sfr.udetbaud;
    UDETCON     = sp_sfr.udetcon;
    UDETCPND = BIT(10) | BIT(15) | BIT(18);          //CLR Pending
    UDETCON = (UDETCON & ~(0xfff << 16)) | (0x5aa << 16);  //KEY MODE RESET

    PICCON      = sp_sfr.piccon;
    PICPR       = sp_sfr.picpr;
    PICEN       = sp_sfr.picen;
    NMICON      = sp_sfr.nmicon;

    spiflash_init(0x0b, 1);
    #if SPIFLASH_DP_PDN4_EN
    spiflash0_release_dp();
    #endif // SPIFLASH_DP_PDN4_EN

    // GPIOAFEN &= ~BIT(9);
    // GPIOADE  |=  BIT(9);
    // GPIOADIR &= ~BIT(9);
    // GPIOASET = BIT(9);
    // asm("nop");asm("nop");asm("nop");asm("nop");
    // GPIOACLR = BIT(9);
}

//pdn4唤醒后需要初始化一些中断和时钟
AT(.pdncom_text.lp.sleep)
void pdn4_back_init(void)
{
    timer0_init();
    PICCON |= BIT(0);                               //global interrupt enable
    WDT_EN();

    // clock init
    adpll_init(SYS_CLK_SEL);
    set_sys_clk(SYS_CLK_SEL);

    adc_cb.channel = sp_sfr.adc_ch;
    bsp_saradc_init(adc_cb.channel);
    sys_set_tmr_enable(1);

#if DAC_EN
    dac_init();
#endif

    // my_printf(str_back);
    // my_printf("back\n");
}

// AT(.pdncom_text.str)
// const char str_back[] = "back\n";
AT(.pdncom_text.lp.sleep)
void pdn4_do(void)
{
    //唤醒源选择 RTC1S RWDT ALM WKO PORT
    memset(&wk_cfg, 0, sizeof(pwroff_wk_t));
    wk_cfg.pdn_io_interface_flag = 1;
    wk_cfg.lp_type = LP_DEEP_SLEEP;
    wk_cfg.source  = WK_PWROFF_GPIO;
    #if (WKU_IO_FALL_NUM != 0 && WKU_IO_PWRON_EN)
        u8 wku_fall_io_temp[WKU_IO_FALL_NUM] = WKU_IO_FALL_GROUP;
        wk_cfg.wk_io_fall_en = 1;
        wk_cfg.wk_io_fall_num = WKU_IO_FALL_NUM;
        memcpy(wk_cfg.wk_io_fall_group, wku_fall_io_temp, WKU_IO_FALL_NUM);
    #endif
    #if (WKU_IO_RISE_NUM != 0 && WKU_IO_PWRON_EN)
        u8 wku_rise_io_temp[WKU_IO_RISE_NUM] = WKU_IO_RISE_GROUP;
        wk_cfg.wk_io_rise_en = 1;
        wk_cfg.wk_io_rise_num = WKU_IO_RISE_NUM;
        memcpy(wk_cfg.wk_io_rise_group, wku_rise_io_temp, WKU_IO_RISE_NUM);
    #endif
    #if (POWER_ON_FALL_NUM != 0)
        u8 fall_io_temp[POWER_ON_FALL_NUM] = POWER_ON_FALL_IO;
        wk_cfg.wk_fall_en = 1;
        wk_cfg.fall_io_num = POWER_ON_FALL_NUM;
        memcpy(wk_cfg.fall_io_group, fall_io_temp, POWER_ON_FALL_NUM);
    #endif
    #if (POWER_ON_RISE_NUM != 0)
        u8 rise_io_temp[POWER_ON_RISE_NUM] = POWER_ON_RISE_IO;
        wk_cfg.wk_rise_en = 1;
        wk_cfg.rise_io_num = POWER_ON_RISE_NUM;
        memcpy(wk_cfg.rise_io_group, rise_io_temp, POWER_ON_RISE_NUM);
    #endif
    pwroff_wk_source_config(&wk_cfg);
    pwroff_rtc_core_config(&wk_cfg);

    RTCCON0 = (RTCCON0 & ~(0x7<<22)) | (2<<22);     //sys reset delay
    RTCCON0 |= BIT(30);     //low power user flag
    lp_sleep(lp_sleep_pdn4_proc, lp_wakeup_pdn4_proc);

    pdn4_back_init();
}

AT(.text.lowpwr.pdn)
void pdn4(void)
{
    sp_sfr.piccon   = PICCON;
    sp_sfr.picen    = PICEN;
    sp_sfr.picpr    = PICPR;
    sp_sfr.nmicon   = NMICON;
    sp_sfr.clkgat0  = CLKGAT0;
    sp_sfr.clkgat1  = CLKGAT1;

    sp_sfr.pwrcon0  = PWRCON0;
    sp_sfr.pwrcon1  = PWRCON1;

    sp_sfr.pll0con  = PLL0CON;
    sp_sfr.pll0div  = PLL0DIV;

    sp_sfr.clkcon0  = CLKCON0;
    sp_sfr.clkcon1  = CLKCON1;
    sp_sfr.clkdivcon0 = CLKDIVCON0;

    sp_sfr.funcoutmcon  = FUNCOUTMCON;
    sp_sfr.funcoutcon   = FUNCOUTCON;
    sp_sfr.funcincon    = FUNCINCON;
    sp_sfr.funcomcon[0] = FUNCMCON0;
    sp_sfr.funcomcon[1] = FUNCMCON1;

    sp_sfr.udetbaud = UDETBAUD;
    sp_sfr.udetcon  = UDETCON;
    sp_sfr.adc_ch   = adc_cb.channel;

    RTCCON0 |= BIT(25);     // enable lpsw_rstn
    sp_sfr.gpioa_fen = GPIOAFEN;
    sp_sfr.gpiog_fen = GPIOGFEN;
    sp_sfr.gpioa_dir = GPIOADIR;
    sp_sfr.gpiog_dir = GPIOGDIR;
    sp_sfr.gpioa_de  = GPIOADE;
    sp_sfr.gpiog_de  = GPIOGDE;
    sp_sfr.gpioa_drv = GPIOADRV;
    sp_sfr.gpiog_drv = GPIOGDRV;
    sp_sfr.gpioa_pu  = GPIOAPU;
    sp_sfr.gpiog_pu  = GPIOGPU;
    sp_sfr.gpioa_pd  = GPIOAPD;
    sp_sfr.gpiog_pd  = GPIOGPD;

    printf("%s poweroff\n", __func__);
    set_all_pin_off();
    PICCONCLR = BIT(0);     //关总中断
    WDT_DIS();

    RTCCON3 &= ~BIT(6);     // dis PD_CORE
    // RTCCON3 |= (1<<2);      // sw vddcore enable
    CLKGAT0 |= (1<<15);     // LP en clock

    // printf("pdn4 down\n");
    RTCCON9 = 0xffffffff;   //clear pending
    RTCCON10 = 0xffffffff;

    RTCCON3 |= BIT(3);     //enable RL2VLC
    // RTCCON3 |= BIT(20);    //enable RL2V11
    // RTCCON3 |= BIT(22);    //enable LVCORE

    // printf("RTCCON0: %x, RTCCON3: %x, RTCCON4: %x, RTCCON12: %x\n", RTCCON0, RTCCON3, RTCCON4, RTCCON12);
    // printf("GPIOAPU: %x, GPIOBPU: %x, GPIOGPU: %x\n", GPIOAPU, GPIOBPU, GPIOGPU);

    pwroff_pmu_voltage_config();
    pdn4_do();
}
#endif

#if(POWER_DOWN_MODE == PWR_RTC1S)
ALIGNED(512)
AT(.text.lowpwr.rtc1s)
void pdn_rtc1s(void)
{
    bool bo_loop = true;
    printf("%s poweroff\n", __func__);
    set_all_pin_off();
    pdn_rtc_sta_save();
    RTCCON12 |= 0xf3;       //disable GPIO reset
    RTCCON12 &= ~0xc;       //PA11 reset
    PICCONCLR = BIT(0);     //关总中断
    WDT_DIS();

    pwroff_pmu_voltage_config();

    while(bo_loop){
        //唤醒源选择 RTC1S RWDT ALM WKO PORT
        memset(&wk_cfg, 0, sizeof(pwroff_wk_t));
        wk_cfg.pdn_io_interface_flag = 1;
        wk_cfg.source = WK_PWROFF_RTC1S;
        if(POWER_ON_FALL_NUM != 0){
            u8 fall_io_temp[POWER_ON_FALL_NUM] = POWER_ON_FALL_IO;
            wk_cfg.wk_fall_en = 1;
            wk_cfg.fall_io_num = POWER_ON_FALL_NUM;
            memcpy(wk_cfg.fall_io_group, fall_io_temp, POWER_ON_FALL_NUM);
        }
        if(POWER_ON_RISE_NUM != 0){
            u8 rise_io_temp[POWER_ON_RISE_NUM] = POWER_ON_RISE_IO;
            wk_cfg.wk_rise_en = 1;
            wk_cfg.rise_io_num = POWER_ON_RISE_NUM;
            memcpy(wk_cfg.rise_io_group, rise_io_temp, POWER_ON_RISE_NUM);
        }
        pwroff_wk_source_config(&wk_cfg);
        RTCCON0 |=  BIT(31);        //power_flag
        CLKGAT0 &= ~BIT(15);        //disable lp_top clock
        RTCCON0 &= ~BIT(26);        //IO interface disable, io控制寄存器无法写
    #if PWR_RTC1S_WK_MODE
        RTCCON &= ~BIT(0);      //关机选择RTCCON1 BIT8~6
        CLKCON0 = (CLKCON0 & ~(0X3<<2)) | (0X0<<2);
        RTCCON3 = //BIT(19)     //pd_core3
                //   BIT(7)        //en vddio_aon
                // | BIT(4)        //pd_core2
                // | BIT(3)        //rl2vlc
                // | BIT(17)       //port wakeup enable
                  BIT(6)        //pd_core
                | BIT(9)
                ;
        LPMCON |= BIT(0);                           //sleep mode
        LPMCON |= BIT(1);                           //idle mode
        while(1);
    #else
        PWRCON1 &= ~0xF;    //disable flash PG
        CLKCON0 &= ~0xC;    //SYS clock switch to RC
        asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");
        PLL0CON = 0;        //disable PLL
        PWRCON0 = (PWRCON0 & ~(0x1f<<0)) | (0x8<<0);    //DI_VCORES 0x13 = 1.275V

        LPMCON |= BIT(0);                           //sleep mode
        LPMCON |= BIT(1);                           //idle mode
        asm("nop");asm("nop");asm("nop");

    //唤醒后的处理
        pdn_rtc_sta_restore();

        GPIOADE  |= BIT(7);
        GPIOADIR |= BIT(7);
        GPIOAPU  |= BIT(7);
        u32 key_cnt = 0;
        u32 chk_cnt = 50;
        while(chk_cnt > 0) {
            chk_cnt--;
            if(GPIOA & BIT(7)) {                    //输入高电平
                key_cnt = 0;
            } else {                                //有IO按键按下
                key_cnt++;
            }
            if (key_cnt > 20) {
                bo_loop = false;
                break;
            }
        }
        // GPIOAFEN &= ~BIT(5);
        // GPIOADE  |=  BIT(5);
        // GPIOADIR &= ~BIT(5);
        // WDT_CLR();
        // GPIOASET = BIT(5);
        // asm("nop");asm("nop");asm("nop");
        // GPIOACLR = BIT(5);
    #endif
    }
}
#endif


#if(PWR_RUNRC == POWER_DOWN_MODE)
ALIGNED(512)
AT(.text.lowpwr.runrc)
void pdn_runrc(void)
{
    u16 gpioa_fen, gpiog_fen;
    u16 gpioa_dir, gpiog_dir;
    u16 gpioa_de, gpiog_de;
    u16 gpioa_drv, gpiog_drv;
    u16 gpioa_pu, gpiog_pu;
    u16 gpioa_pd, gpiog_pd;

    u16 adc_ch;
    u8 sys_clk;
    u32 cpu_ie;
    printf("%s poweroff\n", __func__);

    gui_display(DISP_OFF);
#if DAC_DNR_EN
    u8 sta = dac_dnr_get_sta();
    dac_dnr_set_sta(0);
#endif
    dac_power_off();                    //dac power down
    sdadc_analog_mic_exit(CH_MIC_PA0);
    saradc_exit(adc_cb.channel);        //close saradc及相关通路模拟

    led_off();
    rled_off();

    sys_clk = get_cur_sysclk();
    set_sys_clk(SYS_RC2M);
    PWRCON0 = (PWRCON0 & ~(0x1f<<0)) | (0x10<<0);           //DI_VCORES 0x10 = 1.2
    sys_set_tmr_enable(0);
    adc_ch = adc_cb.channel;
    adc_cb.channel = ADCCH_VBAT;

    //close pll0
    PLL0CON &= ~BIT(18);                //pll10 sdm enable disable
    // PLL0CON &= ~BIT(6);                 //pll0 analog diable
    // PLL0CON &= ~BIT(12);                //pll0 ldo disable

    //io save
    gpioa_fen = GPIOAFEN;
    gpiog_fen = GPIOGFEN;
    gpioa_dir = GPIOADIR;
    gpiog_dir = GPIOGDIR;
    gpioa_de  = GPIOADE;
    gpiog_de  = GPIOGDE;
    gpioa_drv = GPIOADRV;
    gpiog_drv = GPIOGDRV;
    gpioa_pu  = GPIOAPU;
    gpiog_pu  = GPIOGPU;
    gpioa_pd  = GPIOAPD;
    gpiog_pd  = GPIOGPD;

    // my_printf("gpioapu 0: %X\n", gpioapu);
    GPIOAFEN = 0;
    GPIOADIR = 0;
    GPIOADE = 0;
    GPIOADRV = 0;
    GPIOAPU = 0;
    GPIOAPD = 0;

    // GPIOGFEN = 0;
    // GPIOGDIR = 0;
    GPIOGDE = 0x3F;     //MCP FLASH
    // GPIOGDRV = 0;
    // GPIOGPU = 0;
    // GPIOGPD = 0;

    // wakeup_disable();
    set_all_pin_off();
    cpu_ie = PICCON & BIT(0);
    PICCONCLR = BIT(0);     //关总中断
    WDT_DIS();

    u8 fall_io_temp[POWER_ON_FALL_NUM] = POWER_ON_FALL_IO;
    GPIOADE  |= BIT(fall_io_temp[0]-1);
    GPIOADIR |= BIT(fall_io_temp[0]-1);
    GPIOAPU  |= BIT(fall_io_temp[0]-1);
    u32 key_cnt = 0;
    u32 io_sta = GPIOA;
    while ((io_sta & BIT(fall_io_temp[0]-1)) == 0){         //等待IO松开
        io_sta = GPIOA;
        delay_us(5000);
    }
    while(1) {
        if(GPIOA & BIT(fall_io_temp[0]-1)) {                //输入高电平
            key_cnt = 0;
        } else {                                            //有IO按键按下
            key_cnt++;
            delay_us(5000);
        }
        if (key_cnt > 200) {
            break;
        }
    }

    PICCON |= cpu_ie;
    WDT_EN();

    GPIOAFEN = gpioa_fen;
    GPIOGFEN = gpiog_fen;
    GPIOADIR = gpioa_dir;
    GPIOGDIR = gpiog_dir;
    GPIOADE  = gpioa_de;
    GPIOGDE  = gpiog_de;
    GPIOADRV = gpioa_drv;
    GPIOGDRV = gpiog_drv;
    GPIOAPU  = gpioa_pu;
    GPIOGPU  = gpiog_pu;
    GPIOAPD  = gpioa_pd;
    GPIOGPD  = gpiog_pd;

    // wakeup_disable();

    //enable adpll
    // PLL0CON |= BIT(12);                //PLL1 LDO enable
    delay_us(100);
    PLL0CON |= BIT(18);                 //PLL1 SDM enable
    // PLL0CON |= BIT(6);                 //PLL1 Analog enable
    if(sys_clk == SYS_192M){
        PWRCON0 = (PWRCON0 & ~(0x1f<<0)) | (0x13<<0);       //DI_VCORES 0x13 = 1.275V
    }else if (sys_clk > SYS_120M) {
        PWRCON0 = (PWRCON0 & ~(0x1f<<0)) | (0x11<<0);       //DI_VCORES 0x10 = 1.225V
    }
    set_sys_clk(sys_clk);              //还原系统时钟
    adc_cb.channel = adc_ch;
    bsp_saradc_init(adc_cb.channel);
    sys_set_tmr_enable(1);

    dac_restart();
    bsp_change_volume(sys_cb.vol);
    // gui_init();
    // gui_display(DISP_POWERON);
#if DAC_DNR_EN
    dac_dnr_set_sta(sta);
#endif

    func_cb.sta = func_cb.last;
    func_cb.last = FUNC_NULL;
}
#endif

#if(PWR_ALARM == POWER_DOWN_MODE)
AT(.pdncom_text.pdn_alm)
void pdn_alarm(void)
{
    u32 rtccon3;
    printf("%s poweroff\n", __func__);
    set_all_pin_off();
    PICCONCLR = BIT(0);     //关总中断
    WDT_DIS();

    RTCCON9 = 0xffffffff;   //clear pending
    RTCCON10 = 0xffffffff;

    // printf("RTCCON: %x, RTCCON0: %x, RTCCON3: %x, RTCCON9: %x, RTCCON12: %x\n", RTCCON, RTCCON0, RTCCON3, RTCCON9, RTCCON12);
    // printf("GPIOAPU: %x, GPIOBPU: %x, GPIOGPU: %x\n", GPIOAPU, GPIOBPU, GPIOGPU);
    // printf("RTCCON2: %X, RTCALM: %X\n", RTCCON2, RTCALM);
    // NMICON = 0;

    pwroff_pmu_voltage_config();

    //唤醒源选择 RTC1S RWDT ALM WKO PORT
    memset(&wk_cfg, 0, sizeof(pwroff_wk_t));
    wk_cfg.pdn_io_interface_flag = 1;
    wk_cfg.lp_type = LP_DEEP_SLEEP;
    wk_cfg.source  = WK_PWROFF_ALARM;
    if(POWER_ON_FALL_NUM != 0){
        u8 fall_io_temp[POWER_ON_FALL_NUM] = POWER_ON_FALL_IO;
        wk_cfg.wk_fall_en = 1;
        wk_cfg.fall_io_num = POWER_ON_FALL_NUM;
        memcpy(wk_cfg.fall_io_group, fall_io_temp, POWER_ON_FALL_NUM);
    }
    if(POWER_ON_RISE_NUM != 0){
        u8 rise_io_temp[POWER_ON_RISE_NUM] = POWER_ON_RISE_IO;
        wk_cfg.wk_rise_en = 1;
        wk_cfg.rise_io_num = POWER_ON_RISE_NUM;
        memcpy(wk_cfg.rise_io_group, rise_io_temp, POWER_ON_RISE_NUM);
    }
    pwroff_wk_source_config(&wk_cfg);

    rtccon3  =
              BIT(8)            //RTC alarm
            | BIT(4)            //pd_core2
            | BIT(3)            //rl2vlc
            ;
    rtccon3 &= ~BIT(6);         //dis PD_CORE
    RTCCON3 = rtccon3;

    pwroff_rtc_core_config(&wk_cfg);

    CLKGAT0 &= ~BIT(2);         //disable ram0 clock
    RTCCON0 &= ~BIT(27);        //sram0 interface disable

    LPMCON |= BIT(0);           //sleep mode
    LPMCON |= BIT(1);           //idle mode
    asm("nop");asm("nop");asm("nop");
    while(1);
}
#endif

#if (PWR_L3M_ALM == POWER_DOWN_MODE)
AT(.com_text.pdn_shut)
void pdn3_alm_shutdown(void)
{
    CLKGAT0 &= ~BIT(2);     //disable ram0 clock
    RTCCON0 &= ~BIT(27);    //sram0 interface disable

    LPMCON |= BIT(0);       //sleep mode
    LPMCON |= BIT(1);       //idle mode
    while(1);
}

AT(.pdncom_text.l3m_alm)
void pdn_l3m_alm(void)
{
    u32 rtccon3;
    printf("%s poweroff\n", __func__);
    set_all_pin_off();
    PICCONCLR = BIT(0);     //关总中断
    WDT_DIS();

    CLKGAT0 |= (1<<15);     // LP en clock

    RTCCON9 = 0xffffffff;   //clear pending
    RTCCON10 = 0xffffffff;

    // printf("RTCCON: %x, RTCCON0: %x, RTCCON2: %x, RTCCON3: %x, RTCCON12: %x\n", RTCCON, RTCCON0, RTCCON2, RTCCON3, RTCCON12);
    // printf("GPIOAPU: %x, GPIOBPU: %x, GPIOGPU: %x\n", GPIOAPU, GPIOBPU, GPIOGPU);
    // printf("RTCCON2: %X, RTCALM: %X\n", RTCCON2, RTCALM);

    pwroff_pmu_voltage_config();

    //唤醒源选择 RTC1S RWDT ALM WKO PORT
    memset(&wk_cfg, 0, sizeof(pwroff_wk_t));
    wk_cfg.pdn_io_interface_flag = 1;
    wk_cfg.lp_type = LP_DEEP_SLEEP;
    wk_cfg.source  = WK_PWROFF_ALARM | WK_PWROFF_GPIO;
    #if (WKU_IO_FALL_NUM != 0 && WKU_IO_PWRON_EN)
        u8 wku_fall_io_temp[WKU_IO_FALL_NUM] = WKU_IO_FALL_GROUP;
        wk_cfg.wk_io_fall_en = 1;
        wk_cfg.wk_io_fall_num = WKU_IO_FALL_NUM;
        memcpy(wk_cfg.wk_io_fall_group, wku_fall_io_temp, WKU_IO_FALL_NUM);
    #endif
    #if (WKU_IO_RISE_NUM != 0 && WKU_IO_PWRON_EN)
        u8 wku_rise_io_temp[WKU_IO_RISE_NUM] = WKU_IO_RISE_GROUP;
        wk_cfg.wk_io_rise_en = 1;
        wk_cfg.wk_io_rise_num = WKU_IO_RISE_NUM;
        memcpy(wk_cfg.wk_io_rise_group, wku_rise_io_temp, WKU_IO_RISE_NUM);
    #endif
    #if (POWER_ON_FALL_NUM != 0)
        u8 fall_io_temp[POWER_ON_FALL_NUM] = POWER_ON_FALL_IO;
        wk_cfg.wk_fall_en = 1;
        wk_cfg.fall_io_num = POWER_ON_FALL_NUM;
        memcpy(wk_cfg.fall_io_group, fall_io_temp, POWER_ON_FALL_NUM);
    #endif
    #if (POWER_ON_RISE_NUM != 0)
        u8 rise_io_temp[POWER_ON_RISE_NUM] = POWER_ON_RISE_IO;
        wk_cfg.wk_rise_en = 1;
        wk_cfg.rise_io_num = POWER_ON_RISE_NUM;
        memcpy(wk_cfg.rise_io_group, rise_io_temp, POWER_ON_RISE_NUM);
    #endif
    pwroff_wk_source_config(&wk_cfg);

    rtccon3  =
              BIT(17)           //port wakeup enable
#if PWROFF_MIC_BIAS_EN
            | BIT(18)       //block porb
            | BIT(22)       //Lvcore en
            | BIT(23)       //pdn mode en
#endif
            | BIT(8)            //RTC alarm
            | BIT(7)            //en vddio_aon
#if !PWROFF_MIC_BIAS_EN
            | BIT(4)            //pd_core2
#endif
            | BIT(3)            //rl2vlc
            ;
    rtccon3 &= ~BIT(6);         //dis PD_CORE
    RTCCON3 = rtccon3;

    pwroff_rtc_core_config(&wk_cfg);
    pwroff_save_to_cache((u8 *)pdn3_alm_shutdown, 100);
}
#endif

#if (PWR_L4M_ALM == POWER_DOWN_MODE)
AT(.pdncom_text.pdn_shut)
void pdn4_alm_shutdown(void)
{
    u32 rtccon3;
    CLKGAT0 &= ~BIT(2);     //disable ram0 clock
    RTCCON0 &= ~BIT(27);    //sram0 interface disable

    rtccon3 =
              BIT(20)       //vdd11
            | BIT(17)       //port wakeup enable
            | BIT(8)        //RTC alarm
            | BIT(7)        //en vddio_aon
            | BIT(4)        //pd_core2
            | BIT(3)        //rl2vlc
            ;
    rtccon3 &= ~BIT(6);     //dis PD_CORE
    RTCCON3 = rtccon3;

    LPMCON |= BIT(0);                           //sleep mode
    LPMCON |= BIT(1);                           //idle mode
    while(1);
}

AT(.pdncom_text.lp.sleep)
void lp_sleep_pdn4_alm_proc(void)
{
    //需要把RAM0的程序copy到ICACHE RAM上面跑
    u32 i;
    u8 *cp_dst_ptr, *cp_src_ptr;
    cp_src_ptr = (u8*) &pdn4_alm_shutdown;
    cp_dst_ptr = (u8*) 0x71000;
    for(i = 0; i < 0x100; i++){
        *cp_dst_ptr++ = *cp_src_ptr++;
    }

    // 1. save some thing to stack
    RTCCON0 |= BIT(25);    //LPRST

    // 2. analog disable
    // PG OFF
    // PWRCON1 |= BIT(4) ;           // FS PG PD
    // PWRCON1  = PWRCON1 & ~((0xf << 0)) ; // PG off

    // 3. select sniff clock and disable pll/xosc
    // CLKGAT0 =0xffffffff;
    // CLKGAT1 =0xffffffff;
    // CLKCON0  = (CLKCON0 & ~(3<<2)) | (0<<2);  //SEL RC2M
    // PLL0CON = 0;

    asm("j 0x71000");
    while(1);
}

AT(.pdncom_text.lp.sleep)
void lp_wakeup_pdn4_alm_proc(void)
{
    //唤醒后的处理
    RTCCON |= BIT(1);                               //vrtc voltage select
    RTCCON  = (RTCCON  & ~(0x3<<1))  | (0x3<<1);    //RTC clk
    PWRCON0 = (PWRCON0 & ~(0x1f<<5)) | (0x18<<5);   //VDDIO=2.925v

    RTCCON3 |= 0x7;                                 //ldo enable
    PWRCON0 = (PWRCON0 & ~0x1F) | 0x10;             //VDDCORE is 1.2V
    RTCCON3 &= ~(BIT(18) | BIT(23));

    RTCCON3 |= BIT(5) | BIT(21);
    RTCCON3 &= ~(BIT(3) | BIT(20));

    RTCCON0 |= BIT(26);                             //LP if enable
    RTCCON0 |= BIT(27);                             //SRAM0 enable

    CLKGAT0 |= (3<<2) | BIT(15);                    //SRAM0 & LP_TOP clock enable

    RSTCON0 = 0x1f;
    RSTCON0 |= BIT(7);                              // adda clk rstn
    RSTCON0 |= 1<<4;                                //pllsdm reset release

    RTCCON12 |= (0x3<<6);                           //DIS RTC_WDT

    PWRCON0     = sp_sfr.pwrcon0;
    // PWRCON1     = sp_sfr.pwrcon1;
    // PWRCON0    |= BIT(20);

    PWRCON1 &= ~(0xf << 0);                         //软开机时，关掉flash power gate
    PWRCON1    |= BIT(4);                           //VDDF pulldown
    asm("nop"); asm("nop"); asm("nop"); asm("nop"); //至少4us
    asm("nop"); asm("nop"); asm("nop"); asm("nop");
    asm("nop"); asm("nop"); asm("nop"); asm("nop");
    asm("nop"); asm("nop"); asm("nop"); asm("nop");
    PWRCON1 &= ~BIT(4);                             //flash power gate pull down disable
    PWRCON1 |= BIT(0) | BIT(1) | BIT(2);
    asm("nop"); asm("nop"); asm("nop"); asm("nop");
    PWRCON1 |= BIT(3);

    //第一时间拉高MCP Flash的CS脚，以免CLK翻转影响SPIFlash
    SPI_CS1_DIS();
    SPI_CS1_PDIR &= ~BIT(SPI_CS1);

    CLKGAT0     = sp_sfr.clkgat0;
    CLKGAT1     = sp_sfr.clkgat1;

    PLL0DIV     = sp_sfr.pll0div;
    PLL0CON     = sp_sfr.pll0con;
    PLL0CON    |= BIT(20);                          //update pll0div to pll_clk

    PLL0CON |= BIT(18);                             //pll sdm enable

    GPIOAFEN    = sp_sfr.gpioa_fen;
    GPIOGFEN    = sp_sfr.gpiog_fen;

    GPIOADIR    = sp_sfr.gpioa_dir;
    GPIOGDIR    = sp_sfr.gpiog_dir;

    GPIOADE     = sp_sfr.gpioa_de;
    GPIOGDE     = sp_sfr.gpiog_de;

    GPIOADRV    = sp_sfr.gpioa_drv;
    GPIOGDRV    = sp_sfr.gpiog_drv;

    GPIOAPU     = sp_sfr.gpioa_pu;
    GPIOGPU     = sp_sfr.gpiog_pu;

    GPIOAPD     = sp_sfr.gpioa_pd;
    GPIOGPD     = sp_sfr.gpiog_pd;

    CLKDIVCON0  = sp_sfr.clkdivcon0;
    CLKCON1     = sp_sfr.clkcon1;
    asm("nop");
    asm("nop");
    asm("nop");
    CLKCON0     = (sp_sfr.clkcon0 & (~0x0c));
    asm("nop");
    asm("nop");
    asm("nop");
    CLKCON0 = sp_sfr.clkcon0;

    LVDCON = (LVDCON & ~0x07) | 1;                  //LVD=2.2V,  LVDCON[2:0]
    PICADR = (u32)&__comm_vma;
#if SYS_BRAM_EN
    #if (SYS_BRAM_LEN == 0x1000)
    ICLOCK = 0xff;          //cache预留4K空间用作bram
    #elif (SYS_BRAM_LEN == 0x2000)
    ICLOCK = 0xffff;        //cache预留8K空间用作bram
    #endif
#endif
    CACHCON0 = BIT(16) | BIT(0);                    //dCache Enable, iCache Enable

    EFCON0 |= BIT(3) | BIT(2);                      //EF WP
    MEMCON |= BIT(16);                              //MROM Protect

    SPI0BAUD = 1;                                   //RC CLK / (1 + 1) = 1M
    SPI0CON = BIT(0);                               //SPI EN

    FUNCOUTMCON = sp_sfr.funcoutmcon;
    FUNCOUTCON  = sp_sfr.funcoutcon;
    FUNCINCON   = sp_sfr.funcincon;
    FUNCMCON0   = sp_sfr.funcomcon[0];
    FUNCMCON1   = sp_sfr.funcomcon[1];

    UDETBAUD    = sp_sfr.udetbaud;
    UDETCON     = sp_sfr.udetcon;
    UDETCPND = BIT(10) | BIT(15) | BIT(18);          //CLR Pending
    UDETCON = (UDETCON & ~(0xfff << 16)) | (0x5aa << 16);  //KEY MODE RESET

    PICCON      = sp_sfr.piccon;
    PICPR       = sp_sfr.picpr;
    PICEN       = sp_sfr.picen;
    NMICON      = sp_sfr.nmicon;

    spiflash_init(0x0b, 1);
    #if SPIFLASH_DP_PDN4_EN
    spiflash0_release_dp();
    #endif // SPIFLASH_DP_PDN4_EN

	//memcpy(&__comm_vma, &__comm_lma, (u32)&__comm_size);

    // GPIOAFEN &= ~BIT(9);
    // GPIOADE  |=  BIT(9);
    // GPIOADIR &= ~BIT(9);
    // GPIOASET = BIT(9);
    // asm("nop");asm("nop");asm("nop");asm("nop");
    // GPIOACLR = BIT(9);
}

//pdn4唤醒后需要初始化一些中断和时钟
AT(.pdncom_text.lp.sleep)
void pdn4_alm_back_init(void)
{
    timer0_init();
    PICCON |= BIT(0);                               //global interrupt enable
    WDT_EN();

    // clock init
    adpll_init(SYS_CLK_SEL);
    set_sys_clk(SYS_CLK_SEL);

    adc_cb.channel = sp_sfr.adc_ch;
    bsp_saradc_init(adc_cb.channel);
    sys_set_tmr_enable(1);

    // my_printf(str_back);
    // my_printf("back\n");
}

// AT(.pdncom_text.str)
// const char str_back[] = "back\n";
AT(.pdncom_text.lp.sleep)
void pdn4_alm_do(void)
{
    //唤醒源选择 RTC1S RWDT ALM WKO PORT
    memset(&wk_cfg, 0, sizeof(pwroff_wk_t));
    wk_cfg.pdn_io_interface_flag = 1;
    wk_cfg.lp_type = LP_DEEP_SLEEP;
    wk_cfg.source  = WK_PWROFF_GPIO | WK_PWROFF_ALARM;
    #if (WKU_IO_FALL_NUM != 0 && WKU_IO_PWRON_EN)
        u8 wku_fall_io_temp[WKU_IO_FALL_NUM] = WKU_IO_FALL_GROUP;
        wk_cfg.wk_io_fall_en = 1;
        wk_cfg.wk_io_fall_num = WKU_IO_FALL_NUM;
        memcpy(wk_cfg.wk_io_fall_group, wku_fall_io_temp, WKU_IO_FALL_NUM);
    #endif
    #if (WKU_IO_RISE_NUM != 0 && WKU_IO_PWRON_EN)
        u8 wku_rise_io_temp[WKU_IO_RISE_NUM] = WKU_IO_RISE_GROUP;
        wk_cfg.wk_io_rise_en = 1;
        wk_cfg.wk_io_rise_num = WKU_IO_RISE_NUM;
        memcpy(wk_cfg.wk_io_rise_group, wku_rise_io_temp, WKU_IO_RISE_NUM);
    #endif
    #if (POWER_ON_FALL_NUM != 0)
        u8 fall_io_temp[POWER_ON_FALL_NUM] = POWER_ON_FALL_IO;
        wk_cfg.wk_fall_en = 1;
        wk_cfg.fall_io_num = POWER_ON_FALL_NUM;
        memcpy(wk_cfg.fall_io_group, fall_io_temp, POWER_ON_FALL_NUM);
    #endif
    #if (POWER_ON_RISE_NUM != 0)
        u8 rise_io_temp[POWER_ON_RISE_NUM] = POWER_ON_RISE_IO;
        wk_cfg.wk_rise_en = 1;
        wk_cfg.rise_io_num = POWER_ON_RISE_NUM;
        memcpy(wk_cfg.rise_io_group, rise_io_temp, POWER_ON_RISE_NUM);
    #endif
    pwroff_wk_source_config(&wk_cfg);
    pwroff_rtc_core_config(&wk_cfg);

    RTCCON0 = (RTCCON0 & ~(0x7<<22)) | (1<<22);     //sys reset delay
    RTCCON0 |= BIT(30);     //low power user flag
    lp_sleep(lp_sleep_pdn4_alm_proc, lp_wakeup_pdn4_alm_proc);

    pdn4_alm_back_init();
}

AT(.pdncom_text.lp.sleep)
void pdn_l4m_alm(void)
{
    sp_sfr.piccon   = PICCON;
    sp_sfr.picen    = PICEN;
    sp_sfr.picpr    = PICPR;
    sp_sfr.nmicon   = NMICON;
    sp_sfr.clkgat0  = CLKGAT0;
    sp_sfr.clkgat1  = CLKGAT1;

    sp_sfr.pwrcon0  = PWRCON0;
    sp_sfr.pwrcon1  = PWRCON1;

    sp_sfr.pll0con  = PLL0CON;
    sp_sfr.pll0div  = PLL0DIV;

    sp_sfr.clkcon0  = CLKCON0;
    sp_sfr.clkcon1  = CLKCON1;
    sp_sfr.clkdivcon0 = CLKDIVCON0;

    sp_sfr.funcoutmcon  = FUNCOUTMCON;
    sp_sfr.funcoutcon   = FUNCOUTCON;
    sp_sfr.funcincon    = FUNCINCON;
    sp_sfr.funcomcon[0] = FUNCMCON0;
    sp_sfr.funcomcon[1] = FUNCMCON1;

    sp_sfr.udetbaud = UDETBAUD;
    sp_sfr.udetcon  = UDETCON;
    sp_sfr.adc_ch   = adc_cb.channel;

    RTCCON0 |= BIT(25);     // enable lpsw_rstn
    sp_sfr.gpioa_fen = GPIOAFEN;
    sp_sfr.gpiog_fen = GPIOGFEN;
    sp_sfr.gpioa_dir = GPIOADIR;
    sp_sfr.gpiog_dir = GPIOGDIR;
    sp_sfr.gpioa_de  = GPIOADE;
    sp_sfr.gpiog_de  = GPIOGDE;
    sp_sfr.gpioa_drv = GPIOADRV;
    sp_sfr.gpiog_drv = GPIOGDRV;
    sp_sfr.gpioa_pu  = GPIOAPU;
    sp_sfr.gpiog_pu  = GPIOGPU;
    sp_sfr.gpioa_pd  = GPIOAPD;
    sp_sfr.gpiog_pd  = GPIOGPD;

    printf("%s poweroff\n", __func__);
    set_all_pin_off();
    PICCONCLR = BIT(0);     //关总中断
    WDT_DIS();

    CLKGAT0 |= (1<<15);     // LP en clock

    RTCCON9 = 0xffffffff;   //clear pending
    RTCCON10 = 0xffffffff;

    // RTCCON3 |= BIT(3);     //enable RL2VLC
    // RTCCON3 |= BIT(20);    //enable RL2V11
    // RTCCON3 |= BIT(22);    //enable LVCORE

    // printf("RTCCON0: %x, RTCCON3: %x, RTCCON4: %x, RTCCON12: %x\n", RTCCON0, RTCCON3, RTCCON4, RTCCON12);
    // printf("GPIOAPU: %x, GPIOBPU: %x, GPIOGPU: %x\n", GPIOAPU, GPIOBPU, GPIOGPU);

    pwroff_pmu_voltage_config();
    pdn4_alm_do();
}
#endif

void pwron_fall_set_do(u8 io_num)
{
    if(io_num > IO_PA15){
        GPIOBFEN &= ~(BIT(io_num-1-IO_PA15));
        GPIOBDE  |=  (BIT(io_num-1-IO_PA15));
        GPIOBPU  |=  (BIT(io_num-1-IO_PA15));
        GPIOBPD  &= ~(BIT(io_num-1-IO_PA15));       //取消相应脚的下拉
    }else{
        GPIOAFEN &= ~(BIT(io_num-1));
        GPIOADE  |=  (BIT(io_num-1));
        GPIOAPU  |=  (BIT(io_num-1));
        GPIOAPD  &= ~(BIT(io_num-1));
    }
}

void pwron_rise_set_do(u8 io_num)
{
    if(io_num > IO_PA15){
        GPIOBFEN &= ~(BIT(io_num-1-IO_PA15));
        GPIOBDE  |=  (BIT(io_num-1-IO_PA15));
        GPIOBPD  |=  (BIT(io_num-1-IO_PA15));
        GPIOBPU  &= ~(BIT(io_num-1-IO_PA15));       //取消相应脚的上拉
    }else{
        GPIOAFEN &= ~(BIT(io_num-1));
        GPIOADE  |=  (BIT(io_num-1));
        GPIOAPD  |=  (BIT(io_num-1));
        GPIOAPU  &= ~(BIT(io_num-1));
    }
}

void wku_io_fall_set_do(u8 pin_num)
{
    if(pin_num == WKU_PA1){
        GPIOAFEN &= ~BIT(1);
        GPIOADE  |=  BIT(1);
        GPIOAPU  |=  BIT(1);
        GPIOAPD  &= ~BIT(1);
    }else if(pin_num == WKU_PA2){
        GPIOAFEN &= ~BIT(2);
        GPIOADE  |=  BIT(2);
        GPIOAPU  |=  BIT(2);
        GPIOAPD  &= ~BIT(2);
    }else if(pin_num == WKU_PA4){
        GPIOAFEN &= ~BIT(4);
        GPIOADE  |=  BIT(4);
        GPIOAPU  |=  BIT(4);
        GPIOAPD  &= ~BIT(4);
    }else if(pin_num == WKU_PA9){
        GPIOAFEN &= ~BIT(9);
        GPIOADE  |=  BIT(9);
        GPIOAPU  |=  BIT(9);
        GPIOAPD  &= ~BIT(9);
    }else if(pin_num == WKU_PA10){
        GPIOAFEN &= ~BIT(10);
        GPIOADE  |=  BIT(10);
        GPIOAPU  |=  BIT(10);
        GPIOAPD  &= ~BIT(10);
    }else if(pin_num == WKU_PA13){
        GPIOAFEN &= ~BIT(13);
        GPIOADE  |=  BIT(13);
        GPIOAPU  |=  BIT(13);
        GPIOAPD  &= ~BIT(13);
    }
}

void wku_io_rise_set_do(u8 pin_num)
{
    if(pin_num == WKU_PA1){
        GPIOAFEN &= ~BIT(1);
        GPIOADE  |=  BIT(1);
        GPIOAPD  |=  BIT(1);
        GPIOAPU  &= ~BIT(1);
    }else if(pin_num == WKU_PA2){
        GPIOAFEN &= ~BIT(2);
        GPIOADE  |=  BIT(2);
        GPIOAPD  |=  BIT(2);
        GPIOAPU  &= ~BIT(2);
    }else if(pin_num == WKU_PA4){
        GPIOAFEN &= ~BIT(4);
        GPIOADE  |=  BIT(4);
        GPIOAPD  |=  BIT(4);
        GPIOAPU  &= ~BIT(4);
    }else if(pin_num == WKU_PA9){
        GPIOAFEN &= ~BIT(9);
        GPIOADE  |=  BIT(9);
        GPIOAPD  |=  BIT(9);
        GPIOAPU  &= ~BIT(9);
    }else if(pin_num == WKU_PA10){
        GPIOAFEN &= ~BIT(10);
        GPIOADE  |=  BIT(10);
        GPIOAPD  |=  BIT(10);
        GPIOAPU  &= ~BIT(10);
    }else if(pin_num == WKU_PA13){
        GPIOAFEN &= ~BIT(13);
        GPIOADE  |=  BIT(13);
        GPIOAPD  |=  BIT(13);
        GPIOAPU  &= ~BIT(13);
    }
}

#if MUL_PWRON_IO_EN
extern const u8 mulpwr_fall_group[];
extern const u8 mulpwr_rise_group[];

void mul_pwron_set(void)
{
#if (MUL_PWRON_FALL_NUM != 0)
    for (int i = 0; i < MUL_PWRON_FALL_NUM; i++) {
        pwron_fall_set_do(mulpwr_fall_group[i]);
    }
#endif
#if (MUL_PWRON_RISE_NUM != 0)
    for (int i = 0; i < MUL_PWRON_RISE_NUM; i++) {
        pwron_rise_set_do(mulpwr_rise_group[i]);
    }
#endif
}

void mul_pwron_fall_int_do(u8 io_num)
{
    PORTINTEDG |=  BIT(io_num-1);      //IO fall edge
    PORTINTEN  |=  BIT(io_num-1);      //enable IO interrupt
    WKUPEDG |= BIT(6);                 //fall edge
    WKUPCON |= BIT(6);                 //6 enable
    WKUPCPND = BIT(22);                //clear 6 pending
    WKUPCON |= 0x1<<20;                //fall filter select
    WKUPCON |= BIT(17);                //wakup sniff enable
}

void mul_pwron_rise_int_do(u8 io_num)
{
    PORTINTEDG &= ~BIT(io_num-1);      //IO rise edge
    PORTINTEN  |=  BIT(io_num-1);      //enable IO interrupt
    WKUPEDG &= ~BIT(7);                //rise edge
    WKUPCON |=  BIT(7);                //7 enable
	WKUPCPND =  BIT(23);			   //clear 7 pending
    WKUPCON |= 0x1<<22;                //rise filter select
    WKUPCON |= BIT(17);                //wakup sniff enable
}

void mul_pwron_int_set(void)
{
#if (MUL_PWRON_FALL_NUM != 0)
    for (int i = 0; i < MUL_PWRON_FALL_NUM; i++) {
        mul_pwron_fall_int_do(mulpwr_fall_group[i]);
    }
#endif
#if (MUL_PWRON_RISE_NUM != 0)
    for (int i = 0; i < MUL_PWRON_RISE_NUM; i++) {
        mul_pwron_rise_int_do(mulpwr_rise_group[i]);
    }
#endif
}
#endif

//设置所有IO,避免关机后漏电
void set_all_pin_off(void)
{
    GPIOADE = 0;
    GPIOBDE = 0;

    GPIOAPU = 0;
    GPIOBPU = 0;
    GPIOAPD = 0;
    GPIOBPD = 0;
    GPIOGPU = 0x0;
}

AT(.pdncom_text.pwroff)
void wk_gpio_pin_init_do(pwroff_wk_t *wkcfg)
{
    if(wk_cfg.wk_io_fall_en){
        for (int i = 0; i < wk_cfg.wk_io_fall_num; i++) {
            wku_io_fall_set_do(wk_cfg.wk_io_fall_group[i]);
        }
    }
    if(wk_cfg.wk_fall_en){
        for (int i = 0; i < wk_cfg.fall_io_num; i++) {
            pwron_fall_set_do(wk_cfg.fall_io_group[i]);
        }
    }
    if((!wk_cfg.wk_io_fall_en) && (!wk_cfg.wk_fall_en)){
        GPIOBPU = 0;
        GPIOAPU = 0;
    }

    if(wk_cfg.wk_io_rise_en){
        for (int i = 0; i < wk_cfg.wk_io_rise_num; i++) {
            wku_io_rise_set_do(wk_cfg.wk_io_rise_group[i]);
        }
    }
    if(wk_cfg.wk_rise_en){
        for (int i = 0; i < wk_cfg.rise_io_num; i++) {
            pwron_rise_set_do(wk_cfg.rise_io_group[i]);
        }
    }
    if((!wk_cfg.wk_io_rise_en) && (!wk_cfg.wk_rise_en)){
        GPIOBPD = 0;
        GPIOAPD = 0;
    }
}

AT(.pdncom_text.pwroff)
void wk_gpio_wakeup_init_do(pwroff_wk_t *wkcfg)
{
    if(wk_cfg.wk_io_fall_en){
        for (int i = 0; i < wk_cfg.wk_io_fall_num; i++) {
            WKUPEDG  |= BIT(wk_cfg.wk_io_fall_group[i]);        //fall edge
            WKUPCON  |= BIT(wk_cfg.wk_io_fall_group[i]);        //wakeup enable
            WKUPCPND |= BIT(wk_cfg.wk_io_fall_group[i] + 16);   //clear wakeup pending
        }
        // WKUPCON |= 0x1<<20;         //fall filter select
    }
    if(wk_cfg.wk_io_rise_en){
        for (int i = 0; i < wk_cfg.wk_io_rise_num; i++) {
            WKUPEDG  &= ~BIT(wk_cfg.wk_io_rise_group[i]);       //rise edge
            WKUPCON  |=  BIT(wk_cfg.wk_io_rise_group[i]);       //wakeup enable
            WKUPCPND |=  BIT(wk_cfg.wk_io_rise_group[i] + 16);  //clear wakeup pending
        }
        // WKUPCON |= 0x1<<22;         //fall filter select
    }
    if(wk_cfg.wk_fall_en){
        for (int i = 0; i < wk_cfg.fall_io_num; i++) {
            PORTINTEDG |=  BIT(wk_cfg.fall_io_group[i]-1);      //IO fall edge
            PORTINTEN  |=  BIT(wk_cfg.fall_io_group[i]-1);      //enable IO interrupt
        }
        WKUPEDG |= BIT(6);          //fall edge
        WKUPCON |= BIT(6);          //6 enable
        WKUPCPND = BIT(22);         //clear 6 pending
        WKUPCON |= 0x1<<20;         //fall filter select
    }
    if(wk_cfg.wk_rise_en){
        for (int i = 0; i < wk_cfg.rise_io_num; i++) {
            PORTINTEDG &= ~BIT(wk_cfg.rise_io_group[i]-1);      //IO rise edge
            PORTINTEN  |=  BIT(wk_cfg.rise_io_group[i]-1);      //enable IO interrupt
        }
        WKUPEDG &= ~BIT(7);         //rise edge
        WKUPCON |=  BIT(7);         //7 enable
        WKUPCPND =  BIT(23);        //clear 7 pending
        WKUPCON |= 0x1<<22;         //rise filter select
    }

    WKUPCON |= BIT(17);             //wakup sniff enable
}

AT(.pdncom_text.pwroff)
void pwroff_wk_source_config(pwroff_wk_t *wkcfg)
{
    u32 rtccon;
    if(wkcfg){
        rtccon   = RTCCON;
        // my_printf("wkcfg->source: %X\n", wkcfg->source);
        if(wkcfg->source & WK_PWROFF_ALARM){
            // my_printf("WK_PWROFF_ALARM\n");
            u32 rc2m_clk;
            clk_clk32k_rtc_set(CLK32K_RTC_RC2M_RTC);        //enable rtc clock
            rtc_clr_alarm();
            rc2m_clk = rc2m_clk_nhz_get(1);                 //get rc2m actual frequency
            rtc_prescale_set((rc2m_clk/64) - 1);            //Config predivision factor for 1Hz counter
            rtc_set_alarm(rtc_get_alarm() + PWR_ALARM_WK_TIME);//Set the alarm to wake up after x second

            RTCCON0 |= BIT(0) | BIT(2);                     //RC enable & RTC output clock to core enable
            RTCCNT   = 0;
            CLKCON0  = (CLKCON0 & ~(0x3<<2)) | (0x0<<2);    //sys_clk switch to RC
            asm("nop");
            asm("nop");
            asm("nop");
            rtccon  |= BIT(8);                              //RTC alarm wakeup sleep enable
        }
        if(wkcfg->source & WK_PWROFF_RTC1S){
            RTCCPND  = BIT(18);                             //RTC wakeup pending
            #if !PWR_RTC1S_WK_MODE
            rtccon  |= BIT(7);                              //RTC1s wakeup enable
            #endif
            RTCCON0 |= BIT(0);                              //RC enable
            RTCCON0  = (RTCCON0 & ~(0x3<<8)) | (0x1<<8);    //clk2m_rtc sel
            RTCCON2  = PWR_RTC1S_WK_TIME;                   //timer PR
            RTCCNT   = 0;
        }
        if(wkcfg->source & WK_PWROFF_GPIO){
            // my_printf("WK_PWROFF_GPIO\n");
            wk_gpio_pin_init_do(wkcfg);                     //开机IO初始化上下拉配置

            #if MUL_PWRON_IO_EN
                mul_pwron_set();
            #endif

            if(MCLR_S_RESET){
                GPIOADE |= BIT(11);
                GPIOAPU |= BIT(11);
            }

            #if SPIFLASH_DP_PDN4_EN
            spiflash0_enter_dp();
            #endif // SPIFLASH_DP_PDN4_EN

            if(VERSIONID != 0){             //fix 169s_v3 关机功耗高，v1异常
                GPIOGDE  = 0;
                GPIOGFEN = 0;
                GPIOGPU  = 0;
            }

            wk_gpio_wakeup_init_do(wkcfg);                  //开机IO初始化wakeup唤醒配置

            #if MUL_PWRON_IO_EN
                mul_pwron_int_set();
            #endif
        }
        if(wkcfg->lp_type == LP_DEEP_SLEEP){
            rtccon |= BIT(5);       //when wakeup ,will reset system
        }else{
            rtccon &= ~BIT(5);
        }

        RTCCON0 |=  BIT(31);            //power_flag
        CLKGAT0 &= ~BIT(15);            //disable lp_top clock

        if(wkcfg->pdn_io_interface_flag){
            RTCCON0 &= ~BIT(26);        //IO interface disable, io控制寄存器无法写
        }

        rtccon  &= ~BIT(0);         //关机选择RTCCON1 BIT8~6
        RTCCON  = rtccon;
    }
}

AT(.text.lowpwr.pwroff)
void pwroff_do(void)
{
    #if(PWR_L1M == POWER_DOWN_MODE)
        pdn1();
    #elif(PWR_L2M == POWER_DOWN_MODE)
        pdn2();
    #elif(PWR_L3M == POWER_DOWN_MODE)
        pdn3();
    #elif(PWR_L4M == POWER_DOWN_MODE)
        pdn4();
    #elif(PWR_RTC1S == POWER_DOWN_MODE)
        pdn_rtc1s();
    #elif(PWR_RUNRC == POWER_DOWN_MODE)
        pdn_runrc();
    #elif(PWR_ALARM == POWER_DOWN_MODE)
        pdn_alarm();
    #elif(PWR_L3M_ALM == POWER_DOWN_MODE)
        pdn_l3m_alm();
    #elif(PWR_L4M_ALM == POWER_DOWN_MODE)
        pdn_l4m_alm();
    #endif
}

#if (!SOFT_POWER_ON_OFF) || MIX_PWR_DOWN_EN
ALIGNED(512)
AT(.text.lowpwr.lowbat)
void sfunc_lowbat_do(void)
{
    sp_sfr.piccon   = PICCON;
    sp_sfr.picen    = PICEN;
    sp_sfr.picpr    = PICPR;
    sp_sfr.nmicon   = NMICON;
    sp_sfr.clkgat0  = CLKGAT0;
    sp_sfr.clkgat1  = CLKGAT1;

    sp_sfr.pwrcon0  = PWRCON0;
    sp_sfr.pwrcon1  = PWRCON1;

    sp_sfr.pll0con  = PLL0CON;
    sp_sfr.pll0div  = PLL0DIV;

    sp_sfr.clkcon0  = CLKCON0;
    sp_sfr.clkcon1  = CLKCON1;
    sp_sfr.clkdivcon0 = CLKDIVCON0;

    sp_sfr.funcoutmcon  = FUNCOUTMCON;
    sp_sfr.funcoutcon   = FUNCOUTCON;
    sp_sfr.funcincon    = FUNCINCON;
    sp_sfr.funcomcon[0] = FUNCMCON0;
    sp_sfr.funcomcon[1] = FUNCMCON1;

    sp_sfr.udetbaud = UDETBAUD;
    sp_sfr.udetcon  = UDETCON;
    sp_sfr.adc_ch   = adc_cb.channel;

    sp_sfr.gpioa_fen = GPIOAFEN;
    sp_sfr.gpiog_fen = GPIOGFEN;
    sp_sfr.gpioa_dir = GPIOADIR;
    sp_sfr.gpiog_dir = GPIOGDIR;
    sp_sfr.gpioa_de  = GPIOADE;
    sp_sfr.gpiog_de  = GPIOGDE;
    sp_sfr.gpioa_drv = GPIOADRV;
    sp_sfr.gpiog_drv = GPIOGDRV;
    sp_sfr.gpioa_pu  = GPIOAPU;
    sp_sfr.gpiog_pu  = GPIOGPU;
    sp_sfr.gpioa_pd  = GPIOAPD;
    sp_sfr.gpiog_pd  = GPIOGPD;
#if 0   //使能会导致mic bias抖动一下
    dac_power_off();                                //dac power down
    sdadc_analog_mic_exit(CH_MIC_PA0);
    pwroff_mic_bias_on();
#endif
    saradc_exit(adc_cb.channel);                    //close saradc及相关通路模拟

    RTCCON12 |= 3<<4;    //gpio reset disable
    PICCONCLR = BIT(0); //关总中断
    WDT_DIS();

    //唤醒源选择 RTC1S RWDT ALM WKO PORT
    memset(&wk_cfg, 0, sizeof(pwroff_wk_t));
    wk_cfg.source = WK_PWROFF_GPIO;
    #if MIX_PWR_DOWN_EN
        wk_cfg.wk_fall_en = 1;
        wk_cfg.fall_io_num = 1;
        wk_cfg.fall_io_group[0] = MIX_PWR_SLEEP_UP_IO;
    #else
    if(POWER_ON_FALL_NUM != 0){
        u8 fall_io_temp[POWER_ON_FALL_NUM] = POWER_ON_FALL_IO;
        wk_cfg.wk_fall_en = 1;
        wk_cfg.fall_io_num = POWER_ON_FALL_NUM;
        memcpy(wk_cfg.fall_io_group, fall_io_temp, POWER_ON_FALL_NUM);
    }
    if(POWER_ON_RISE_NUM != 0){
        u8 rise_io_temp[POWER_ON_RISE_NUM] = POWER_ON_RISE_IO;
        wk_cfg.wk_rise_en = 1;
        wk_cfg.rise_io_num = POWER_ON_RISE_NUM;
        memcpy(wk_cfg.rise_io_group, rise_io_temp, POWER_ON_RISE_NUM);
    }
    #endif
    pwroff_wk_source_config(&wk_cfg);

    PWRCON1 &= ~0xF;    //disable flash PG
    CLKCON0 &= ~0xC;    //SYS clock switch to RC
    asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");
    PLL0CON = 0;        //disable PLL
    PWRCON0 = (PWRCON0 & ~(0x1f<<0)) | (0x8<<0);    //DI_VCORES 0x13 = 1.275V

    LPMCON |= BIT(0);                           //sleep mode
    //LPMCON |= BIT(1);                           //idle mode 此bit无意义
    asm("nop");asm("nop");asm("nop");

//唤醒后的处理
    PWRCON0     = sp_sfr.pwrcon0;

    PWRCON1 &= ~(0xf << 0);                         //软开机时，关掉flash power gate
    PWRCON1    |= BIT(4);                           //VDDF pulldown
    asm("nop"); asm("nop"); asm("nop"); asm("nop"); //至少4us
    asm("nop"); asm("nop"); asm("nop"); asm("nop");
    asm("nop"); asm("nop"); asm("nop"); asm("nop");
    asm("nop"); asm("nop"); asm("nop"); asm("nop");
    PWRCON1 &= ~BIT(4);                             //flash power gate pull down disable
    PWRCON1 |= BIT(0) | BIT(1) | BIT(2);
    asm("nop"); asm("nop"); asm("nop"); asm("nop");
    PWRCON1 |= BIT(3);

    //第一时间拉高MCP Flash的CS脚，以免CLK翻转影响SPIFlash
    SPI_CS1_DIS();
    SPI_CS1_PDIR &= ~BIT(SPI_CS1);

    CLKGAT0     = sp_sfr.clkgat0;
    CLKGAT1     = sp_sfr.clkgat1;

    PLL0DIV     = sp_sfr.pll0div;
    PLL0CON     = sp_sfr.pll0con;
    PLL0CON    |= BIT(20);                          //update pll0div to pll_clk
    PLL0CON    |= BIT(18);                          //pll sdm enable

    GPIOAFEN    = sp_sfr.gpioa_fen;
    GPIOADIR    = sp_sfr.gpioa_dir;
    GPIOADE     = sp_sfr.gpioa_de;
    GPIOADRV    = sp_sfr.gpioa_drv;
    GPIOAPU     = sp_sfr.gpioa_pu;
    GPIOAPD     = sp_sfr.gpioa_pd;

    GPIOGFEN    = sp_sfr.gpiog_fen;
    GPIOGDIR    = sp_sfr.gpiog_dir;
    GPIOGDE     = sp_sfr.gpiog_de;
    GPIOGDRV    = sp_sfr.gpiog_drv;
    GPIOGPU     = sp_sfr.gpiog_pu;
    GPIOGPD     = sp_sfr.gpiog_pd;

    CLKDIVCON0  = sp_sfr.clkdivcon0;
    CLKCON1     = sp_sfr.clkcon1;
    asm("nop");
    asm("nop");
    asm("nop");
    CLKCON0     = (sp_sfr.clkcon0 & (~0x0c));
    asm("nop");
    asm("nop");
    asm("nop");
    CLKCON0 = sp_sfr.clkcon0;

    LVDCON = (LVDCON & ~0x07) | 1;                  //LVD=2.2V,  LVDCON[2:0]
    PICADR = (u32)&__comm_vma;
#if SYS_BRAM_EN
    #if (SYS_BRAM_LEN == 0x1000)
    ICLOCK = 0xff;          //cache预留4K空间用作bram
    #elif (SYS_BRAM_LEN == 0x2000)
    ICLOCK = 0xffff;        //cache预留8K空间用作bram
    #endif
#endif
    CACHCON0 = BIT(16) | BIT(0);                    //dCache Enable, iCache Enable

    EFCON0 |= BIT(3) | BIT(2);                      //EF WP
    MEMCON |= BIT(16);                              //MROM Protect

    SPI0BAUD = 1;                                   //RC CLK / (1 + 1) = 1M
    SPI0CON = BIT(0);                               //SPI EN

    FUNCOUTMCON = sp_sfr.funcoutmcon;
    FUNCOUTCON  = sp_sfr.funcoutcon;
    FUNCINCON   = sp_sfr.funcincon;
    FUNCMCON0   = sp_sfr.funcomcon[0];
    FUNCMCON1   = sp_sfr.funcomcon[1];

    UDETBAUD    = sp_sfr.udetbaud;
    UDETCON     = sp_sfr.udetcon;
    UDETCPND = BIT(10) | BIT(15) | BIT(18);          //CLR Pending
    UDETCON = (UDETCON & ~(0xfff << 16)) | (0x5aa << 16);  //KEY MODE RESET

    PICCON      = sp_sfr.piccon;
    PICPR       = sp_sfr.picpr;
    PICEN       = sp_sfr.picen;
    NMICON      = sp_sfr.nmicon;

    PORTINTEDG  = 0;
    PORTINTEN   = 0;
    WKUPCON &= ~(BIT(6) | BIT(7));
    WKUPCPND = BIT(22);                             //clear 6 pending
    WKUPCPND =  BIT(23);			                //clear 7 pending
    WKUPCON &= ~BIT(17);                            //wakup sniff enable

    spiflash_init(0x0b, 1);

    timer0_init();
    PICCON |= BIT(0);                               //global interrupt enable
    WDT_EN();

    adc_cb.channel = sp_sfr.adc_ch;
    bsp_saradc_init(adc_cb.channel);
    sys_set_tmr_enable(1);
}
#endif

#if MIX_PWR_DOWN_EN
AT(.text.lowpwr.mixpwr)
void mix_pwr_down(u8 pwr_mode)
{
    printf("%s\n", __func__);
    u32 io_sta;
    io_sta = GPIOA;
    if(pwr_mode){
        while ((io_sta & BIT(MIX_PWR_SLEEP_IO-1)) == 0){     //等待按键松开
            io_sta = GPIOA;
            delay_5ms(1);
            WDT_CLR();
        }
        sfunc_lowbat();
    }else{
#if MIX_PWR_DOWN_EN
        while ((io_sta & BIT(MIX_PWR_DOWN_IO-1)) == 0){     //等待按键松开
            io_sta = GPIOA;
            delay_5ms(1);
            WDT_CLR();
        }
#endif // MIX_PWR_DOWN_EN
        memcpy(&__pdn_comm_vma, &__pdn_comm_lma, (u32)&__pdn_comm_size);//复用aram
        sfunc_pwroff();
    }
    func_cb.sta = func_cb.last;
    func_cb.last = FUNC_NULL;
}
#endif

//硬开关方案，低电时，进入省电状态
AT(.text.lowpwr.lowbat)
void sfunc_lowbat(void)
{
    memcpy(&__pdn_comm_vma, &__pdn_comm_lma, (u32)&__pdn_comm_size);//复用aram
    memset(&wk_cfg, 0, sizeof(pwroff_wk_t));
#if (POWER_DOWN_MODE == PWR_L4M) || (POWER_DOWN_MODE == PWR_L4M_ALM) || (!SOFT_POWER_ON_OFF) || (POWER_DOWN_MODE == PWR_RTC1S) || MIX_PWR_DOWN_EN
    memset(&sp_sfr, 0, sizeof(sleep_sfr_t));
#endif
#if (!SOFT_POWER_ON_OFF) || MIX_PWR_DOWN_EN
    printf("%s\n", __func__);
    set_sys_clk(SYS_48M);
    PWRCON0 = (PWRCON0 & ~(0x1f<<0)) | (0x10<<0);       //DI_VCORES 0x10 = 1.2V
    sfunc_lowbat_do();
#endif
}

AT(.text.lowpwr.pwroff)
void sfunc_pwroff(void)
{
    memcpy(&__pdn_comm_vma, &__pdn_comm_lma, (u32)&__pdn_comm_size);//复用aram
    memset(&wk_cfg, 0, sizeof(pwroff_wk_t));
#if (POWER_DOWN_MODE == PWR_L4M) || (POWER_DOWN_MODE == PWR_L4M_ALM) || (!SOFT_POWER_ON_OFF) || (POWER_DOWN_MODE == PWR_RTC1S) || MIX_PWR_DOWN_EN
    memset(&sp_sfr, 0, sizeof(sleep_sfr_t));
#endif
#if EX_SPIFLASH_SUPPORT
        spiflash1_power_down();
#endif
    set_sys_clk(SYS_24M);
    PWRCON0 = (PWRCON0 & ~(0x1f<<0)) | (0x10<<0);       //DI_VCORES 0x10 = 1.2V
    pwroff_do();
}

AT(.text.lowpwr.pwroff)
void func_pwroff(int pwroff_tone_en)
{
    printf("%s\n", __func__);
#if !MUL_PWRON_IO_EN
    u32 io_sta;
    u8 wait = 1;
    /*-------------------------------------------------------------*/
   //此段判断,仅为下降沿关机IO使用,作用是避免按键未松开就进行关机流程
   //如要使用上升沿唤醒IO进行关机,要修改此部分判断,避免上升沿未松开就进入关机
#if (POWER_OFF_IO != 0)
    u16 gpio_fen, gpio_de, gpio_pu;
    gpio_fen = GPIOAFEN;
    gpio_de  = GPIOADE;
    gpio_pu  = GPIOAPU;
    GPIOAFEN &= ~BIT(POWER_OFF_IO-1);                   //设置IO状态
    GPIOADE  |=  BIT(POWER_OFF_IO-1);
    GPIOAPU  |=  BIT(POWER_OFF_IO-1);
    io_sta = GPIOA;
    if (wait) {
        while ((io_sta & BIT(POWER_OFF_IO-1)) == 0){    //等待按键松开
            io_sta = GPIOA;
            delay_5ms(1);
            WDT_CLR();
        }
    }
    GPIOAFEN = gpio_fen;                                //还原IO状态
    GPIOADE  = gpio_de;
    GPIOAPU  = gpio_pu;
#endif
    /*-------------------------------------------------------------*/
#endif
    if (SOFT_POWER_ON_OFF) {
        sfunc_pwroff();
    } else {
        sfunc_lowbat();                                 //低电关机进入Sleep Mode
    }
    //sleep->wakeup
    func_cb.sta = func_cb.last;
    func_cb.last = FUNC_NULL;
}


