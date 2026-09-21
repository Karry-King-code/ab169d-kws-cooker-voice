#ifndef _PORT_INIT_H
#define _PORT_INIT_H

#define VOL_MAX                         sys_cb.vol_max   //最大音量级数

typedef struct {
    u8  play_mode;
    u8  vol;
    u8  vol_max;
    u8  eq_mode;
    u8  cur_dev;
    u8  lang_id;
    u8  lpwr_warning_cnt;
    u8  lpwr_warning_times;     //播报低电次数，0xff表示一直播
    u8  vbat_nor_cnt;
    u8  lpwr_cnt;               //低电计数

    u16 vbat;
    u16 kh_vol_msg;
    u32 wakeup_reason;          //开机唤醒源
    u32 tmr1ms_cnt;
    u32 tmr5ms_cnt;
	u32 ms_ticks;               //ms为单位
	u32 ir_rx_cnt;
#if ASR_RECOG_EN
    u8 asr_en;
#endif

volatile u8  cm_times;
// volatile u8  loudspeaker_mute;//功放MUTE标志
// volatile u8  charge_sta;     //0:充电关闭， 1：充电开启， 2：充满
volatile u8  poweron_flag;      //pwrkey开机标志
volatile u8  pwrdwn_tone_en;    //POWER DOWN是否播放提示音
// volatile u8  pwrdwn_hw_flag; //模拟硬开关，关机标志

    ///位变量不要用于需要在中断改值的变量。 请谨慎使用位变量，尽量少定义位变量。
    u8  rtc_first_pwron  : 1,   //RTC是否第一次上电
        mute             : 1,   //系统MUTE控制标志
        cm_factory       : 1,   //是否第一次FLASH上电
        cm_vol_change    : 1,   //音量级数是否需要更新到FLASH
        port2led_en      : 1,   //1个IO推两个灯
        voice_evt_brk_en : 1;   //播放提示音时，U盘、SD卡、LINEIN等插入事件是否立即响应.

    u8  maxvol_fade      : 2,
		lowbat_flag      : 1;
} sys_cb_t;
extern sys_cb_t sys_cb;

void bsp_sys_init(void);
void bsp_update_init(void);
#endif
