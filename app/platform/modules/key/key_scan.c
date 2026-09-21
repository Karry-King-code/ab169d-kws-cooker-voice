#include "include.h"

#define VBG_VOLTAGE             1200
#define VBAT_RESET_VAL_OFS      200       //电池电压与vddio联动调节差值,相差0.1V就下调vddio电压

extern u32 __pdn_comm_vma, __pdn_comm_lma, __pdn_comm_size;

u16 vbat_reset_val_ofs = VBAT_RESET_VAL_OFS;

adc_cb_t adc_cb AT(.buf.key.cb);
#if USER_MULTI_PRESS_EN
u8 dblkey_num[5] AT(.buf.key);
const u16 tbl_double_key[] = USER_KEY_DOUBLE;
#elif TKEY_SLIDE_DET_EN
u8 dblkey_num[5] AT(.buf.key);
const u16 tbl_double_key[] = TKEY_SLIDE_BUF ;
#endif

#if USER_ADKEY
u8 adkey_num[13] AT(.buf.key);
// adkey_typec_tbl_t key_tbl_buf[4];
#endif

bool get_adc_val(void);
u16 get_vbat_val(void);
u32 getcfg_reset_sel(void);
u16 getcfg_vddio_val_ofs(void);
void vddio_voltage_update(u16 vbat_val);
u16 get_cur_vddio_vol(void);
extern bool tkey_finish_flag;

AT(.com_rodata.key.table)
const key_shake_tbl_t key_shake_table = {
    .scan_cnt = KEY_SCAN_TIMES,
    .up_cnt   = KEY_UP_TIMES,
    .long_cnt = KEY_LONG_TIMES,
    .hold_cnt = KEY_LONG_HOLD_TIMES,
};

AT(.text.init)
void bsp_saradc_init(u16 adc_ch)
{
    u8 bg;
//    printf("%s: %04x\n", __func__, adc_ch);
    if (adc_ch == 0) {
        return;
    }
    bg = adc_cb.bg;
    memset(&adc_cb, 0, sizeof(adc_cb));
    adc_cb.bg = bg;
    adc_cb.sfr = (psfr_t)(&SADCDAT0);           //SADCDAT SFR
    adc_cb.channel = adc_ch;
    saradc_init(adc_cb.channel);

#if USER_ADKEY
    adc_cb.key_val = 0xff;
#endif // USER_ADKEY

#if USER_ADKEY2
    adc_cb.key2_val = 0xff;
#endif // USER_ADKEY2

#if (VBAT_DETECT_EN || USER_ADKEY)
    saradc_kick_start(adc_cb.channel, USER_ADKEY_MUX_LED);
#endif

#if VBAT_DETECT_EN
    while(!get_adc_val());                  //获取一次初值，再kick一次
    adc_cb.vbat_val =  (uint)adc_cb.vbat2 * VBG_VOLTAGE / adc_cb.bg;
    adc_cb.vbat_total = adc_cb.vbat_val << 5;
    sys_cb.vbat = get_vbat_val();
#endif // VBAT_DETECT_EN

#if VDDIO_FOLLOW_VBAT_EN
    if (adc_cb.vbat_val < getcfg_vddio_val_ofs()) {
        return;
    }
#endif

#if LOW_VBAT_POWER_ON
    //adc_cb.channel &= ~BIT(ADCCH_ANA);     //不关闭adc_bg采集
    //PWRCON0  &= ~BIT(12);
#else
    adc_cb.channel &= ~BIT(ADCCH_ANA);     //只进行一次BG ADC采集
    PWRCON0  &= ~BIT(12);                   //BG output enable
#endif // LOW_VBAT_POWER_ON
}

//AT(.com_rodata.bb)
//const char str_adc_v[] = "SADCDAT: %02x\n";

#if TSENSOR_TEST_EN
AT(.com_rodata.bb)
const char str_tsensor_v[] = "ADCDAT: %x\n";
#endif

#if TYPEC_GET_PB1_AD_VAL
AT(.com_text.port.key)
u8 get_io_adval(void)
{
    u8 io_val = 0;
    io_val = (u8)(adc_cb.sfr[ADCCH_PB1] >> 2);
    return io_val;
}

static u32 PB1_ad_tick = 0;
static u8 exsert_cnt = 0;
AT(.com_text.port.key)
void phin_det_io_proc(void)
{
    if(tick_check_expire(PB1_ad_tick, 100)){
        PB1_ad_tick = tick_get();
        u8 io_val = get_io_adval();
        // printf("io_val 1: %d\n", io_val);
        if(io_val > 200){
            exsert_cnt++;
            if(exsert_cnt >= 3){
                exsert_cnt = 0;
                msg_enqueue(EVT_PC_REMOVE);
            }
        }else{
            exsert_cnt = 0;
        }
    }
}
#endif

#if (VBAT_DETECT_EN || USER_ADKEY)
AT(.com_text.port.key)
bool get_adc_val(void)
{
    u16 channel;

    if (!is_saradc_convert_finish()) {
        return false;
    }

#if USER_ADKEY
    if (xcfg_cb.user_adkey_en) {
        adc_cb.key_val = (u8)(adc_cb.sfr[ADKEY_CH] >> 2);
    }
#endif // USER_ADKEY

#if USER_ADKEY2
    adc_cb.key2_val = (u8)(adc_cb.sfr[ADKEY2_CH] >> 2);
#endif // USER_ADKEY2

#if USER_ADKEY_MUX_SDCLK
    if (xcfg_cb.user_adkey_mux_sdclk_en) {
        adc_cb.sdclk_valid = 0;
        if (adc_cb.sdclk_convert) {
            adc_cb.sdclk_val = (u8)(adc_cb.sfr[SDCLK_AD_CH] >> 2);
            adc_cb.sdclk_valid = 1;
        }
    }
#endif

    if (adc_cb.channel & BIT(ADCCH_ANA)) {
        adc_cb.bg = (u8)(adc_cb.sfr[ADCCH_ANA] >> 2);
    }
#if VBAT_DETECT_EN
    adc_cb.vbat2 = (u8)(adc_cb.sfr[VBAT2_ADCCH] >> 2);
#endif // VBAT_DETECT_EN


    plugin_saradc_sel_channel(&adc_cb.channel);      //其他的ADC通道
    channel = adc_cb.channel;

#if USER_ADKEY_MUX_SDCLK
    if (xcfg_cb.user_adkey_mux_sdclk_en) {
        adc_cb.sdclk_convert = 1;
        if (is_det_sdcard_busy()) {
            channel &= ~BIT(SDCLK_AD_CH);
            adc_cb.sdclk_convert = 0;
        }
    }
#endif // USER_ADKEY_MUX_SDCLK

    saradc_kick_start(channel, USER_ADKEY_MUX_LED);
//    printf(str_adc_v, adc_cb.key_val);
    return true;
}
#endif // (VBAT_DETECT_EN || USER_ADKEY)

#if LOW_VBAT_POWER_ON
bool get_low_power_adc_val(void)
{
    u16 channel;

    if (!is_saradc_convert_finish()) {
        return false;
    }

    if (adc_cb.channel & BIT(ADCCH_ANA)) {
        adc_cb.bg = (u8)(adc_cb.sfr[ADCCH_ANA] >> 2);
    }

    adc_cb.vbat2 = (u8)(adc_cb.sfr[VBAT2_ADCCH] >> 2);


    plugin_saradc_sel_channel(&adc_cb.channel);      //其他的ADC通道
    channel = adc_cb.channel;

    saradc_kick_start(channel, USER_ADKEY_MUX_LED);
    return true;
}
#endif // LOW_VBAT_POWER_ON

#if USER_ADKEY_MUX_SDCLK
AT(.com_text.port.key)
void adc_mux_sdclk_w4_convert(void)
{
    if ((xcfg_cb.user_adkey_mux_sdclk_en) && (adc_cb.sdclk_convert)) {
        while (!is_saradc_convert_finish()) {
            WDT_CLR();
        }
    }
}
#endif // USER_ADKEY_MUX_SDCLK

#if (USER_ADKEY || USER_ADKEY_MUX_SDCLK)
//AT(.com_text.str)
//const char str_1[] = "key_val:%d\n";
AT(.com_text.port.key)
static u8 get_adkey(u8 key_val, u8 key_config_en)
{
    u8 num = 0;

    //配置工具是否使能
    if (!key_config_en) {
        return NO_KEY;
    }

    //my_printf(str_1,key_val);
    while (key_val > adkey_table[num].adc_val) {
        num++;
    }
    if (xcfg_cb.adkey_config_en) {
        return adkey_num[num];
    }
    return adkey_table[num].usage_id;
}
#endif // USER_ADKEY

#if USER_ADKEY2
AT(.com_text.port.key)
static u8 get_adkey2(void)
{
    u8 num = 0;

    //配置工具是否使能了第二组ADKEY2？
    if (!xcfg_cb.user_adkey2_en) {
        return NO_KEY;
    }

    while (adc_cb.key2_val > adkey2_table[num].adc_val) {
        num++;
    }
    return adkey2_table[num].usage_id;
}
#endif // USER_ADKEY2

#if VBAT_DETECT_EN
extern u32 vio_fkick;

//AT(.com_rodata.bat)
//const char bat_str[] = "VBAT: %d.%03dV\n";
AT(.com_text.port.vbat)
u16 get_vbat_val(void)
{
    static u16 vbat_bak = 0;
    u32 vbat = (u32)adc_cb.vbat2 * VBG_VOLTAGE / adc_cb.bg;
    //不同方案可能采用不同 vbat 滤波算法, 在方案对应的plugin.c中处理
    plugin_vbat_filter(&vbat);
    //默认的取平均值算法.
    adc_cb.vbat_total = adc_cb.vbat_total - adc_cb.vbat_val + vbat; //均值
    adc_cb.vbat_val = adc_cb.vbat_total>>5;

    if(adc_cb.vbat_val > vbat_bak) {
        vbat = adc_cb.vbat_val - vbat_bak;
    } else {
        vbat = vbat_bak - adc_cb.vbat_val;
    }

    if (vbat >= 10) {   //偏差大于一定值则更新
#if VDDIO_FOLLOW_VBAT_EN
        adc_cb.channel |= BIT(ADCCH_ANA);
        PWRCON0  |= BIT(12);
        vio_fkick = tick_get();
#endif
        vbat_bak = adc_cb.vbat_val;
//        printf(bat_str, adc_cb.vbat_val/1000, adc_cb.vbat_val%1000);
    }
//    printf(bat_str, adc_cb.vbat_val/1000, adc_cb.vbat_val%1000);

    return vbat_bak;
}

int is_lowpower_vbat_warning(void)
{
    if (sys_cb.vbat <= ((u16)LPWR_OFF_VBAT*100+1800)) {
        if (LPWR_OFF_VBAT) {
            if (!sys_cb.lpwr_cnt) {
                sys_cb.lpwr_cnt = 1;
            } else if (sys_cb.lpwr_cnt >= 10) {
                return 1;       //VBAT低电关机
            }
        }
        return 0;               //VBAT低电不关机
    }
//#if WARNING_LOW_BATTERY
    else {
        sys_cb.lpwr_cnt = 0;
        if (sys_cb.vbat < ((u16)LPWR_WARNING_VBAT*100 + 1900)) {
    #if LED_LOWBAT_EN
            if (xcfg_cb.rled_lowbat_en) {
                if ((!RLED_LOWBAT_FOLLOW_EN)) {
                    led_lowbat();
                } else {
                    led_lowbat_recover();
                }
            }
    #endif // LED_LOWBAT_EN
            if (xcfg_cb.lowpower_warning_en) {
                return 2;       //低电压提示音播报
            }
        } else {
    #if LED_LOWBAT_EN
            if (xcfg_cb.rled_lowbat_en) {
                led_lowbat_recover();
            }
    #endif // LED_LOWBAT_EN
        }
        return 0;
    }
//#endif // WARNING_LOW_BATTERY
    sys_cb.lpwr_cnt = 0;
    return 0;
}
#endif  //VBAT_DETECT_EN

AT(.text.bsp.power)
bool is_powron_frist_enable(u32 rtccon9)
{
    if (!PWRON_FRIST_BAT_EN) {
        return false;
    }
    if (rtccon9 & BIT(7)) {                                        //GPIO wakeup不能直接开机
        return false;
    }

    return true;
}

AT(.text.bsp.power)
void power_on_check(void)
{
    int up_cnt = 0, ticks = 0;
    int pwrkey_pressed_flag = 0;
    u32 pwron_press_nms;
    bool bo_mulpwr = false;
    u8 key_val = 0;

    u32 rtccon9 = RTCCON9;                          //wakeup pending
    // u32 rtccon10 = RTCCON10;
    // printf("power_on_check: %08x, %08x\n", rtccon9, rtccon10);
    // printf("RTCCON: %X, RTCCON0: %X, RTCCON3: %x, RTCCON9: %x\n", RTCCON, RTCCON0, RTCCON3, RTCCON9);
    // printf("RTCCON10: %X, RTCCON12: %X\n", RTCCON10, RTCCON12);
#if(POWER_DOWN_MODE == PWR_RTC1S)
    if(RTCCON9 & BIT(1)){
        RTCCON3 |= 0X7;
    }
#endif
    RTCCON9  = 0xfff;                               //Clr pending
    RTCCON10 = 0x7fff;                              //Clr pending
    // RTCCPND  = 0xb0060000;                          //Clr RTC pending

    PORTINTEDG  = 0;
    PORTINTEN   = 0;
    WKUPCON &= ~(BIT(6) | BIT(7));
    WKUPCPND =  BIT(22);                            //clear 6 pending
    WKUPCPND =  BIT(23);			                //clear 7 pending
    WKUPCON &= ~BIT(17);                            //wakup sniff enable

#if LOW_VBAT_POWER_ON
    u32 vbat = (u32)adc_cb.vbat2 * VBG_VOLTAGE / adc_cb.bg;
	while(vbat < POWER_ON_MIN_VBAT){
	    WDT_CLR();
        if(get_low_power_adc_val()) {
            static u32 ticks = 0;

            if (tick_check_expire(ticks,100)) {
                ticks = tick_get();
                vbat = (u32)adc_cb.vbat2 * VBG_VOLTAGE / adc_cb.bg;
            }
        }

        if(vbat >= POWER_ON_MIN_VBAT) {
            break;
        }
	}
#endif // LOW_VBAT_POWER_ON

    if (!SOFT_POWER_ON_OFF) {
        return;
    }

#if MIX_PWR_DOWN_EN
    return;
#endif

    if (LVDCON & BIT(19)) {
        return;                                     //长按WKO 10S复位后直接开机。
    }

    pwron_press_nms = PWRON_PRESS_TIME;
    if (pwron_press_nms == 0) {
        pwron_press_nms = 15;                       //最小开机时间在100ms左右
    }

    //第一次上电是否直接开机
    if (is_powron_frist_enable(rtccon9)) {
        return;
    }

#if ((POWER_ON_FALL_NUM != 0) || (POWER_ON_RISE_NUM != 0))
    u16 gpioa_fen, gpioa_de, gpioa_pu, gpioa_pd;
    gpioa_fen = GPIOAFEN;
    gpioa_de  = GPIOADE;
    gpioa_pu  = GPIOAPU;
    gpioa_pd  = GPIOAPD;
    poweron_key_init();
#endif
    while (1) {
    WDT_CLR();
    delay_ms(5);

#if MUL_PWRON_IO_EN
    u8 mulpwr_press = get_mulpwr_key();
    if(mulpwr_press != 0){//有其中一个IO按下
        bo_mulpwr = true;
    }else{
        bo_mulpwr = false;
    }
#endif

#if ((POWER_ON_FALL_NUM != 0) || (POWER_ON_RISE_NUM != 0))
    key_val = get_poweron_key();
    if(key_val != 0){//有其中一个IO符合触发条件
        key_val = K_PWR;
    }
#endif
        if (((key_val & K_PWR_MASK) == K_PWR) ||  bo_mulpwr) {
            up_cnt = 0;
            if (!pwrkey_pressed_flag) {
                ticks = tick_get();
                sys_cb.ms_ticks = ticks;                                //记录PWRKEY按键按下的时刻
                pwrkey_pressed_flag = 1;
            }
            if (!sys_cb.poweron_flag) {
                if (tick_check_expire(ticks, pwron_press_nms)) {        //长按开机时间配置
                    sys_cb.poweron_flag = 1;
                }
            }
        } else {
            if (up_cnt < 3) {
                up_cnt++;
            }
            if (up_cnt == 3) {
                up_cnt = 10;
                pwrkey_pressed_flag = 0;
                sys_cb.poweron_flag = 0;
            }
        }

        if (sys_cb.poweron_flag) {
#if VBAT_DETECT_EN
            if (sys_cb.vbat <= 2950) {  //电压小于2.95v不开机
                continue;
            }
#endif
            break;
        } else {
            //PWKKEY松开，立刻关机
            if (!pwrkey_pressed_flag) {
                // saradc_exit(adc_cb.channel);
                memcpy(&__pdn_comm_vma, &__pdn_comm_lma, (u32)&__pdn_comm_size);//复用aram
                sfunc_pwroff();
            }
        }
#if MUL_PWRON_IO_EN
        msg_enqueue(mulpwr_press);
#endif
    }
#if ((POWER_ON_FALL_NUM != 0) || (POWER_ON_RISE_NUM != 0))
    GPIOAFEN = gpioa_fen;
    GPIOADE  = gpioa_de;
    GPIOAPU  = gpioa_pu;
    GPIOAPD  = gpioa_pd;
    poweron_key_exit();
#endif
}

AT(.text.bsp.key.init)
void key_init(void)
{
    u16 adc_ch = 0;

    key_var_init();

#if USER_IOKEY
    io_key_init();
#endif

#if USER_MULTI_PRESS_EN
    if (xcfg_cb.key_multi_config_en) {
        dblkey_num[0] = key_config_table[xcfg_cb.dblkey_num0];
        dblkey_num[1] = key_config_table[xcfg_cb.dblkey_num1];
        dblkey_num[2] = key_config_table[xcfg_cb.dblkey_num2];
        dblkey_num[3] = key_config_table[xcfg_cb.dblkey_num3];
        dblkey_num[4] = key_config_table[xcfg_cb.dblkey_num4];
    }
#endif

#if USER_ADKEY
    if (xcfg_cb.user_adkey_en) {
        adc_ch |= BIT(ADKEY_CH);
    #if ADKEY_PU10K_EN
        adcch_io_pu10k_enable(ADKEY_CH);        //开内部10K上拉
    #endif // ADKEY_PU10K_EN
        if (xcfg_cb.adkey_config_en) {
            adkey_num[0] = key_config_table[xcfg_cb.adkey1_num0];
            adkey_num[1] = key_config_table[xcfg_cb.adkey1_num1];
            adkey_num[2] = key_config_table[xcfg_cb.adkey1_num2];
            adkey_num[3] = key_config_table[xcfg_cb.adkey1_num3];
            adkey_num[4] = key_config_table[xcfg_cb.adkey1_num4];
            adkey_num[5] = key_config_table[xcfg_cb.adkey1_num5];
            adkey_num[6] = key_config_table[xcfg_cb.adkey1_num6];
            adkey_num[7] = key_config_table[xcfg_cb.adkey1_num7];
            adkey_num[8] = key_config_table[xcfg_cb.adkey1_num8];
            adkey_num[9] = key_config_table[xcfg_cb.adkey1_num9];
            adkey_num[10] = key_config_table[xcfg_cb.adkey1_num10];
            adkey_num[11] = key_config_table[xcfg_cb.adkey1_num11];
        }
    }
#endif // USER_ADKEY

#if USER_ADKEY2
    adc_ch |= BIT(ADKEY2_CH);
#endif // USER_ADKEY2

#if USER_ADKEY_MUX_SDCLK
    if (xcfg_cb.user_adkey_mux_sdclk_en) {
        adc_ch |= BIT(SDCLK_AD_CH);
    }
#endif // USER_ADKEY_MUX_SDCLK

	adc_ch |= BIT(ADCCH_ANA);
#if VBAT_DETECT_EN
    adc_ch |=  BIT(VBAT2_ADCCH);
#endif // VBAT_DETECT_EN

#if TSENSOR_TEST_EN
    adc_ch |=  BIT(ADCCH_TS);
#endif

    plugin_saradc_init(&adc_ch);   //其他的ADC通道
    bsp_saradc_init(adc_ch);

#if MAXTRIX_KEY_EN
    maxtrix_key_init();
#endif // MAXTRIX_KEY_EN

#if MUL_PWRON_IO_EN
    mulpwr_key_init();
#endif

    power_on_check();
    CRSTPND = 0x1ff0000;          //clear reset pending
}

#if USER_MULTI_PRESS_EN || TKEY_SLIDE_DET_EN
AT(.com_text.bsp.key)
bool check_key_return(u16 key_return)
{
    u8 i;
    if (key_return == NO_KEY) {
        return false;
    }

#if USER_MULTI_PRESS_EN
    //工具配置了哪些按键支持双击功能？
    i = check_key_double_configure(key_return);
    if (i > 0) {
        return (i - 1);
    }
#endif // USER_MULTI_PRESS_EN
    for (i=0; i<sizeof(tbl_double_key)/2; i++) {
        if (key_return == tbl_double_key[i]) {
            return true;
        }
    }
    return false;
}
#else
bool check_key_return(u16 key_return)
{
    return false;
}
#endif // USER_KEY_DOUBLE_EN

/*
AT(.com_text.bsp.key)
void pwrkey10s_counter_clr(void)
{
//    if (GPIO_10S_RESET) {
//        RTCCON10 = BIT(10);                 //clear pwrkey10s pending and counter
//    }
}
*/

AT(.text.bsp.key)
u8 get_pwroff_pressed_time(void)
{
    return PWROFF_PRESS_TIME;
}

AT(.text.bsp.key)
u8 get_double_key_time(void)
{
    if(DOUBLE_KEY_TIME > 7) {
        return 60;
    } else {
        return (u8)((u8)(DOUBLE_KEY_TIME + 2) * 20 + 1);
    }
}

AT(.com_text.bsp.key)
bool get_poweron_flag(void)
{
    return sys_cb.poweron_flag;
}

AT(.com_text.bsp.key)
void set_poweron_flag(bool flag)
{
    sys_cb.poweron_flag = flag;
}

#if USER_KEY_KNOB_EN
AT(.com_text.bsp.key)
void key_knob_process(u16 adc_val, const u8 *knob_level, u8 *key_val)
{
    u8 step = 0;
    if (s_abs((int)knob_level[*key_val] - (int)adc_val) > 4) {    //取绝对值消抖
        while (adc_val > knob_level[step]) {
            step++;
        }
        *key_val = step;
    }
}
#endif

#if USER_KEY_KNOB2_EN
AT(.com_text.bsp.key)
u16 key_knob_process2(u16 *key_val)
{
    static u8 key_flag = 0;
    static u8 key_cnt_0 = 0, key_cnt_1 = 0;
//    my_printf(test_printf,*key_val);
    if (*key_val != KEY_VOL_DOWN_PREV && *key_val != KEY_VOL_UP_NEXT) {     //过滤掉其他按键
        key_flag = 0;
        if (*key_val == NO_KEY) {
            return NO_KEY;
        }
        return 0xffff;
    }

    if (*key_val == KEY_VOL_DOWN_PREV) {
        if (key_cnt_0 > 1) {    //消抖
            key_cnt_0 = 0;
            if (key_flag & BIT(1)) {
                key_flag = 0;
                return KEY_VOL_UP_NEXT | KEY_SHORT_UP;   //顺时针
            } else {
                key_flag |= BIT(0);
            }
        } else {
            key_cnt_0++;
        }
    }
    if (*key_val == KEY_VOL_UP_NEXT) {
        if (key_cnt_1 > 1) {    //消抖
            key_cnt_1 = 0;
            if (key_flag & BIT(0)) {
                key_flag = 0;
                return KEY_VOL_DOWN_PREV | KEY_SHORT_UP; //逆时针
            } else {
                key_flag |= BIT(1);
            }
        } else {
            key_cnt_1++;
        }
    }
    *key_val = NO_KEY;
    return NO_KEY;
}
#endif

AT(.com_text.bsp.key)
u16 bsp_key_process(u16 key_val)
{
    u16 key_return = NO_KEY;
#if USER_KEY_KNOB2_EN
 	static u8 timer1ms_cnt=0;
    timer1ms_cnt++;
    u16 key_ret = key_knob_process2(&key_val);
    if (key_ret != 0xffff && key_ret != NO_KEY) {
        return key_ret;
    }
    if(timer1ms_cnt > 5) {
        timer1ms_cnt=0;

        key_return = key_process(key_val);

    //双击处理
#if USER_MULTI_PRESS_EN
        //配置工具是否使能了按键2/3/4/5击功能？
        if (xcfg_cb.user_key_multi_press_en) {
            key_return = key_multi_press_process(key_return);
        }
#endif
        return key_return;
    }
    return NO_KEY;
#else
	key_return = key_process(key_val);

    //双击处理
#if USER_MULTI_PRESS_EN || TKEY_SLIDE_DET_EN
    //配置工具是否使能了按键2/3/4/5击功能？
    if (xcfg_cb.user_key_multi_press_en) {
#if USER_MULTI_PRESS_EN
        key_return = key_multi_press_process(key_return);
#elif TKEY_SLIDE_DET_EN
        key_return = key_slide_press_process(key_return);
#endif //
    }
#endif
    return key_return;
#endif
}

// AT(.com_rodata.bsp.key)
// const char key_str[] = "enqueue: %04x\n";

// AT(.com_text.str)
// const char str_ii[] = "tkey:%d\n";

AT(.com_text.bsp.key)
u8 bsp_key_scan(void)
{
    u8 key_val = NO_KEY;
    u16 key = NO_KEY;

#if (VBAT_DETECT_EN || USER_ADKEY)
    if (!get_adc_val()) {
        return NO_KEY;
    }
#endif // (VBAT_DETECT_EN || USER_ADKEY)

#if TKEY_MUL_SCAN_EN && !TKEY_SCAN_SWITCH_EN
    if(tkey_finish_flag) {
        if (key_val == NO_KEY) {
            key_val = get_tkey();
        }
    }
#endif // TKEY_MUL_SCAN_EN

#if MAXTRIX_KEY_EN
    key_val = bsp_maxtrix_get();
#endif // MAXTRIX_KEY_EN

#if USER_ADKEY
    if (key_val == NO_KEY) {
        key_val = get_adkey(adc_cb.key_val, xcfg_cb.user_adkey_en);
    }
#endif // USER_ADKEY

#if USER_ADKEY2
    if (key_val == NO_KEY) {
        key_val = get_adkey2();
    }
#endif // USER_ADKEY2

#if USER_IOKEY
    if (key_val == NO_KEY) {
        key_val = get_iokey();
    }
#endif // USER_IOKEY

#if VBAT_DETECT_EN
    sys_cb.vbat = get_vbat_val();
#endif // VBAT_DETECT_EN

#if IRRX_SW_EN
    if (key_val == NO_KEY) {
        key_val = get_irkey();
    }
#endif // IRRX_SW_EN

#if MUL_PWRON_IO_EN
    if (key_val == NO_KEY) {
        key_val = get_mulpwr_key();
    }
#endif

#if USER_ADKEY_MUX_SDCLK
    //需要放到最后处理,当没进行adc convert需要返回
    if (key_val == NO_KEY) {
        if (!adc_cb.sdclk_valid) {
            return NO_KEY;
        }
        key_val = get_adkey(adc_cb.sdclk_val, xcfg_cb.user_adkey_mux_sdclk_en);
    }
#endif // USER_ADKEY_MUX_SDCLK

    key = bsp_key_process(key_val);
    if (key != NO_KEY) {
        //防止enqueue多次HOLD消息
        if ((key & KEY_TYPE_MASK) == KEY_LONG) {
            sys_cb.kh_vol_msg = (key & 0xff) | KEY_HOLD;
        } else if ((key & KEY_TYPE_MASK) == KEY_LONG_UP) {
            msg_queue_detach(sys_cb.kh_vol_msg, 0);
            sys_cb.kh_vol_msg = NO_KEY;
        } else if (sys_cb.kh_vol_msg == key) {
            msg_queue_detach(key, 0);
        }
        // printf(key_str, key);
        msg_enqueue(key);
    }
    return key_val;
}

