#include "include.h"
#define VBG_VOLTAGE             600

static psfr_t sfr1;

void e_saradc_init(u16 ch)
{
    if (ch == 0) {
        return;
    }
    sfr1 = (psfr_t)(&SADCDAT0);           //SADCDAT SFR

    if (ch & BIT(ADCCH_VBAT)) {
        PWRCON0  |= BIT(10);                        //VBATDIV2 enable
    }
    //BG
    if (ch & BIT(ADCCH_ANA)) {
        RTCCON1  |= BIT(24);                        //VBGPWRT = 1
        PWRCON0  |= BIT(12);                        //BG output enable

    }
    CLKCON0 = (CLKCON0 & ~(3 << 28)) | (1 << 28);   //select saradc_clk  x26m_clkdiv4
    SADCBAUD  = 0x03;                               //Fadc clock / [2(SADCBAUD+1)] , SARADC bit clock maximum is 1MHz
    SADCST    = 0xffffffff;                         //延时采样配置
    SADCCON   = BIT(19);                            //hardware auto control analog adc enable
    SADCCON  |= BIT(16);                            //saradc enable
}

bool e_get_adc_val_test(void)
{
    if (!is_saradc_convert_finish()) {
        return false;
    }
    return true;

}

u16 e_get_saradc_data(u16 ch)
{
    if (!ch) {
        return 0;
    }
    SADCCON &= ~BIT(18);

    SADCCON |= BIT(16);                              //saradc enable
    SADCCH  = 0;                                    //clear pending
    SADCCH  |= BIT(ch);

    while(!e_get_adc_val_test());

    return (u8)(sfr1[ch] >> 2);
}

//saradc值转成电压mv单位示例
void e_saradc_proc(void)
{
    my_printf("%s,%d\n",__func__,__LINE__);
    u16 adc_bg, adc_val;
    static u16 adc_ch = 0;

    adc_ch |= BIT(ADCCH_ANA);
    adc_ch |= BIT(ADCCH_PA5);
    e_saradc_init(adc_ch);

    adc_bg = e_get_saradc_data(ADCCH_ANA);
    adc_val = e_get_saradc_data(ADCCH_PA5);

#if 0
    while(1) {
        WDT_CLR();
        static u32 ticks = 0;
        if (tick_check_expire(ticks,1000)) {
            ticks = tick_get();
            adc_bg = e_get_saradc_data(ADCCH_ANA);
            adc_val = e_get_saradc_data(ADCCH_PA5);

            my_printf("adc_bg %d\n",adc_bg);
            my_printf("adc_val %d\n",adc_val);
            my_printf("cur_mv %d\n",adc_val * VBG_VOLTAGE / adc_bg);
        }
    }
#endif // 0
        my_printf("adc_bg %d\n",adc_bg);
        my_printf("adc_val %d\n",adc_val);
        my_printf("cur_mv %d\n",adc_val * VBG_VOLTAGE / adc_bg);
}
