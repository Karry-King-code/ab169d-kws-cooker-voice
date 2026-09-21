#ifndef __FUNC_LOWPWR_H
#define __FUNC_LOWPWR_H

// typedef bool (*is_sleep_func)(void);

#define PWR_L1M         0   //0-IO不能唤醒,RAM不保存(不使用)
#define PWR_L2M         1   //1-IO能唤醒,RAM不保存(不使用)
#define PWR_L3M         2   //2-IO能唤醒,RAM不保存(deep sleep, slow start),功耗:4uA~5uA,唤醒时间:约40ms
#define PWR_L4M         3   //3-IO能唤醒,RAM保存(fast sleep, fast start),功耗:7uA~10uA,唤醒时间:约803us
#define PWR_RTC1S       5   //5-RTC1秒唤醒功能
#define PWR_RUNRC       6   //6-RC低功耗模式
#define PWR_ALARM       7   //7-闹钟定时唤醒模式
#define PWR_L3M_ALM     8   //8-pdn3唤醒+闹钟唤醒模式
#define PWR_L4M_ALM     9   //9-pdn4唤醒+闹钟唤醒模式

typedef enum{
    LP_DEEP_SLEEP,
    LP_FAST_SLEEP,
}LP_TYPE_SEL;

typedef enum{
    WK_EDGE_RISE,
    WK_EDGE_FALL,
}WK_EDGE_SEL;

//对应RTCCON9
typedef enum{
    WK_PWROFF_BGUV    = BIT(11),
    WK_PWROFF_GPIO    = BIT(7),
    WK_PWROFF_RTC1S   = BIT(1),
    WK_PWROFF_ALARM   = BIT(0),
}WK_SOURCE_SEL;

typedef struct{
    LP_TYPE_SEL lp_type;
    WK_SOURCE_SEL source;
    union{
        struct{
            u8  wk_io_fall_en   :1,     //固定IO的下降沿唤醒
                wk_io_rise_en   :1,     //固定IO的上升沿唤醒
                wk_fall_en      :1,     //任意IO的下降沿唤醒
                wk_rise_en      :1;     //任意IO的上升沿唤醒
        };
        u8 wk_sel;
    };
    u8 wk_io_fall_num;
    u8 wk_io_fall_group[WKU_IO_FALL_NUM];
    u8 wk_io_rise_num;
    u8 wk_io_rise_group[WKU_IO_RISE_NUM];

    u8 fall_io_num;
    u8 fall_io_group[POWER_ON_FALL_NUM];
    u8 rise_io_num;
    u8 rise_io_group[POWER_ON_RISE_NUM];
    u8 pdn_io_interface_flag;
}pwroff_wk_t;

typedef struct{
    u32 piccon;
    u32 picen;
    u32 picpr;
    u32 nmicon;
    u32 clkgat0;
    u32 clkgat1;

    u32 rtccon;

    u32 pwrcon0;
    u32 pwrcon1;

    u32 pll0con;
    u32 pll0div;

    u32 clkcon0;
    u32 clkcon1;
    u32 clkdivcon0;

    u16 gpioa_fen;
    u16 gpiog_fen;
    u16 gpioa_dir;
    u16 gpiog_dir;
    u16 gpioa_de;
    u16 gpiog_de;
    u16 gpioa_drv;
    u16 gpiog_drv;

    u16 gpioa_pu;
    u16 gpiog_pu;
    u16 gpioa_pd;
    u16 gpiog_pd;

    u32 funcoutmcon;
    u32 funcoutcon;
    u32 funcincon;
    u32 funcomcon[2];

    u32 udetbaud;
    u16 udetcon;
    u16 adc_ch;
}sleep_sfr_t;

#define GLOBAL_INT_DISABLE()    uint32_t cpu_ie = PICCON&BIT(0); PICCONCLR = BIT(0); asm volatile("":::"memory")
#define GLOBAL_INT_RESTORE()    asm volatile("":::"memory"); PICCON |= cpu_ie

void pwron_fall_set_do(u8 io_num);
void pwron_rise_set_do(u8 io_num);

u32 lowpwr_get_wakeup_source(void);
void pwroff_pmu_voltage_config(void);
void pwroff_rtc_core_config(pwroff_wk_t *wkcfg);

void func_pwroff(int pwroff_tone_en);                   //进入低电或者关机状态
void func_sleepmode(void);                              //sleepmode假关机，带红外唤醒

void pdn4_shutdown(void);
void set_all_pin_off(void);
void pwroff_wk_source_config(pwroff_wk_t *wkcfg);
void mix_pwr_down(u8 pwr_mode);

void pwroff_do(void);
void sfunc_lowbat(void);
void sfunc_pwroff(void);

#endif // __FUNC_LOWPWR_H
