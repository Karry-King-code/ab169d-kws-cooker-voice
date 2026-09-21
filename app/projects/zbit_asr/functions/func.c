#include "include.h"

func_cb_t func_cb AT(.buf.func_cb);

void asr_kws_process(void);
void sdadc_isr_process(void);
void uart_deal_process(void);

#if VBAT_DETECT_EN
void lowpower_vbat_process(void)
{
    int lpwr_vbat_sta = is_lowpower_vbat_warning();
    //my_printf("sys_cb.vbat %d\n",sys_cb.vbat);   //打印电池电量

    if (lpwr_vbat_sta == 1) {
#if WARNING_LOW_BATTERY
        //func_mp3_res_play(RES_BUF_LOW_BATTERY_MP3, RES_LEN_LOW_BATTERY_MP3);  //没有低电量提示音，先注释
#endif
        sys_cb.pwrdwn_tone_en = 1;
        func_cb.sta = FUNC_PWROFF;     //低电，进入关机或省电模式
        return;
    }
    if ((func_cb.mp3_res_play == NULL) || (lpwr_vbat_sta != 2)) {
        if ((sys_cb.lowbat_flag) && (sys_cb.vbat > 3800)) {
            sys_cb.vbat_nor_cnt++;
            if (sys_cb.vbat_nor_cnt > 40) {
                sys_cb.lowbat_flag = 0;
                sys_cb.lpwr_warning_times = LPWR_WARING_TIMES;
                plugin_lowbat_vol_recover();    //离开低电, 恢复音乐音量
            }
        }
        return;
    }

    //低电提示音播放
    sys_cb.vbat_nor_cnt = 0;
    xcfg_cb.lpwr_warning_period = 30;
    if (sys_cb.lpwr_warning_cnt > xcfg_cb.lpwr_warning_period) {
        sys_cb.lpwr_warning_cnt = 0;
        if (sys_cb.lpwr_warning_times) {        //低电语音提示次数
            if (RLED_LOWBAT_FOLLOW_EN) {
                //led_lowbat_follow_warning();
            }

            sys_cb.lowbat_flag = 1;
#if WARNING_LOW_BATTERY
            //func_mp3_res_play(RES_BUF_LOW_BATTERY_MP3, RES_LEN_LOW_BATTERY_MP3);
#endif
            plugin_lowbat_vol_reduce();         //低电降低音乐音量

            if (RLED_LOWBAT_FOLLOW_EN) {
//                while (get_led_sta(1)) {        //等待红灯闪完
//                    delay_5ms(2);
//                }
//                led_lowbat_recover();
            }

            if (sys_cb.lpwr_warning_times != 0xff) {
                sys_cb.lpwr_warning_times--;
            }
        }
    }
}

#if VDDIO_FOLLOW_VBAT_EN
u32 vio_fkick = 0;

void vddio_follow_vbat_process(void)
{
    vddio_voltage_update(adc_cb.vbat_val);
    if ((adc_cb.channel & BIT(ADCCH_ANA)) && (get_cur_vddio_vol() < adc_cb.vbat_val)) {
        if (tick_check_expire(vio_fkick,1000)) {
            vio_fkick = tick_get();
            adc_cb.channel &= ~BIT(ADCCH_ANA);
            PWRCON0 &= ~BIT(12);                   //BG output disable
        }
    }
}
#endif
#endif // VBAT_DETECT_EN

AT(.text.func.process)
void print_info(void)
{
    static u32 ticks = 0;
    if (tick_check_expire(ticks,1000)) {
        ticks = tick_get();
        my_printf(".");
    }
}

AT(.text.func.process)
void func_process(void)
{
    WDT_CLR();
    //print_info();
#if VBAT_DETECT_EN
    lowpower_vbat_process();
#if VDDIO_FOLLOW_VBAT_EN
    vddio_follow_vbat_process();
#endif
#endif // VBAT_DETECT_EN

    sdadc_isr_process();

// #if ASR_RECOG_EN
//     asr_kws_process();
// #endif

#if HUART_EN
//    huart_deal_process();
#endif

#if USER_UART0_EN
    #if UART_AUDIO_EN
    uart_decode_process();
    uart_main_process();
    #else
    uart_deal_process();
    #endif
#endif

#if TKEY_PRESS_UPDATE
    tkey_press_update();
#endif
#if TKEY_BUF_DOWN_EN
    tkey_tkbuf_down();
#endif
}

//func common message process
AT(.text.func.msg)
void func_message(u16 msg)
{
    switch (msg) {
        case KU_VOL_UP:
        case KL_VOL_UP:
        case KH_VOL_UP:
            bsp_set_volume(bsp_volume_inc(sys_cb.vol, 1));
            break;

        case KU_VOL_DOWN:
        case KL_VOL_DOWN:
        case KH_VOL_DOWN:
            bsp_set_volume(bsp_volume_dec(sys_cb.vol, 1));
            break;

        case KU_MODE:
        case KU_MODE_POWER:
        case KL_PLAY_MODE:
            func_cb.sta = FUNC_NULL;
            break;

        case KU_PLAY:
            my_printf("KU_PLAY\n");
            break;

        case KD_PLAY:
            my_printf("KD_PLAY\n");
            break;

        case KD_VOL_UP:
            my_printf("KD_VOL_UP\n");
            break;

        case KD_VOL_DOWN:
            my_printf("KD_VOL_DOWN\n");
            break;

        case KTH_VOL_UP:
            my_printf("slide! right to lift\n");
            break;

        case KTH_VOL_DOWN:
            my_printf("slide! lift to right\n");
            break;

        case KFI_PREV:
            my_printf("slide! 5key slide lift_to_right\n");
            break;


        case KFI_VOL_UP:
            my_printf("slide! 5key slide right_to_lift\n");
            break;

#if MIX_PWR_DOWN_EN
        case KLH_PLAY_POWER:
            mix_pwr_down(1);
            break;
        case KLH_MODE_POWER:
            mix_pwr_down(0);
            break;
#else
        //长按PP/POWER软关机(通过PWROFF_PRESS_TIME控制长按时间)
        case KLH_PLAY_POWER:
        case KLH_MODE_POWER:
        case KLH_HSF_POWER:
        case KLH_POWER:
            sys_cb.pwrdwn_tone_en = 1;
            func_cb.sta = FUNC_PWROFF;
            break;
#endif

#if MUL_PWRON_IO_EN
        case KEY_NUM_0:
            break;
        case KEY_NUM_1:
            break;
        case KEY_NUM_2:
        case KEY_NUM_3:
        case KEY_NUM_4:
        case KEY_NUM_5:
        case KEY_NUM_6:
        case KEY_NUM_7:
        case KEY_NUM_8:
        case KEY_NUM_9:
        case KEY_NUM_P100:
            break;
#endif

        case MSG_SYS_1S:

            break;
        default:
            break;

    }

    //调节音量，3秒后写入flash
    if ((sys_cb.cm_vol_change) && (sys_cb.cm_times >= 6)) {
        sys_cb.cm_vol_change = 0;
        cm_sync();
    }
}

///进入一个功能的总入口
AT(.text.func)
void func_enter(void)
{
#if (GUI_SELECT != GUI_NO)
    gui_box_clear();
#endif
    param_sync();
    func_cb.mp3_res_play = NULL;
    func_cb.set_vol_callback = NULL;
//    bsp_clr_mute_sta();
}

AT(.text.func)
void func_exit(void)
{
    u8 func_num;
    u8 funcs_total = get_funcs_total();

    for (func_num = 0; func_num != funcs_total; func_num++) {
        if (func_cb.last == func_sort_table[func_num]) {
            break;
        }
    }
    func_num++;                                     //切换到下一个任务
    if (func_num >= funcs_total) {
        func_num = 0;
    }
    func_cb.sta = func_sort_table[func_num];        //新的任务
}

AT(.text.func)
void func_run(void)
{
    printf("%s\n", __func__);

    while (1) {
        func_enter();
        switch (func_cb.sta) {
#if FUNC_MUSIC_EN
        case FUNC_MUSIC:
            func_music();
            break;
#endif // FUNC_MUSIC_EN

#if EX_SPIFLASH_SUPPORT
        case FUNC_EXSPIFLASH_MUSIC:
            func_exspiflash_music();
            break;
#endif // EX_SPIFLASH_SUPPORT

#if FUNC_SPEAKER_EN
        case FUNC_SPEAKER:
            func_speaker();
            break;
#endif // FUNC_SPEAKER_EN

#if FUNC_IDLE_EN
        case FUNC_IDLE:
            func_idle();
            break;
#endif // FUNC_IDLE_EN

        case FUNC_PWROFF:
            func_pwroff(sys_cb.pwrdwn_tone_en);
            break;

        case FUNC_SLEEPMODE:
            func_sleepmode();
            break;

        default:
            func_exit();
            break;
        }
    }
}
