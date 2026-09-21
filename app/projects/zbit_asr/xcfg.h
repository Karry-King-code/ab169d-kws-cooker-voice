//该头文件由软件自动生成，请勿随意修改！
#ifndef _XCFG_H
#define _XCFG_H

#define XCFG_EN             1

typedef struct __attribute__((packed)) _xcfg_cb_t {
    u32 lang_id                          : 2;   //提示音语言选择: 英文: 0, 中文: 1, 中英文(出厂默认英文): 2, 中英文(出厂默认中文): 3
    u32 lowpower_warning_en              : 1;   //低电提示
    u8 lpwr_warning_vbat;                       //低电语音提示电压: 0:1.9V, 1:2.0V, 2:2.1V, 3:2.2V, 4:2.3V, 5:2.4V, 6:2.5V, 7:2.6V, 8:2.7V, 9:2.8V, 10:2.9V, 11:3.0V, 12:3.1V, 13:3.2V, 14:3.3V, 15:3.4V, 16:3.5V, 17:3.6V, 18:3.7V
    u8 lpwr_off_vbat;                           //低电关机电压: 0:不关机, 1:1.9V, 2:2.0V, 3:2.1V, 4:2.2V, 5:2.3V, 6:2.4V, 7:2.5V, 8:2.6V, 9:2.7V, 10:2.8V, 11:2.9V, 12:3.0V, 13:3.1V, 14:3.2V, 15:3.3V, 16:3.4V, 17:3.5V, 18:3.6V, 19:3.7V
    u8 lpwr_warning_period;                     //低电语音播报周期(秒)
    u32 vol_max                          : 2;   //音量级数: 16级音量: 0, 32级音量: 1, 50级音量: 2
    u8 sys_init_vol;                            //开机默认音量
    u8 warning_volume;                          //提示音播放音量
    u32 vddio_sel                        : 5;   //VDDIO电压: 1.8V: 4, 1.9V: 6, 2.0V: 7, 2.1V: 8, 2.2V: 10, 2.3V: 11, 2.4V: 12, 2.5V: 14, 2.6V: 15, 2.7V: 16, 2.8V: 18, 2.9V: 19, 3.0V: 20, 3.1V: 22, 3.2V: 23, 3.3V: 24, 3.4V: 26, 3.5V: 27, 3.6V: 28, 3.7V: 30, 3.8V: 31
    u32 reset_sel                        : 3;   //复位电压: 1.8V: 0, 2.0V: 1, 2.2V: 2, 2.4V: 3, 2.6V: 4, 2.8V: 5, 3.0V: 6, 1.7V: 7
    u32 mic_bias_en                      : 1;   //MIC_BIAS使能
    u32 mic_bias_sel                     : 4;   //MIC_BIAS电压: 1.8V: 0, 2.0V: 1, 2.2V: 2, 2.4V: 3, 2.5V: 4, 2.6V: 5, 2.7V: 6, 2.8V: 7
    u32 mic_bias_method                  : 2;   //MIC偏置电路配置: MIC外部电阻电容: 0, MIC省电阻电容: 1, MIC只省电阻: 2
    u32 mic_anl_gain                     : 4;   //MIC模拟增益(9~21DB)
    u32 mic_dig_gain                     : 6;   //MIC数字增益(0~36DB)
    u32 user_adkey_en                    : 1;   //ADKEY1功能
    u32 user_adkey2_en                   : 1;   //ADKEY2功能
    u32 user_iokey_en                    : 1;   //IOKEY功能
    u32 user_adkey_mux_sdclk_en          : 1;   //复用SDCLK的ADKEY
    u32 adkey_ch                         : 4;   //ADKEY1通路选择: ADCCH_PA0: 0, ADCCH_PA1: 1, ADCCH_PA2: 2, ADCCH_PA3: 3, ADCCH_PA4: 4, ADCCH_PA5: 5, ADCCH_PA6: 6, ADCCH_PA7: 7, ADCCH_PA8: 8, ADCCH_PA9: 9, ADCCH_PA10: 10, ADCCH_PA11: 11
    u32 adkey2_ch                        : 4;   //ADKEY2通路选择: ADCCH_PA0: 0, ADCCH_PA1: 1, ADCCH_PA2: 2, ADCCH_PA3: 3, ADCCH_PA4: 4, ADCCH_PA5: 5, ADCCH_PA6: 6, ADCCH_PA7: 7, ADCCH_PA8: 8, ADCCH_PA9: 9, ADCCH_PA10: 10, ADCCH_PA11: 11
    u32 user_key_multi_press_en          : 1;   //按键多击功能
    u32 double_key_time                  : 3;   //双击响应时间选择: 200ms: 0, 300ms: 1, 400ms: 2, 500ms: 3, 600ms: 4, 700ms: 5, 800ms: 6, 900ms: 7
    u32 pwron_press_time                 : 3;   //软开机长按时间选择: 0.1秒: 0, 0.5秒: 1, 1秒: 2, 1.5秒: 3, 2秒: 4, 2.5秒: 5, 3秒: 6, 3.5秒: 7
    u32 pwroff_press_time                : 3;   //软关机长按时间选择: 1.5秒: 0, 2秒: 1, 2.5秒: 2, 3秒: 3, 3.5秒: 4, 4秒: 5, 4.5秒: 6, 5秒: 7
    u32 key_multi_config_en              : 1;   //支持多击的按键定制
    u32 dblkey_num0                      : 5;   //多击按键1选择: None: 0, PP/POWER: 1, P/P: 2, POWER: 3, NEXT/VOL+: 4, VOL+/NEXT: 5, PREV/VOL-: 6, VOL-/PREV: 7, VOL+: 8, VOL-: 9, NEXT: 10, PREV: 11, MODE: 12, PP/MODE: 13, HSF: 14, EQ: 15, REPEAT: 16, MUTE: 17, MODE/POWER: 18, REC: 19, PLAY/HSF: 20, A-B: 21
    u32 dblkey_num1                      : 5;   //多击按键2选择: None: 0, PP/POWER: 1, P/P: 2, POWER: 3, NEXT/VOL+: 4, VOL+/NEXT: 5, PREV/VOL-: 6, VOL-/PREV: 7, VOL+: 8, VOL-: 9, NEXT: 10, PREV: 11, MODE: 12, PP/MODE: 13, HSF: 14, EQ: 15, REPEAT: 16, MUTE: 17, MODE/POWER: 18, REC: 19, PLAY/HSF: 20, A-B: 21
    u32 dblkey_num2                      : 5;   //多击按键3选择: None: 0, PP/POWER: 1, P/P: 2, POWER: 3, NEXT/VOL+: 4, VOL+/NEXT: 5, PREV/VOL-: 6, VOL-/PREV: 7, VOL+: 8, VOL-: 9, NEXT: 10, PREV: 11, MODE: 12, PP/MODE: 13, HSF: 14, EQ: 15, REPEAT: 16, MUTE: 17, MODE/POWER: 18, REC: 19, PLAY/HSF: 20, A-B: 21
    u32 dblkey_num3                      : 5;   //多击按键4选择: None: 0, PP/POWER: 1, P/P: 2, POWER: 3, NEXT/VOL+: 4, VOL+/NEXT: 5, PREV/VOL-: 6, VOL-/PREV: 7, VOL+: 8, VOL-: 9, NEXT: 10, PREV: 11, MODE: 12, PP/MODE: 13, HSF: 14, EQ: 15, REPEAT: 16, MUTE: 17, MODE/POWER: 18, REC: 19, PLAY/HSF: 20, A-B: 21
    u32 dblkey_num4                      : 5;   //多击按键5选择: None: 0, PP/POWER: 1, P/P: 2, POWER: 3, NEXT/VOL+: 4, VOL+/NEXT: 5, PREV/VOL-: 6, VOL-/PREV: 7, VOL+: 8, VOL-: 9, NEXT: 10, PREV: 11, MODE: 12, PP/MODE: 13, HSF: 14, EQ: 15, REPEAT: 16, MUTE: 17, MODE/POWER: 18, REC: 19, PLAY/HSF: 20, A-B: 21
    u32 adkey_config_en                  : 1;   //ADKEY1按键定制
    u32 adkey1_num0                      : 5;   //ADKEY1按键1功能: None: 0, PP/POWER: 1, P/P: 2, POWER: 3, NEXT/VOL+: 4, VOL+/NEXT: 5, PREV/VOL-: 6, VOL-/PREV: 7, VOL+: 8, VOL-: 9, NEXT: 10, PREV: 11, MODE: 12, PP/MODE: 13, HSF: 14, EQ: 15, REPEAT: 16, MUTE: 17, MODE/POWER: 18, REC: 19, PLAY/HSF: 20, A-B: 21
    u32 adkey1_num1                      : 5;   //ADKEY1按键2功能: None: 0, PP/POWER: 1, P/P: 2, POWER: 3, NEXT/VOL+: 4, VOL+/NEXT: 5, PREV/VOL-: 6, VOL-/PREV: 7, VOL+: 8, VOL-: 9, NEXT: 10, PREV: 11, MODE: 12, PP/MODE: 13, HSF: 14, EQ: 15, REPEAT: 16, MUTE: 17, MODE/POWER: 18, REC: 19, PLAY/HSF: 20, A-B: 21
    u32 adkey1_num2                      : 5;   //ADKEY1按键3功能: None: 0, PP/POWER: 1, P/P: 2, POWER: 3, NEXT/VOL+: 4, VOL+/NEXT: 5, PREV/VOL-: 6, VOL-/PREV: 7, VOL+: 8, VOL-: 9, NEXT: 10, PREV: 11, MODE: 12, PP/MODE: 13, HSF: 14, EQ: 15, REPEAT: 16, MUTE: 17, MODE/POWER: 18, REC: 19, PLAY/HSF: 20, A-B: 21
    u32 adkey1_num3                      : 5;   //ADKEY1按键4功能: None: 0, PP/POWER: 1, P/P: 2, POWER: 3, NEXT/VOL+: 4, VOL+/NEXT: 5, PREV/VOL-: 6, VOL-/PREV: 7, VOL+: 8, VOL-: 9, NEXT: 10, PREV: 11, MODE: 12, PP/MODE: 13, HSF: 14, EQ: 15, REPEAT: 16, MUTE: 17, MODE/POWER: 18, REC: 19, PLAY/HSF: 20, A-B: 21
    u32 adkey1_num4                      : 5;   //ADKEY1按键5功能: None: 0, PP/POWER: 1, P/P: 2, POWER: 3, NEXT/VOL+: 4, VOL+/NEXT: 5, PREV/VOL-: 6, VOL-/PREV: 7, VOL+: 8, VOL-: 9, NEXT: 10, PREV: 11, MODE: 12, PP/MODE: 13, HSF: 14, EQ: 15, REPEAT: 16, MUTE: 17, MODE/POWER: 18, REC: 19, PLAY/HSF: 20, A-B: 21
    u32 adkey1_num5                      : 5;   //ADKEY1按键6功能: None: 0, PP/POWER: 1, P/P: 2, POWER: 3, NEXT/VOL+: 4, VOL+/NEXT: 5, PREV/VOL-: 6, VOL-/PREV: 7, VOL+: 8, VOL-: 9, NEXT: 10, PREV: 11, MODE: 12, PP/MODE: 13, HSF: 14, EQ: 15, REPEAT: 16, MUTE: 17, MODE/POWER: 18, REC: 19, PLAY/HSF: 20, A-B: 21
    u32 adkey1_num6                      : 5;   //ADKEY1按键7功能: None: 0, PP/POWER: 1, P/P: 2, POWER: 3, NEXT/VOL+: 4, VOL+/NEXT: 5, PREV/VOL-: 6, VOL-/PREV: 7, VOL+: 8, VOL-: 9, NEXT: 10, PREV: 11, MODE: 12, PP/MODE: 13, HSF: 14, EQ: 15, REPEAT: 16, MUTE: 17, MODE/POWER: 18, REC: 19, PLAY/HSF: 20, A-B: 21
    u32 adkey1_num7                      : 5;   //ADKEY1按键8功能: None: 0, PP/POWER: 1, P/P: 2, POWER: 3, NEXT/VOL+: 4, VOL+/NEXT: 5, PREV/VOL-: 6, VOL-/PREV: 7, VOL+: 8, VOL-: 9, NEXT: 10, PREV: 11, MODE: 12, PP/MODE: 13, HSF: 14, EQ: 15, REPEAT: 16, MUTE: 17, MODE/POWER: 18, REC: 19, PLAY/HSF: 20, A-B: 21
    u32 adkey1_num8                      : 5;   //ADKEY1按键9功能: None: 0, PP/POWER: 1, P/P: 2, POWER: 3, NEXT/VOL+: 4, VOL+/NEXT: 5, PREV/VOL-: 6, VOL-/PREV: 7, VOL+: 8, VOL-: 9, NEXT: 10, PREV: 11, MODE: 12, PP/MODE: 13, HSF: 14, EQ: 15, REPEAT: 16, MUTE: 17, MODE/POWER: 18, REC: 19, PLAY/HSF: 20, A-B: 21
    u32 adkey1_num9                      : 5;   //ADKEY1按键10功能: None: 0, PP/POWER: 1, P/P: 2, POWER: 3, NEXT/VOL+: 4, VOL+/NEXT: 5, PREV/VOL-: 6, VOL-/PREV: 7, VOL+: 8, VOL-: 9, NEXT: 10, PREV: 11, MODE: 12, PP/MODE: 13, HSF: 14, EQ: 15, REPEAT: 16, MUTE: 17, MODE/POWER: 18, REC: 19, PLAY/HSF: 20, A-B: 21
    u32 adkey1_num10                     : 5;   //ADKEY1按键11功能: None: 0, PP/POWER: 1, P/P: 2, POWER: 3, NEXT/VOL+: 4, VOL+/NEXT: 5, PREV/VOL-: 6, VOL-/PREV: 7, VOL+: 8, VOL-: 9, NEXT: 10, PREV: 11, MODE: 12, PP/MODE: 13, HSF: 14, EQ: 15, REPEAT: 16, MUTE: 17, MODE/POWER: 18, REC: 19, PLAY/HSF: 20, A-B: 21
    u32 adkey1_num11                     : 5;   //ADKEY1按键12功能: None: 0, PP/POWER: 1, P/P: 2, POWER: 3, NEXT/VOL+: 4, VOL+/NEXT: 5, PREV/VOL-: 6, VOL-/PREV: 7, VOL+: 8, VOL-: 9, NEXT: 10, PREV: 11, MODE: 12, PP/MODE: 13, HSF: 14, EQ: 15, REPEAT: 16, MUTE: 17, MODE/POWER: 18, REC: 19, PLAY/HSF: 20, A-B: 21
    u32 iokey_io0                        : 5;   //IOKEY按键1的IO: None: 0, PA0: 1, PA1: 2, PA2: 3, PA3: 4, PA4: 5, PA5: 6, PA6: 7, PA7: 8, PA8: 9, PA9: 10, PA10: 11, PA11: 12, PA12: 13, PA13: 14, PA14: 15, PA15: 16
    u32 iokey_io1                        : 5;   //IOKEY按键2的IO: None: 0, PA0: 1, PA1: 2, PA2: 3, PA3: 4, PA4: 5, PA5: 6, PA6: 7, PA7: 8, PA8: 9, PA9: 10, PA10: 11, PA11: 12, PA12: 13, PA13: 14, PA14: 15, PA15: 16
    u32 iokey_io2                        : 5;   //IOKEY按键3的IO: None: 0, PA0: 1, PA1: 2, PA2: 3, PA3: 4, PA4: 5, PA5: 6, PA6: 7, PA7: 8, PA8: 9, PA9: 10, PA10: 11, PA11: 12, PA12: 13, PA13: 14, PA14: 15, PA15: 16
    u32 iokey_io3                        : 5;   //IOKEY按键4的IO: None: 0, PA0: 1, PA1: 2, PA2: 3, PA3: 4, PA4: 5, PA5: 6, PA6: 7, PA7: 8, PA8: 9, PA9: 10, PA10: 11, PA11: 12, PA12: 13, PA13: 14, PA14: 15, PA15: 16
    u32 iokey_io4                        : 5;   //IOKEY按键5的IO: None: 0, PA0: 1, PA1: 2, PA2: 3, PA3: 4, PA4: 5, PA5: 6, PA6: 7, PA7: 8, PA8: 9, PA9: 10, PA10: 11, PA11: 12, PA12: 13, PA13: 14, PA14: 15, PA15: 16
    u32 iokey_num0                       : 5;   //IOKEY按键1功能: None: 0, PP/POWER: 1, P/P: 2, POWER: 3, NEXT/VOL+: 4, VOL+/NEXT: 5, PREV/VOL-: 6, VOL-/PREV: 7, VOL+: 8, VOL-: 9, NEXT: 10, PREV: 11, MODE: 12, PP/MODE: 13, HSF: 14, EQ: 15, REPEAT: 16, MUTE: 17, MODE/POWER: 18, REC: 19, PLAY/HSF: 20, A-B: 21
    u32 iokey_num1                       : 5;   //IOKEY按键2功能: None: 0, PP/POWER: 1, P/P: 2, POWER: 3, NEXT/VOL+: 4, VOL+/NEXT: 5, PREV/VOL-: 6, VOL-/PREV: 7, VOL+: 8, VOL-: 9, NEXT: 10, PREV: 11, MODE: 12, PP/MODE: 13, HSF: 14, EQ: 15, REPEAT: 16, MUTE: 17, MODE/POWER: 18, REC: 19, PLAY/HSF: 20, A-B: 21
    u32 iokey_num2                       : 5;   //IOKEY按键3功能: None: 0, PP/POWER: 1, P/P: 2, POWER: 3, NEXT/VOL+: 4, VOL+/NEXT: 5, PREV/VOL-: 6, VOL-/PREV: 7, VOL+: 8, VOL-: 9, NEXT: 10, PREV: 11, MODE: 12, PP/MODE: 13, HSF: 14, EQ: 15, REPEAT: 16, MUTE: 17, MODE/POWER: 18, REC: 19, PLAY/HSF: 20, A-B: 21
    u32 iokey_num3                       : 5;   //IOKEY按键4功能: None: 0, PP/POWER: 1, P/P: 2, POWER: 3, NEXT/VOL+: 4, VOL+/NEXT: 5, PREV/VOL-: 6, VOL-/PREV: 7, VOL+: 8, VOL-: 9, NEXT: 10, PREV: 11, MODE: 12, PP/MODE: 13, HSF: 14, EQ: 15, REPEAT: 16, MUTE: 17, MODE/POWER: 18, REC: 19, PLAY/HSF: 20, A-B: 21
    u32 iokey_num4                       : 5;   //IOKEY按键5功能: None: 0, PP/POWER: 1, P/P: 2, POWER: 3, NEXT/VOL+: 4, VOL+/NEXT: 5, PREV/VOL-: 6, VOL-/PREV: 7, VOL+: 8, VOL-: 9, NEXT: 10, PREV: 11, MODE: 12, PP/MODE: 13, HSF: 14, EQ: 15, REPEAT: 16, MUTE: 17, MODE/POWER: 18, REC: 19, PLAY/HSF: 20, A-B: 21
    u32 iokey_midkey_num                 : 5;   //两IO中间的按键功能: None: 0, PP/POWER: 1, P/P: 2, POWER: 3, NEXT/VOL+: 4, VOL+/NEXT: 5, PREV/VOL-: 6, VOL-/PREV: 7, VOL+: 8, VOL-: 9, NEXT: 10, PREV: 11, MODE: 12, PP/MODE: 13, HSF: 14, EQ: 15, REPEAT: 16, MUTE: 17, MODE/POWER: 18, REC: 19, PLAY/HSF: 20, A-B: 21
    u32 led_disp_en                      : 1;   //系统指示灯(蓝灯)
    u32 led_pwr_en                       : 1;   //电源状态灯(红灯)
    u32 rled_lowbat_en                   : 1;   //电池低电闪灯
    u32 port_2led_resless_en             : 1;   //省电阻1个IO推两个灯
    u32 bled_io_sel                      : 5;   //蓝灯IO选择: None: 0, PA0: 1, PA1: 2, PA2: 3, PA3: 4, PA4: 5, PA5: 6, PA6: 7, PA7: 8, PA8: 9, PA9: 10, PA10: 11, PA11: 12, PA12: 13, PA13: 14, PA14: 15, PA15: 16
    u32 rled_io_sel                      : 5;   //红灯IO选择: None: 0, PA0: 1, PA1: 2, PA2: 3, PA3: 4, PA4: 5, PA5: 6, PA6: 7, PA7: 8, PA8: 9, PA9: 10, PA10: 11, PA11: 12, PA12: 13, PA13: 14, PA14: 15, PA15: 16
    u8 xm_keep_start[0];                        //For Keep Area Start
    u8 zbit_auth_private_test123[20];           //授权密钥
    u8 xm_keep_end[0];                          //For Keep Area End
} xcfg_cb_t;

extern xcfg_cb_t xcfg_cb;
#endif
