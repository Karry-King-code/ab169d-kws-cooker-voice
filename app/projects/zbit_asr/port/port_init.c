#include "include.h"

sys_cb_t sys_cb AT(.buf.bsp.sys_cb);

//timer tick interrupt(5ms)
AT(.com_text.timer)
void usr_tmr5ms_isr(void)
{
    sys_cb.tmr5ms_cnt++;
#if !USER_KEY_KNOB2_EN
    bsp_key_scan();
#endif

    //500ms timer process
    if ((sys_cb.tmr5ms_cnt % 100) == 0) {
        sys_cb.cm_times++;
    }

    //1s timer process
    if ((sys_cb.tmr5ms_cnt % 200) == 0) {
        msg_enqueue(MSG_SYS_1S);
        sys_cb.tmr5ms_cnt = 0;
        sys_cb.lpwr_warning_cnt++;
    }
}

extern int led_tick;
//extern int led_1s_tick;
//extern int led_15min_tick;
extern volatile bool time_set_flag;
//timer tick interrupt(1ms)
AT(.com_text.timer)
void usr_tmr1ms_isr(void)
{
    sys_cb.tmr1ms_cnt++;

    if ((sys_cb.tmr1ms_cnt % 5) == 0) {
        usr_tmr5ms_isr();
    }

    if(time_set_flag)
    {
        led_tick++;
    }

#if LED_DISP_EN
    if ((sys_cb.tmr1ms_cnt % 50) == 0) {
        led_scan();
    }
#endif // LED_DISP_EN

    //100ms timer process
    if ((sys_cb.tmr1ms_cnt % 100) == 0) {
        sys_cb.tmr1ms_cnt = 0;
        if (sys_cb.lpwr_cnt > 0) {
            sys_cb.lpwr_cnt++;
        }
    }

#if TKEY_SCAN_SWITCH_EN
    if ((sys_cb.tmr1ms_cnt % TKEY_SCAN_TIME) == 0) {
    	tkey_cir_scan_en();
    }
#endif
}

AT(.text.bsp.sys.init)
void power_on_off_init(void)
{
#if SOFT_POWER_ON_OFF
    // //避免ADkey睡眠唤醒后状态异常
    // GPIOAFEN &= ~BIT(POWER_ON_FALL_IO-1);
    // GPIOADE  |=  BIT(POWER_ON_FALL_IO-1);
    // GPIOADIR |=  BIT(POWER_ON_FALL_IO-1);
#endif
}

AT(.rodata.vol)
const u8 maxvol_tbl[4] = {16, 32, 50};

//开user timer前初始化的内容
AT(.text.bsp.sys.init)
static void bsp_var_init(void)
{
    memset(&sys_cb, 0, sizeof(sys_cb));
    sys_cb.ms_ticks = tick_get();
    sys_cb.vol_max = maxvol_tbl[xcfg_cb.vol_max];
    if (SYS_INIT_VOLUME > sys_cb.vol_max) {
        SYS_INIT_VOLUME = sys_cb.vol_max;
    }
    if (WARNING_VOLUME > sys_cb.vol_max) {
        WARNING_VOLUME = sys_cb.vol_max;
    }

#if (PIN_SOP_SEL != 2)
    pin_sop_sel = PIN_SOP_SEL;
#endif
    // key_var_init();
    plugin_var_init();

    msg_queue_init();

    sdadc_var_init();

    dac_cb_init();
//    dev_init(is_sd_support());
}

AT(.text.bsp.sys.init)
static void bsp_io_init(void)
{
    if(!GPIO_10S_RESET){
        GPIOAPU &= ~BIT(GPIO_10S_SEL-1);
    }
    if(!MCLR_S_RESET){
        GPIOAPU &= ~BIT(11);
    }
    GPIOADE = 0;
    if(MCLR_S_RESET){
        GPIOADE |= BIT(11);
    }
    if(GPIO_10S_RESET){
        GPIOADE |= BIT(GPIO_10S_SEL-1);
    }
    GPIOBDE = 0;
    GPIOGDE = 0x3F;             //MCP FLASH
    uart0_mapping_sel();        //调试UART IO选择或关闭
}

AT(.text.bsp.sys.init)
void bsp_update_init(void)
{
    /// config
    if (!xcfg_init(&xcfg_cb, sizeof(xcfg_cb))) {           //获取配置参数
        printf("xcfg init error\n");
    }

    // io init
    bsp_io_init();

    // var init
    bsp_var_init();
    sys_cb.lang_id = 0;

    // peripheral init
    rtc_init();
//    param_init(sys_cb.rtc_first_pwron);

    plugin_init();
    sys_set_tmr_enable(1);

    adpll_init(SYS_CLK_SEL);
#if DAC_EN
    dac_init();
#endif
#if WARNING_UPDATE_DONE
    mp3_res_play(RES_BUF_UPDATE_DONE_MP3, RES_LEN_UPDATE_DONE_MP3);
#endif
}

AT(.text.bsp.sys.init)
void bsp_sys_init(void)
{
    /// config
    if (!xcfg_init(&xcfg_cb, sizeof(xcfg_cb))) {           //获取配置参数
        printf("xcfg init error\n");
    }

    // io init
    bsp_io_init();

    // var init
    bsp_var_init();

    // power init
    pmu_init(0);

    // clock init
    adpll_init(SYS_CLK_SEL);
    set_sys_clk(SYS_CLK_SEL);
    // dbg_clk_out(9, 10);

    // peripheral init
    rtc_init();
//    param_init(sys_cb.rtc_first_pwron);
    plugin_init();
    power_on_off_init();        //开关机io初始化

#if IRRX_SW_EN
    irrx_sw_init();
#endif // IRRX_SW_EN

    led_init();
    key_init();

    /// enable user timer for display & dac
    sys_set_tmr_enable(1);

#if DAC_EN
    dac_init();
#endif

    bsp_change_volume(sys_cb.vol);

#if DAC_DRC_EN
    drc_v3_init((u8 *)RES_BUF_DRC_DAC_MUSIC_DRC, RES_LEN_DRC_DAC_MUSIC_DRC);
#endif

#if EX_SPIFLASH_SUPPORT
    exspiflash_init();
#endif

#if SPK_SPI_DUMP
    my_spi_init();
#endif
#if HUART_DEUMP_EN
    huart_dump_init();
#endif

#if HUART_EN
    bsp_huart_init();
#endif

#if USER_UART0_EN
    bsp_uart_init();
#endif

#if TKEY_MUL_SCAN_EN
    tkey_init();
#endif // TKEY_MUL_SCAN_EN

#if (WARNING_POWER_ON && WARNING_MP3_PLAY)
    mp3_res_play(RES_BUF_EN_POWERON_MP3, RES_LEN_EN_POWERON_MP3);
#endif

#if PWM_TMR2_EN
    tmr2pwm_cfg_init();
#endif

#if IRTX_HW_EN
    bsp_irtx_init(method_level);
#endif

#if I2C_SW_EN
    bsp_i2c_init();
#endif

#if SPI_AUDIO_TEST_EN
    spi_test_init();
#endif
}
