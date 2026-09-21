#include "include.h"

volatile int pwrkey_detect_flag;            //pwrkey 820K用于复用检测的标志。
uint8_t dac_en = DAC_EN;
uint8_t mic_bias_en;
uint8_t mic_bias_sel;
uint8_t mic_bres_sel;
uint8_t pin_sop_sel;
uint8_t dac_soft_gain   = 0;
uint8_t dac_drc_en      = DAC_DRC_EN;
uint8_t lvd_irq_en      = LVD_IRQ_EN;

void plugin_init(void)
{
#if (LANG_SELECT == LANG_EN_ZH)
    multi_lang_init(sys_cb.lang_id);
#endif

    mic_bias_en     = xcfg_cb.mic_bias_en;
    mic_bias_sel    = xcfg_cb.mic_bias_sel;
    mic_bres_sel    = MIC_BRES_SEL;
    dac_soft_gain   = DAC_MAX_GAIN;

#if (PWR_L4M == POWER_DOWN_MODE)
    if(0 == VERSIONID) {
        printf("CHIP_V1 DO NOT SUPPORT PWR_DOWN_L4M!\n");
    }
#endif
}

void plugin_var_init(void)
{

#if LED_DISP_EN
	led_func_init();
#endif // LED_DISP_EN
}

AT(.com_text.port.key)
void plugin_saradc_sel_channel(u16 *adc_ch)
{
#if USER_KEY_KNOB_EN
    u8 mic_vol_val = (u8)(adc_cb.sfr[ADCCH_PE5] >> 2);
    u8 echo_level_val = (u8)(adc_cb.sfr[ADCCH_PE6] >> 2);

    static u8 echo_level = USER_KEY_KNOB_LEVEL, mic_vol = USER_KEY_KNOB_LEVEL;

    key_knob_process(echo_level_val, tbl_knob_level_16, &echo_level);
    if (echo_level != sys_cb.echo_level) {
        sys_cb.echo_level = echo_level;
        msg_enqueue(EVT_ECHO_LEVEL);
    }

    key_knob_process(mic_vol_val, tbl_knob_level_16, &mic_vol);
    if (mic_vol != sys_cb.mic_vol) {
        sys_cb.mic_vol = mic_vol;
        msg_enqueue(EVT_MIC_VOL);
    }
#endif
}

void plugin_saradc_init(u16 *adc_ch)
{

}

AT(.com_text.bsp.sys)
bool is_sd_support(void)
{
    return 0;
}

void plugin_lowbat_vol_reduce(void)
{

}

void plugin_lowbat_vol_recover(void)
{

}

AT(.com_text.port.vbat)
void plugin_vbat_filter(u32 *vbat)
{
#if  VBAT_FILTER_USE_PEAK
    //电源波动比较大的音箱方案, 取一定时间内的电量"最大值"或"次大值",更能真实反应电量.
    #define VBAT_MAX_TIME  (3000/5)   //电量峰值判断时间 3S
    static u16 cnt = 0;
	static u16 vbat_max_cnt = 0;
    static u32 vbat_max[2] = {0,0};
    static u32 vbat_ret = 0;
    u32 vbat_cur = *vbat;
    if (cnt++  < VBAT_MAX_TIME) {
        if (vbat_max[0] < vbat_cur) {
            vbat_max[1] = vbat_max[0];  //vbat_max[1] is less max (次大值)
            vbat_max[0] = vbat_cur;     //vbat_max[0] is max(最大值)
            vbat_max_cnt = 0;
        } else if (vbat_max[0] == vbat_cur) {
            vbat_max_cnt ++;
        }
    } else {
        if (vbat_max_cnt >= VBAT_MAX_TIME/5) {  //总次数的(1/5)都采到最大值,则返回最大值.
            vbat_ret = vbat_max[0];
        } else if (vbat_max[1] != 0) {   //最大值次数较少,则返回次大值(舍弃最大值)
            vbat_ret = vbat_max[1];
        }
        vbat_max[0] = 0;
        vbat_max[1] = 0;
        vbat_max_cnt = 0;
        cnt = 0;
    }
    //返回值
    if (vbat_ret != 0) {
        *vbat = vbat_ret;
    }
#endif
}

#if LVD_IRQ_EN

#define LVD_RESET_EN        1                           //0-关闭LVD复位, 1-打开LVD复位
#define LVD_INT_EN          0                           //0-关闭LVD中断, 1-打开LVD中断
#define LVD_RESET_SEL       xcfg_cb.reset_sel          	//复位电压选择

AT(.com_text.isr) FIQ
void lvd_isr(void)
{
    if (LVDCON & BIT(7)) {              //LVD pending
        CRSTPND = BIT(7);               //Clear LVD pending

        //此处放置关机前需要处理的代码,尽量简洁,不能处理过于复杂的
        GPIOAFEN &= ~BIT(8);            //test
        GPIOADE  |=  BIT(8);
        GPIOADIR &= ~BIT(8);
        GPIOASET = BIT(8);
        asm("nop");asm("nop");asm("nop");
        GPIOACLR = BIT(8);

        LVDCON = (LVDCON & ~0x07) | 2;  //关机前还原电压设置
    }
}

//设置触发LVD中断电压
void lvd_level_set(void)
{
#if LVD_RESET_EN
    LVDCON |=  BIT(5);                  //LVD reset enable
#else
    LVDCON &= ~BIT(5);                  //LVD reset disable
#endif
#if LVD_INT_EN
    LVDCON |=  BIT(6);                  //LVD interrupt enable
#else
    LVDCON &= ~BIT(6);                  //LVD interrupt disable
#endif
    LVDCON = (LVDCON & ~0x07) | LVD_RESET_SEL;      //0-1.8V, 1-2V, 2-2.2V, 3-2.4V, 4-2.6V, 5-2.8V, 6-3V, 7-1.7V
}
#else
AT(.com_text.isr) FIQ
void lvd_isr(void) {}
#endif
