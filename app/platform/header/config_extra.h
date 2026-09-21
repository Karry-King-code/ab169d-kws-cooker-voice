#ifndef __CONFIG_EXTRA_H__
#define __CONFIG_EXTRA_H__

/*****************************************************************************
 * Module    : 系统功能配置
 *****************************************************************************/
#if !AUX_2_SDADC_EN
#undef AUX_REC_EN
#define AUX_REC_EN                  0
#endif

#ifndef SYS_MODE_BREAKPOINT_EN
#define SYS_MODE_BREAKPOINT_EN      0
#endif // SYS_MODE_BREAKPOINT_EN

#ifndef WARNING_WAV_PLAY
#define WARNING_WAV_PLAY              0
#endif


/*****************************************************************************
 * Module    : 音乐功能配置
 *****************************************************************************/
#if !FUNC_MUSIC_EN
#undef  MUSIC_UDISK_EN
//#undef  MUSIC_SDCARD_EN
#undef  MUSIC_SDCARD1_EN

#undef  MUSIC_WAV_SUPPORT
#undef  MUSIC_SBC_SUPPORT
#undef  MUSIC_ID3_TAG_EN

#define MUSIC_UDISK_EN              0
//#define MUSIC_SDCARD_EN             0
#define MUSIC_SDCARD1_EN             0
#define MUSIC_WAV_SUPPORT           0
#define MUSIC_SBC_SUPPORT           0
#define MUSIC_ID3_TAG_EN            0
#endif // FUNC_MUSIC_EN

#if !MUSIC_BREAKPOINT_EN
#undef MUSIC_AB_BREAKPOINT_EN
#define MUSIC_AB_BREAKPOINT_EN      0
#endif


#if !EARPHONE_DETECT_EN
#undef SDCMD_MUX_DETECT_EARPHONE
#undef EARPHONE_DETECT_INIT
#undef EARPHONE_IS_ONLINE
#undef IS_DET_EAR_BUSY
#define EARPHONE_DETECT_INIT()
#define SDCMD_MUX_DETECT_EARPHONE   0
#define EARPHONE_IS_ONLINE()        0
#define IS_DET_EAR_BUSY()           0
#endif

#if !LINEIN_DETECT_EN
#undef SDCMD_MUX_DETECT_LINEIN
#undef LINEIN_DETECT_INIT
#undef LINEIN_IS_ONLINE
#undef IS_DET_LINEIN_BUSY

#define LINEIN_DETECT_INIT()
#define SDCMD_MUX_DETECT_LINEIN     0
#define LINEIN_IS_ONLINE()          0
#define IS_DET_LINEIN_BUSY()        0
#endif

#ifndef PWROFF_PRESS_TIME
#define PWROFF_PRESS_TIME           3               //1:大约1秒    3: 1.5s,  6: 2s,  9: 2.5s, 12: 3s
#endif // PWROFF_PRESS_TIME

#ifndef PWRON_PRESS_TIME
#define PWRON_PRESS_TIME            1500           //1.5s
#endif // PWRON_PRESS_TIME

#ifndef DOUBLE_KEY_TIME
#define DOUBLE_KEY_TIME             1
#endif // DOUBLE_KEY_TIME

#ifndef SYS_LIMIT_VOLUME
#define SYS_LIMIT_VOLUME            SYS_INIT_VOLUME
#endif // SYS_LIMIT_VOLUME

#ifndef LINEIN_2_PWRDOWN_TONE_EN
#define LINEIN_2_PWRDOWN_TONE_EN    0   //插入Linein直接关机，是否播放关机提示音。
#endif // LINEIN_2_PWRDOWN_TONE_EN

#ifndef LPWR_WARING_TIMES
#define LPWR_WARING_TIMES           0xff
#endif // LPWR_WARING_TIMES

#if !BUZZER_EN
#undef BUZZER_INIT
#define BUZZER_INIT()
#undef BUZZER_ON
#define BUZZER_ON()
#undef BUZZER_OFF
#define BUZZER_OFF()
#endif

#ifndef LED_LOWBAT_EN
#define LED_LOWBAT_EN              0
#endif // RLED_LOWBAT_EN

#ifndef RLED_LOWBAT_FOLLOW_EN
#define RLED_LOWBAT_FOLLOW_EN       0
#endif // RLED_LOWBAT_FOLLOW_EN

#ifndef SLEEP_DAC_OFF_EN
#define SLEEP_DAC_OFF_EN            1
#endif // SLEEP_DAC_OFF_EN

#ifndef PWRON_FRIST_BAT_EN
#define PWRON_FRIST_BAT_EN          1
#endif // PWRON_FRIST_BAT_EN

#if !LED_DISP_EN
#undef LED_LOWBAT_EN
#undef RLED_LOWBAT_FOLLOW_EN
#undef BLED_FFT_EN
#undef BLED_LOW2ON_EN
#undef BLED_CHARGE_FULL_EN

#define LED_LOWBAT_EN              0
#define RLED_LOWBAT_FOLLOW_EN       0
#define BLED_FFT_EN                 0
#define BLED_LOW2ON_EN              0
#define BLED_CHARGE_FULL_EN         0

#endif

#if !LED_DISP_EN
#undef LED_INIT
#define LED_INIT()
#undef LED_SET_ON
#define LED_SET_ON()
#undef LED_SET_OFF
#define LED_SET_OFF()
#endif

#if !LED_PWR_EN
#undef LED_PWR_INIT
#define LED_PWR_INIT()
#undef LED_PWR_SET_ON
#define LED_PWR_SET_ON()
#undef LED_PWR_SET_OFF
#define LED_PWR_SET_OFF()
#endif

#if ((GUI_SELECT & DISPLAY_LCD) == DISPLAY_LCD)
#define GUI_LCD_EN                      1
#else
#define GUI_LCD_EN                      0
#undef  MUSIC_NAVIGATION_EN
#define MUSIC_NAVIGATION_EN             0
#endif

#if !LOUDSPEAKER_MUTE_EN
#undef LOUDSPEAKER_MUTE_INIT
#define LOUDSPEAKER_MUTE_INIT()
#undef LOUDSPEAKER_MUTE_DIS
#define LOUDSPEAKER_MUTE_DIS()
#undef LOUDSPEAKER_MUTE
#define LOUDSPEAKER_MUTE()
#undef LOUDSPEAKER_UNMUTE
#define LOUDSPEAKER_UNMUTE()
#undef LOUDSPEAKER_UNMUTE_DELAY
#define LOUDSPEAKER_UNMUTE_DELAY        0
#endif

#ifndef IR_INPUT_NUM_MAX
#define IR_INPUT_NUM_MAX                999         //最大输入数字9999
#endif // IR_INPUT_NUM_MAX

#ifndef FMRX_THRESHOLD_VAL
#define FMRX_THRESHOLD_VAL              128
#endif // FMRX_THRESHOLD_VAL

#ifndef FMRX_SEEK_DISP_CH_EN
#define FMRX_SEEK_DISP_CH_EN            0
#endif // FMRX_SEEK_DISP_CH_EN

#ifndef SD_SOFT_DETECT_EN
#define SD_SOFT_DETECT_EN               0
#undef SD_IS_SOFT_DETECT
#define SD_IS_SOFT_DETECT()             0
#endif // SD_SOFT_DETECT_EN

#if USER_ADKEY_MUX_LED
#undef  USER_ADKEY
#undef  USER_ADKEY_MUX_SDCLK
#undef  ADKEY_PU10K_EN

#define USER_ADKEY                      1
#define USER_ADKEY_MUX_SDCLK            0
#define ADKEY_PU10K_EN                  0
#endif // USER_ADKEY_MUX_LED

//#if ((SD0_MAPPING == SD0MAP_G2) || (SD0_MAPPING == SD0MAP_G3) || (SD0_MAPPING == SD0MAP_G4)) && MUSIC_UDISK_EN && MUSIC_SDCARD_EN
//#define SD_USB_MUX_IO_EN				1
//#elif((SD1_MAPPING == SD0MAP_G2) || (SD1_MAPPING == SD0MAP_G3) || (SD1_MAPPING == SD0MAP_G4)) && MUSIC_UDISK_EN && MUSIC_SDCARD1_EN
//#define SD_USB_MUX_IO_EN				1
//#else
//#define SD_USB_MUX_IO_EN				0
//#endif

#ifndef DAC_DRC_EN
#define DAC_DRC_EN                      0
#endif


#if !I2C_EN
#undef  I2C_MUX_SD_EN
#define I2C_MUX_SD_EN               0
#endif

#if ((GUI_SELECT & DISPLAY_LCD) == DISPLAY_LCD)
#define GUI_LCD_EN                      1
#else
#define GUI_LCD_EN                      0
#endif


#ifndef EX_SPIFLASH_SUPPORT
#define EX_SPIFLASH_SUPPORT             0
#endif

/*****************************************************************************
 * Module    : 录音功能配置
 *****************************************************************************/
#if FUNC_REC_EN
#undef ASR_AINS_EN
#define ASR_AINS_EN                     0   //录音功能和降噪算法不同时使用
#endif
/*****************************************************************************
 * Module    : uart0 printf 功能自动配置(自动关闭SD卡，USB)
 *****************************************************************************/
#if (UART0_PRINTF_SEL == PRINTF_NONE)
    //关闭所以打印信息
    #undef printf
    #undef vprintf
    #undef print_r
    #undef print_r16
    #undef print_r32
    #undef printk
    #undef vprintk
    #undef print_kr
    #undef print_kr16
    #undef print_kr32

#if HUART_PRINTF_EN
    //使用高速串口打印
    #define printf(...)                     my_printf(__VA_ARGS__)
    #define vprintf(...)                    my_vprintf(__VA_ARGS__)
    #define print_r(...)                    my_print_r(__VA_ARGS__)
    #define print_r16(...)                  my_print_r16(__VA_ARGS__)
    #define print_r32(...)                  my_print_r32(__VA_ARGS__)
#else
    #define printf(...)
    #define vprintf(...)
    #define print_r(...)
    #define print_r16(...)
    #define print_r32(...)
#endif

    #define printk(...)
    #define vprintk(...)
    #define print_kr(...)
    #define print_kr16(...)
    #define print_kr32(...)
#endif

#if !MUSIC_SDCARD_EN
#undef SDCLK_MUX_DETECT_SD
#undef SD_DETECT_INIT
#undef SD_IS_ONLINE
#undef IS_DET_SD_BUSY

#define SD_DETECT_INIT()
#define SDCLK_MUX_DETECT_SD         0
#define SD_IS_ONLINE()              0
#define IS_DET_SD_BUSY()            0
#endif

#if !MUSIC_SDCARD1_EN
#undef SD1_DETECT_INIT
#undef SD1_IS_ONLINE
#undef IS_DET_SD1_BUSY

#define SD1_DETECT_INIT()
#define SD1_IS_ONLINE()              0
#define IS_DET_SD1_BUSY()            0
#endif

#ifndef USER_BOOT_EN
#define USER_BOOT_EN                 0
#endif

#if USER_BOOT_EN
#define SETUSERBIN_PARAM3            0
#else
#define SETUSERBIN_PARAM3            1
#endif

/*****************************************************************************
 * Module    : TouchKey按键配置
 *****************************************************************************/
#if (!TKEY_MUL_SCAN_EN)
#undef TKEY_TKCNT_ADJ_EN
#undef TKEY_SLIDE_DET_EN

#define TKEY_TKCNT_ADJ_EN             0
#define TKEY_SLIDE_DET_EN             0
#endif
/*****************************************************************************
 * Module    : 提示音配置
 *****************************************************************************/
#if (!WARNING_TONE_EN)
#undef WARNING_POWER_ON
#undef WARNING_POWER_OFF
#undef WARNING_FUNC_MUSIC
#undef WARNING_FUNC_BT
#undef WARNING_FUNC_CLOCK
#undef WARNING_FUNC_FMRX
#undef WARNING_FUNC_AUX
#undef WARNING_FUNC_USBDEV
#undef WARNING_FUNC_SPEAKER
#undef WARNING_LOW_BATTERY
#undef WARNING_USB_SD
#undef WARNING_MAX_VOLUME
#undef WARNING_UPDATE_DONE

#define WARNING_POWER_ON                0
#define WARNING_POWER_OFF               0
#define WARNING_FUNC_MUSIC              0
#define WARNING_FUNC_BT                 0
#define WARNING_FUNC_CLOCK              0
#define WARNING_FUNC_FMRX               0
#define WARNING_FUNC_AUX                0
#define WARNING_FUNC_USBDEV             0
#define WARNING_FUNC_SPEAKER            0
#define WARNING_LOW_BATTERY             0
#define WARNING_USB_SD                  0
#define WARNING_MAX_VOLUME              0
#define WARNING_UPDATE_DONE             0
#endif

#if ((!MUSIC_UDISK_EN) && (!MUSIC_SDCARD_EN) && (!MUSIC_SDCARD1_EN))
#undef  USB_SD_UPDATE_EN
#define USB_SD_UPDATE_EN               0
#endif


#ifndef UPD_FILENAME
#define UPD_FILENAME                   "fw5000.upd"
#endif

#if ((SD0_MAPPING == SD0MAP_G1) || (SD0_MAPPING == SD0MAP_G2) || (SD0_MAPPING == SD0MAP_G3) || (SD0_MAPPING == SD0MAP_G4))
#define SDCLK_AD_CH                    ADCCH_PB1
#elif (SD0_MAPPING == SD0MAP_G6)
#define SDCLK_AD_CH                    ADCCH_PA1
#else
#undef  USER_ADKEY_MUX_SDCLK
#define USER_ADKEY_MUX_SDCLK           0
#endif

#define SD_SUPPORT_EN                  (MUSIC_SDCARD_EN | MUSIC_SDCARD1_EN)

#if MUSIC_SDCARD_EN
#define SDCMD_MUX_DETECT_EN            1
#define SDCLK_MUX_DETECT_EN            1
#else
#undef  USER_ADKEY_MUX_SDCLK
#undef  I2C_MUX_SD_EN

#define I2C_MUX_SD_EN                  0
#define SDCMD_MUX_DETECT_EN            0
#define SDCLK_MUX_DETECT_EN            0
#define USER_ADKEY_MUX_SDCLK           0
#endif // MUSIC_SDCARD_EN

#if !SD_SUPPORT_EN
#undef  UDE_STORAGE_EN
#define UDE_STORAGE_EN                 0
#endif

#if UDE_FOT_EN
#undef  UDE_STORAGE_EN
#define UDE_STORAGE_EN                 1
#endif

#if !VBAT_DETECT_EN
#undef LOW_VBAT_POWER_ON
#undef POWER_ON_MIN_VBAT
#define LOW_VBAT_POWER_ON              0
#define POWER_ON_MIN_VBAT              0
#endif

#if EX_SPIFLASH_SUPPORT && ((!DAC_EN) || (!(WARNING_MP3_PLAY || WARNING_WAV_PLAY)) || (!WARNING_TONE_EN))
#error "DUMP warning: please set DAC_EN if ues EX_SPIFLASH_SUPPORT and WARNING_TONE_EN\n"
#endif

#if WARNING_TONE_EN && (!DAC_EN)
#error "DUMP warning: please set DAC_EN if ues WARNING_TONE_EN\n"
#endif

#if (SPK_DNN_EN && ASR_RECOG_EN) || (SPK_DNN_EN && FUNC_REC_EN) || (SPK_DNN_EN && ASR_AINS_EN)
#error "DUMP warning: please disable ASR_RECOG_EN && FUNC_REC_EN&& ASR_AINS_ENSPK_DNN_EN if ues SPK_DNN_EN\n"
#endif

#if USER_BOOT_UPDATE_EN
#undef WEIGHT_BIN_ADDR
#undef WEIGHT_MINI_BIN_ADDR
///用boot 升级的时候，在原来的地址上weight.bin + 0x7000
#define WEIGHT_BIN_ADDR         0x2F000      //(0x28000 + 0x7000)
#define WEIGHT_MINI_BIN_ADDR    0x18000      //(0x11000 + 0x7000)
#endif

#endif // __CONFIG_EXTRA_H__

