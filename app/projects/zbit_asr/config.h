/*****************************************************************************
 * Module    : Config
 * File      : config.h
 * Function  : SDK配置文件
 *****************************************************************************/

#ifndef USER_CONFIG_H
#define USER_CONFIG_H
#include "config_define.h"

/*****************************************************************************
 * Module    : Function选择相关配置
 *****************************************************************************/
#define FUNC_MUSIC_EN                   0 //是否打开MUSIC功能
#define FUNC_SPEAKER_EN                 1 //是否打开Speaker模式
#define FUNC_IDLE_EN                    0 //是否打开IDLE功能

/*****************************************************************************
 * Module    : 系统功能选择配置
 *****************************************************************************/
#define SYS_CLK_SEL                     SYS_192M                    //选择系统时钟
#define GPIO_10S_RESET                  0                           //长按复位，默认PA10，和短按复位只能二选一
#define GPIO_10S_SEL                    IO_PA10                     //长按复位IO选择
#define MCLR_S_RESET                    0                           //短按复位，固定PA11，和长按复位只能二选一
#define SOFT_POWER_ON_OFF               1                           //是否使用软开关机功能
#define SYS_SLEEP_TIME                  0                           //自动休眠时间(time * 1秒)
#define SYS_OFF_TIME                    0                           //自动关机时间(time * 30秒)
#define POWER_DOWN_MODE                 PWR_L3M                     //PWR_DOWN模式
#define POWER_ON_FALL_NUM               1                           //保留1路占位(用未占用脚)，避免NUM=0零长数组；勿用PA4/PA1
#define POWER_ON_FALL_IO                {IO_PA10}                   //模组脚1组，本产品无LED/按键
#define POWER_ON_RISE_NUM               0                           //高电平唤醒IO个数(0-即不使用)
#define POWER_ON_RISE_IO                {IO_NONE}                   //高电平唤醒IO
#define POWER_OFF_IO                    IO_PA10                     //关机等待松开IO；勿用冷暖PWM脚
#define PWROFF_MIC_BIAS_EN              0                           //是否关机后保持mic bias电压
#define GUI_SELECT                      GUI_NO                      //GUI Display Select
#define PIN_SOP_SEL                     2                           //芯片SOP引脚数选择:0-SOP16, 1-SOP8, 2-使用efuse默认值
#define SYS_BRAM_EN                     0                           //是否使能bram(使能后会从cache中锁住4k作为bram使用)
#define UART0_PRINTF_SEL                PRINTF_PA8                 //调试串口：SOP8脚6为PA1/2/3同键，勿用PA2(烧录UTX)；PA7/8/9已被LED占用故不用PA8；PA3为空闲调试脚，接J2调试口
#define USER_BOOT_UPDATE_EN             0                           //是否开启用户二次BOOT升级功能

/*****************************************************************************
 * Module    : FLASH配置
 *****************************************************************************/
#define FLASH_SIZE                      FSIZE_512K //芯片内置Flash 512KB，实际导出prd文件要小于496K, param占用16k)
#define FLASH_CODE_SIZE                 303K                    //程序使用空间大小
#define FLASH_ERASE_4K                  1                       //是否支持4K擦除
#define FLASH_DUAL_READ                 1                       //是否支持2线模式
#define FLASH_QUAD_READ                 0                       //是否支持4线模式
#define SPIFLASH_SPEED_UP_EN            1                       //SPI FLASH提速。
#define SPIFLASH0_RW_EN                 0                       //内置flash读写接口,Flash UID读取示例

#define SYS_BRAM_EN                     0                       //是否启用bram(bram会占用cache空间)
#define SYS_BRAM_LEN                    0x1000                  //0x1000->bram为4K可用, 0x2000->bram为8K可用

/*****************************************************************************
* Module    : ASR配置控制
******************************************************************************/
#define ASR_RECOG_EN                    1                       //是否使能语音识别功能
#define ASR_VAD_EN                      0                       //是否使能VAD功能
#define ASR_LIBS_SEL                    ASR_ZH                  //选择中英文识别库
#define MIC_CHANNEL_CFG                 CH_MIC_PA0              //选择MIC的通路
#define MIC_BRES_SEL                    4                       //MICIN RES
#define ASR_AINS_EN                     0                       //是否打开降噪功能
#define ASR_AINS3_LEVEL				    (0)                     //级别-20~30(0是平稳噪音降20db, -5是-15db, 6是-26db)
#define ASR_MIC_MAXPOW_EN               0                       //MIC能量检测使能

#define TDNN_HW_EN                      0                       //硬件TDNNACC
#define WEIGHT_MINI_EN                  1                       //0-标准模型,1-mini模型

#define ASR_SKIP_SAMPLES                1                       //ASR跳过前5帧(mini->4帧)
#define WEIGHT_BIN_ADDR                 0x28000                 //weight.bin/weight_en.bin存放地址
#define WEIGHT_BIN_LEN                  0x37000                 //weight.bin长度
#define WEIGHT_DATA_LEN                 0x37000                 //weight.bin长度
#define WEIGHT_EN_BIN_LEN               0x36000                 //weight_en.bin长度
#define WEIGHT_EN_DATA_LEN              0x35C00                 //weight_en.bin长度

#define WEIGHT_MINI_BIN_ADDR            0x11000                 //weight_mini.bin存放地址
#define WEIGHT_MINI_BIN_LEN             0x28000                 //weight_mini.bin长度
#define WEIGHT_MINI_DATA_LEN            0x279C0                 //weight_mini数据实际长度

#define SPK_DNN_EN                      0                       //使能Speaker单麦DNN降噪算法
#define SPK_DNN_LEVEL                   6                       //降噪量：0-30级

#define FFT_PROC_SAMPLE                 0                       //FFT处理示例
#define SPK_SPI_DUMP                    0                       //speaker data spi dump
#define HUART_DEUMP_EN                  0                       //Huart dump en

/*****************************************************************************
 * Module    : 音乐功能配置
 *****************************************************************************/
#define MUSIC_UDISK_EN                  0   //是否支持播放UDISK
#define MUSIC_SDCARD_EN                 0   //是否支持播放SDCARD

#define MUSIC_WAV_SUPPORT               0   //是否支持WAV格式解码
#define MUSIC_SBC_SUPPORT               0   //是否支持SBC格式解码

#define MUSIC_FOLDER_SELECT_EN          0   //文件夹选择功能
#define MUSIC_AUTO_SWITCH_DEVICE        0   //双设备循环播放
#define MUSIC_BREAKPOINT_EN             0   //音乐断点记忆播放
#define MUSIC_AB_BREAKPOINT_EN          0   //AB断点记忆播放，需在MUSIC_BREAKPOINT_EN使能的情况下才能使用
#define MUSIC_QSKIP_EN                  0   //快进快退功能
#define MUSIC_PLAYMODE_NUM              4   //音乐播放模式总数
#define MUSIC_MODE_RETURN               0   //退出音乐模式之后是否返回原来的模式
#define MUSIC_PLAYDEV_BOX_EN            0   //是否显示“USB”, "SD"界面
#define MUSIC_ID3_TAG_EN                0   //是否获取MP3 ID3信息
#define MUSIC_REC_FILE_FILTER           0   //是否区分录音文件与非录音文件分别播放
#define MUSIC_LRC_EN                    0   //是否支持歌词显示
#define MUSIC_NAVIGATION_EN             0   //音乐文件导航功能(LCD点阵屏功能)
#define MUSIC_ENCRYPT_EN                0   //是否支持加密MP3文件播放(使用MusicEncrypt.exe工具进行MP3加密)
#define MUSIC_MP3_LOOPBACK_EN           0   //是否开启MP3音乐循环播放
#define MUSIC_FILE_SORT_CHAR_EN         0   //是否将文件按字符排序
#define MUSIC_FILE_SORT_NUM             20  //参与排序文件数上限

#define MUSIC_ENCRYPT_KEY               12345   //MusicEncrypt.exe工具上填的加密KEY

/*****************************************************************************
* Module    : DAC配置控制
******************************************************************************/
#define DAC_EN                          1                   //DAC使能-->PA3_DACout
#define DAC_MAP_EN                      0                   //CLASSD MAP EN
#define DAC_CLASSD_P                    FO_PA8              //CLASSD_P MAP
#define DAC_CLASSD_N                    FO_PA9              //CLASSD_N MAP
#define DAC_CH_SEL                      0                   //DAC_MONO ~ DAC_VCMBUF_DUAL
#define DAC_MAX_GAIN                    9                   //配置DAC最大模拟增益，默认设置为dac_vol_table[VOL_MAX]
#define DAC_OUT_SPR                     DAC_OUT_48K         //dac out sample rate
#define DAC_LDOH_SEL                    3
#define DAC_VCM_CAPLESS_EN              true                //DAC VCM省电容方案,使用内部VCM
#define DACVDD_LDO_SEL                  2                   //DACVDD供电选择： 0->外部电容独立LDO， 1->BYPASS, 2->CAPLESS
#define DAC_PULL_DOWN_DELAY             80                  //控制DAC隔直电容的放电时间, 无电容时可设为0，减少开机时间。
#define DAC_DNR_EN                      1                   //是否使能动态降噪(压播放底噪)
#define DAC_DRC_EN                      0                   //是否使能DRC功能（暂不支持录音、Karaok），注：暂不支持使用
#define DAC_SOFT_EQ_EN                  0                   //是否使能软件EQ功能（DRC前）
#define DAC_SOFT_DIFF_MONO_MIX_EN       0                   //DAC软件差分单声道输出，DACR和DACL输出差分单声道信号
#define AUDIO_STRETCH_EN				0                   //变速不变调,系统时钟需要设置成120M及以上,目前只有播放MP3才能支持变速不变调
#define DAC_FAST_SETUP_EN               0                   //DAC快速上电，有噪声需要外部功放MUTE
#define DAC_MUTE_IO_SEL                 IO_NONE             //DAC快速上电，外部功放MUTE IO选择
#define DAC_MUTE_STA_SEL                0 			        //DAC快速上电，MUTE外部DAC的状态选择，0：低mute  1：高mute

/*****************************************************************************
 * Module    : 录音功能配置
 *****************************************************************************/
#define FUNC_REC_EN                     0   //录音功能总开关
#define INTERNAL_FLASH_REC              0   //录音到内置flash
#define FMRX_REC_EN                     0   //是否打开FM录音功能
#define AUX_REC_EN                      0   //是否打开AUX录音功能
#define MIC_REC_EN                      0   //是否打开MIC录音功能
#define REC_ONE_FOLDER_EN               0   //是否各模式录音放到同一目录下
#define REC_DIR_LFN                     0   //是否打开长文件名目录
#define REC_AUTO_PLAY                   0   //录音结束是否回放当前录音内容
#define REC_FAST_PLAY                   0   //播卡播U下快速播放最新的录音文件(双击REC)
#define REC_STOP_MUTE_1S                0   //录音停止时, MUTE 1S功放. //提醒客户录音结束.
#define REC_TYPE_SEL                    REC_MP3
#define REC_DIG_GAIN_EN                 0    //录音是否需要加大数字增益
#define REC_AUTO_END_EN                 0   //flash录音自动覆盖并自动停止功能
#define REC_AUTO_END_TIME               10  //flash自动停止录音时间(s)

/*****************************************************************************
 * Module    :外接SPIFLASH配置, 外接SPIFLASH可以播放MP3音乐文件, 及录音，同时要开启DAC_EN
 *****************************************************************************/
#define EX_SPIFLASH_SUPPORT             0               //可以配置为 EXSPI_NOT_SUPPORT(0) 或 EXSPI_MUSIC 或 EXSPI_REC 或 (EXSPI_MUSIC | EXSPI_REC)
#define FLASH_GREATER_THAN_16MBYTE      0

#define SPIFLASH_BAUD                   (500000)        //SPI波特率500K
#define SPIFLASH_MAP                    SPI1MAP_G2      //SPI CLK/DI/DO IO
#define SPIFLASH_CS_IO                  IO_PA6          //SPI CS IO

#if (EX_SPIFLASH_SUPPORT & EXSPI_MUSIC)
//FLASH_MUSIC.BIN 镜像文件占用区域(BYTE)
#define SPIFLASH_MUSIC_BEGIN_ADDR       0               //FLASH_MUSIC.BIN镜像文件默认从0地址开始存放,此宏暂不支持修改.
#define SPIFLASH_MUSIC_END_ADDR         (1024*36)       //FLASH_MUSIC.BIN镜像文件结束地址. 测试DEMO的镜像文件是36K大小.
#define SPIFLASH_MUSIC_BIN_WRITE_TEST   0               //默认的FLASH_MUSIC.BIN写入SPIFLASH, 可以在func_exspiflash_music中测试外接SPIFLASH播放MP3.

#define EXSPI_WAV_EN                    1               //是否支持exspiflash  wav音频播放
#define EXSPI_MP3_EN                    1               //是否支持exspiflash  mp3音频播放
#define EXSPI_VSBC_EN                   1               //是否支持exspiflash vsbc音频播放
#define EXSPI_ESBC_EN                   1               //是否支持exspiflash esbc音频播放
#endif

#if (EX_SPIFLASH_SUPPORT & EXSPI_REC)
//录音占用区域(BYTE)  //注意SPIFLASH的录音区域不要覆盖 FLASH_MUSIC.BIN区域
#if INTERNAL_FLASH_REC
#define SPIFLASH_REC_BEGIN_ADDR         0x5F000         //录音起始地址
#define SPIFLASH_REC_END_ADDR           0x6F000         //录音结束地址
#else
#define SPIFLASH_REC_BEGIN_ADDR         (1024*37)       //录音起始地址
#define SPIFLASH_REC_END_ADDR           (1024*1024)     //录音结束地址
#endif
#endif
#define EXSPI_REC_AUTOTUNE_PLAY         0               //SPIFLASH音调变调  //Auto Tune
#define SPIFLASH_MUSIC_PLAY_REC         0               // 1 FUNC_EXSPIFLASH_MUSIC 模式下播放录音示例： 支持录音上下曲，及删除当前播放的录音  //0 播放镜像音乐示例
#define SPIFLASH_MUSIC_BREAK_MEMORY     0               //断点记忆，切出flash模式时记忆断点，再次切入flash播放时从上次记忆的断点开始播放，加入100ms断电记忆一次功能
#define SPIFLASH_MUSIC_BREAK_LOOP       0               //0-打开断点记忆时,对当前音乐循环播放 1-打开断点记忆时,对Flash中的所有歌曲循环播放
#define SPIFLASH_MUSIC_SEAMLESS_LOOP    0               //1-对当前歌曲开启无缝循环播放

#if (EX_SPIFLASH_SUPPORT && (!FUNC_REC_EN))             //TEST CONFIG
#undef GUI_SELECT
#undef FUNC_REC_EN
#undef MIC_REC_EN
#undef REC_AUTO_PLAY

#define GUI_SELECT                      GUI_NO
#define FUNC_REC_EN                     0
#define MIC_REC_EN                      0
#define REC_AUTO_PLAY                   0
#endif

#if EXSPI_REC_AUTOTUNE_PLAY
#define SAMPLE_RATE      16000                       //采样率目前仅支持16k
#endif
/*****************************************************************************
 * Module    : User按键配置 (可以同时选择多组按键)
 *****************************************************************************/
#define USER_ADKEY                      0           //ADKEY的使用， 0为不使用
#define USER_ADKEY2                     0           //ADKEY2的使用，0为不使用
#define USER_IOKEY                      0           //IOKEY的使用， 0为不使用（冷暖灯无按键）
#define IOKEY_HIGH_TRIGGER              0           //IOKEY高电平触发，0：低电平触发， 1：高电平触发
#define USER_ADKEY_MUX_SDCLK            0           //是否使用复用SDCLK的ADKEY, 共用USER_ADKEY的按键table
#define USER_ADKEY_MUX_LED              0           //是否使用ADKEY与LED复用, 共用USER_ADKEY的流程(ADKEY与BLED配置同一IO)
#define ADKEY_PU10K_EN                  0           //ADKEY是否使用内部10K上拉, 按键数量及阻值见port_key.c

#define USER_KEY_KNOB_EN                0           //旋钮的使用，0为不使用
#define USER_KEY_KNOB_LEVEL             16          //旋钮的级数

#define USER_MULTI_PRESS_EN             0           //按键N击检测使能
#define USER_KEY_DOUBLE                 {KU_REC, KU_PLAY_POWER, KU_PLAY,KU_VOL_DOWN, KU_VOL_UP}        //支持双击/三击/四击/五击的按键
#define ADKEY_CH                        xcfg_cb.adkey_ch    //ADCCH_PA0
#define ADKEY2_CH                       xcfg_cb.adkey2_ch   //ADCCH_PA1

#define IS_PWRKEY_PRESS()			    0
#define DOUBLE_KEY_TIME                 (1)                       //按键双击响应时间（单位50ms）
#define PWRON_PRESS_TIME                (500*xcfg_cb.pwron_press_time)                  //长按PWRKEY多长时间开机？
#define PWROFF_PRESS_TIME               (3+3*xcfg_cb.pwroff_press_time)                 //长按PWRKEY多长时间关机？

#define WKU_IO_PWRON_EN                 0                               //使能固定的IO唤醒(此唤醒IO之间互不影响,仅支持PA7/PB0/PB1/PB2/PB9/PA0)
#define WKU_IO_FALL_NUM                 3                               //固定唤醒的下降沿IO个数(0-即不使用)
#define WKU_IO_FALL_GROUP               {WKU_PA1, WKU_PA2, WKU_PA4}     //固定唤醒的下降沿IO
#define WKU_IO_RISE_NUM                 3                               //固定唤醒的上升沿IO个数(0-即不使用)
#define WKU_IO_RISE_GROUP               {WKU_PA9, WKU_PA10, WKU_PA13}   //固定唤醒的上升沿IO

#define MUL_PWRON_IO_EN                 0           //demo功能:使能多个IO唤醒,IO能唤醒和触发KEY_NUM,仅支持触发短按
#define MUL_PWRON_FALL_NUM              8           //下降沿唤醒IO个数,可设置为0,即不使用
#define MUL_PWRON_FALL_GROUP            {IO_PA0, IO_PA1, IO_PA3, IO_PA4, IO_PA5, IO_PA6, IO_PA7, IO_PA8}
#define MUL_PWRON_RISE_NUM              7           //上升沿唤醒IO个数,可设置为0,即不使用
#define MUL_PWRON_RISE_GROUP            {IO_PA9, IO_PA10, IO_PA11, IO_PA12, IO_PA13, IO_PA14, IO_PA15}

#define MIX_PWR_DOWN_EN                 0           //demo功能:sfunc_lowbat->sfunc_pwroff,同时使用两模式
#define MIX_PWR_SLEEP_IO                IO_PA4      //休眠IO
#define MIX_PWR_SLEEP_UP_IO             IO_PA12     //休眠唤醒IO
#define MIX_PWR_DOWN_IO                 IO_PA6      //关机IO
#define MIX_PWR_DOWN_UP_IO              IO_PA10     //关机唤醒IO

/*****************************************************************************
 * Module    : TouchKey按键配置
 *****************************************************************************/
#define TKEY_MUL_SCAN_EN                0                                     //启用Touchkey按键 (需要开按键N击检测使能Tkey才有多击功能)
#define TKEY_MUL_SCAN_NUM               1                                     //支持多通道，对应修改下面TKEY_ARRAY数组
#define TKEY_ARRAY                      {IO_PA4}              				  //例如开启两个通道则数组写成{IO_PA4, IO_PA5}, 多个通道同理，不可使用高压GPIO
#define TKEY_TKCD_PR                    50                                    //触控采集的周期，一般不需要修改
#define TKEY_PRESS_UPDATE               0                                     //触控按压值动态更新，默认时间为3000ms
#define TKEY_PRESS_UPDATE_TIME          3000								  //更新时间长度
#define TKEY_SLEEP_MODE                 0                                     //开启触控进入func_sleepmode模式

#define TKEY_TKCNT_ADJ_EN               1                                     //是否启用自动调整Touchkey按键键值
#define TKEY_TKCNT_ADJ_TIMES            200                                   //TKEY_TKCNT采集次数
#define TKEY_TKCNT_ADJ_LEVEL            20                                    //1 ~ 100 touchkey按键灵敏度

#define TKEY_FILTER_EN                  1                                     //触控软件算术平均值滤波
#define TKEY_FILTER_TIMES               10                                    //连续取N个采样值进行算术平均

#define TKEY_SCAN_SWITCH_EN             0                                     //切换Touchkey扫描到大循环demo，只支持单击
#define TKEY_SCAN_TIME                  5                                     //省去按键流程的扫描时间，单位：ms
#define TKEY_BUF_DOWN_EN                0                                     //增加写tkbuf数据到flash接口                                  //1 ~ 100 touchkey按键灵敏度

#define TKEY_SLIDE_DET_EN               0
#define TKEY_SLIDE_BUF                  {KU_PREV, KU_NEXT, KU_VOL_DOWN, KU_PLAY, KU_VOL_UP} //滑动判断顺序，至左向右
/*****************************************************************************
 * Module    : 矩阵按键配置
 *****************************************************************************/
#define MAXTRIX_KEY_EN                  0            //是否使用44按键矩阵键盘(需要使用8个IO，目前支持只支持PA8~PA15)

#define MAX_KEY_0                       IO_PA8
#define MAX_KEY_1                       IO_PA9
#define MAX_KEY_2                       IO_PA10
#define MAX_KEY_3                       IO_PA11
#define MAX_KEY_4                       IO_PA12
#define MAX_KEY_5                       IO_PA13
#define MAX_KEY_6                       IO_PA14
#define MAX_KEY_7                       IO_PA15

/*****************************************************************************
 * Module    : SD0配置
 *****************************************************************************/
#define SD_SOFT_DETECT_EN               0           //是否使用软件检测（SD发命令检测）
#define SDCLK_MUX_DETECT_SD             0           //是否复用SDCLK检测SD卡
#define SD0_MAPPING                     SD0MAP_G1   //选择SD0 mapping
#define SD1_MAPPING                     SD0MAP_G6   //选择SD1 mapping

///通过配置工具选择检测GPIO
#define SD_DETECT_INIT()                sdcard_detect_init()
#define SD_IS_ONLINE()                  sdcard_is_online()
#define IS_DET_SD_BUSY()                is_det_sdcard_busy()
#define SD_IS_SOFT_DETECT()             (xcfg_cb.sddet_iosel == 31)  //配置工具中选则31是软件检测

#define SD1_DETECT_INIT()               sdcard1_detect_init()
#define SD1_IS_ONLINE()                 sdcard1_is_online()
#define IS_DET_SD1_BUSY()               is_det_sdcard1_busy()

/*****************************************************************************
 * Module    : I2C配置
 *****************************************************************************/
#define I2C_HW_EN                       0           //是否使能硬件I2C功能
#define I2C_HW_EN_TEST                  0           //测试硬件I2C
#define I2C_MAPPING                     I2CMAP_PA3PA4 //选择I2C mapping

#define I2C_SW_EN                       0           //是否使能软件I2C功能
#define I2C_SW_EN_TEST                  0           //测试软件I2C
#define I2C_MUX_SD_EN                   0           //是否I2C复用SD卡的IO

#if I2C_MUX_SD_EN
#define I2C_SCL_IN()                    SD_CMD_DIR_IN()
#define I2C_SCL_OUT()                   SD_CMD_DIR_OUT()
#define I2C_SCL_H()                     SD_CMD_OUT_H()
#define I2C_SCL_L()                     SD_CMD_OUT_L()

#define I2C_SDA_IN()                    SD_DAT_DIR_IN()
#define I2C_SDA_OUT()                   SD_DAT_DIR_OUT()
#define I2C_SDA_H()                     SD_DAT_OUT_H()
#define I2C_SDA_L()                     SD_DAT_OUT_L()
#define I2C_SDA_IS_H()                  SD_DAT_STA()
#else
#define I2C_SCL_IN()                    {GPIOADIR |= BIT(1); GPIOAPU   |= BIT(1);}
#define I2C_SCL_OUT()                   {GPIOADE  |= BIT(1); GPIOADIR &= ~BIT(1);}
#define I2C_SCL_H()                     {GPIOASET  = BIT(1);}
#define I2C_SCL_L()                     {GPIOACLR  = BIT(1);}

#define I2C_SDA_IN()                    {GPIOADIR |= BIT(2); GPIOAPU   |= BIT(2);}
#define I2C_SDA_OUT()                   {GPIOADE  |= BIT(2); GPIOADIR &= ~BIT(2);}
#define I2C_SDA_H()                     {GPIOASET  = BIT(2);}
#define I2C_SDA_L()                     {GPIOACLR  = BIT(2);}
#define I2C_SDA_IS_H()                  (GPIOA & BIT(2))
#endif //I2C_MUX_SD_EN

#define I2C_SDA_SCL_OUT()               {I2C_SDA_OUT(); I2C_SCL_OUT();}
#define I2C_SDA_SCL_H()                 {I2C_SDA_H(); I2C_SCL_H();}
#define I2C_IO_INIT()                   {I2C_SDA_SCL_OUT(); I2C_SDA_H();}

/*****************************************************************************
 * Module    :SPI1配置
 *****************************************************************************/
#define SPI1_AUDIO_EN                   0           //SPI1音频发送接收总使能位
#define SPI_SLAVE_OR_MASTER_EN          0           //0-SPI从机, 1-SPI主机

#if SPI1_AUDIO_EN
#define SPI_AUDIO_TEST_EN               1           //SPI1提示音收发测试
#define     SPI_TXBUF_SIZE              576
#define     SPI_RX_MAX                  2
#define     SPI_RXBUF_SIZE              576
#endif

/*****************************************************************************
 * Module    : IRRX配置
 *****************************************************************************/
#define IRRX_SW_EN                      0           //是否打开timer capture ir
#define IR_NUMKEY_EN                    0           //是否打开数字键输入
#define IR_INPUT_NUM_MAX                999         //最大输入数字9999

//可以打开一个或多个
#define IR_ADDR_FF00_EN                 0
#define IR_ADDR_BF00_EN                 0
#define IR_ADDR_FD02_EN                 0
#define IR_ADDR_FE01_EN                 0
//#define IR_ADDR_7F80_EN                 1

#define IR_CAPTURE_PORT()               {GPIOADE |= BIT(7); GPIOAPU  |= BIT(7); GPIOADIR |= BIT(7);}
#define IRRX_MAPPING                    IO_PA7

/*****************************************************************************
 * Module    : IRTX配置
 *****************************************************************************/
#define IRTX_HW_EN                      0        //是否开启硬件IR tx
#define IRTX_MAPPING                    IO_PA4   //可选PA2 PA4 PA10 PA12

/*****************************************************************************
 * Module    :huart配置
 *****************************************************************************/
#define HUART_EN                        1                   //是否使能huart
#define HUART_TX_MAP                    HSTX_PA10            //Huart tx io
#define HUART_RX_MAP                    HSTX_PA10            //Huart rx io
#define HUART_DMA_EN                    1                   //Huart DMA收发使能
#define HUART_PRINTF_EN                 0                   //Huart 打印信息输出使能(DEBUG实测=1会死机: printf模式裁掉RX/IRQ路径与命令通道冲突, 勿开)
#define HUART_DEBUG_TELEMETRY           0                   //识别遥测帧(2026-08-31调参用; 已定案出正式版=0; 需要时改1: 未命中候选帧发[A5 FB top prob thr CK FB]到PA10)
#define HUART_TX_TEST_EN                0                   //Huart tx发送测试
#define HUART_RX_TEST_EN                0                   //Huart rx接收打印
#define HUART_CROSSBAR                  0                   //crossbar的huart仅支持单工模式

#if HUART_CROSSBAR
#undef  HUART_TX_MAP
#undef  HUART_RX_MAP

#define HUART_CROSSBAR_IO               IO_PA7              //hsuart crossbar only support oneline mode
#endif
/*****************************************************************************
 * Module    : UART0/1配置
 *****************************************************************************/
#define USER_UART0_EN                   1                   //是否开启uart0(开启后需要设UART0_PRINTF_SEL为PRINTF_NONE)
#define UART0_TX_PORT                   UTX0MAP_PA8
#define UART0_RX_PORT                   URX0MAP_PA13
#define UART0_DUPLEX_SEL                0                   //0-半双工模式，1-全双工模式
#define UART0_5V_TRX_EN                 0                   //uart0 5V TX/RX使能(仅PA12~PA15支持5V高压, 需要外部5V上拉)
#define UART0_TX_TEST_EN                0                   //uart0 tx发送测试
#define UART0_RX_TEST_EN                0                   //uart0 rx接收打印
#define UART_CROSSBAR                   0                   //crossbar的uart仅支持单工模式

#if UART_CROSSBAR
#undef  UART0_TX_PORT
#undef  UART0_RX_PORT

#define UART_TX_OR_RX                   0                   //0-TX 1-RX
#define UART_CROSSBAR_IO                IO_PA6              //uart crossbar only support oneline mode
#endif

#define UART_AUDIO_EN                   0                   //UART音频测试
#define MIC_ESBC_PROC_EN                0                   //mic进行esbc编解码算法


/*****************************************************************************
 * Module    : PWM配置
 *****************************************************************************/
#define PWM_TMR2_EN                     1                               //是否开启TIMER2 PWM输出功能，详细配置需要在tmr2pwm_cfg_init内配置
#define PWM_TMR2_IO_MAP_EN              1                               //是否映射到其他IO，1：映射  0：不映射，则使用默认Group（G1、G2、G3）,若映射需要占用crossbar通道，若选择0，选择
#define PWM_TMR2_MAP_SEL                {IO_NONE,IO_NONE,IO_NONE,IO_NONE}   //开机不映射；冷暖PWM由led_ctrl_init()单独配置PA1/PA4
#define PWM_TMR2_GRP_MAP_SEL            PWM_MAPPING_G3                  //PWM tmr2需要映射的Group，可选 G1 G2 G3


/*****************************************************************************
 * Module    : 系统细节配置
 *****************************************************************************/
#define RGB_SERIAL_EN                   0           //串行RGB推灯功能
#define RGB_SPI_PORT                    SPI1MAP_G1  //RGB数据TX口,默认只使能SPIDO口
#define RGB_SPI_CROSSBAR                0           //crossbar的SPI使能
#define RGB_CROSSBAR_IO                 IO_PB2      //crossbar的SPI IO

#define TMR2_US_EN                      0           //timer2做10us定时器
#define SYS_PARAM_RTCRAM                0           //是否系统参数保存到RTCRAM
#define VBAT_DETECT_EN                  0           //电池电量检测功能
#define LOW_VBAT_POWER_ON               0           //低电开机电压功能
#define POWER_ON_MIN_VBAT               2500        //最小开机电压2.5V（2500/1000），最小应大于2.3V
#define VBAT2_ADCCH                     ADCCH_VBAT  //ADCCH_VBAT为内部1/2电压通路，带升压应用需要外部ADC通路检测1/2电池电压
#define VBAT_FILTER_USE_PEAK            0           //电池检测滤波选则://0 取平均值.//1 取峰值(适用于播放音乐时,电池波动比较大的音箱方案).
#define EQ_MODE_EN                      0           //是否调节EQ MODE (POP, Rock, Jazz, Classic, Country)
#define EQ_DBG_IN_UART                  0           //是否使能UART在线调节EQ
#define SYS_INIT_VOLUME                 xcfg_cb.sys_init_vol        //系统默认音量
#define LPWR_WARNING_VBAT               xcfg_cb.lpwr_warning_vbat   //低电提醒电压
#define LPWR_OFF_VBAT                   xcfg_cb.lpwr_off_vbat       //低电关机电压
#define LOWPWR_REDUCE_VOL_EN            0                           //低电是否降低音量
#define LPWR_WARING_TIMES               0xff                        //报低电次数
#define FREQ_DET_EN                     0                           //是否开启频率检测
#define VDDIO_FOLLOW_VBAT_EN            0                           //是否开启vddio跟随vbat,[reset_sel, vddio_sel]
#define LVD_IRQ_EN                      0                           //是否开启LVD低电压中断

/*****************************************************************************
 * Module    : LED指示灯配置
 *****************************************************************************/
#define LED_DISP_EN                     0           //是否使用LED指示灯(蓝灯)
#define LED_PWR_EN                      0           //充电及电源指示灯(红灯)
#define LED_LOWBAT_EN                   0           //电池低电是否闪红灯

#define LED_INIT()                      bled_func.port_init(&bled_gpio)
#define LED_SET_ON()                    bled_func.set_on(&bled_gpio)
#define LED_SET_OFF()                   bled_func.set_off(&bled_gpio)

#define LED_PWR_INIT()                  rled_func.port_init(&rled_gpio)
#define LED_PWR_SET_ON()                rled_func.set_on(&rled_gpio)
#define LED_PWR_SET_OFF()               rled_func.set_off(&rled_gpio)


/*****************************************************************************
 * Module    : Loudspeaker mute及耳机检测配置
 *****************************************************************************/
#define EARPHONE_DETECT_EN              0           //是否打开耳机检测
#define SDCMD_MUX_DETECT_EARPHONE       0           //是否复用SDCMD检测耳机插入

///通过配置工具选择检测GPIO
#define EARPHONE_DETECT_INIT()          earphone_detect_init()
#define EARPHONE_IS_ONLINE()            earphone_is_online()
#define IS_DET_EAR_BUSY()               is_detect_earphone_busy()

#define LOUDSPEAKER_MUTE_EN             0           //是否使能功放MUTE
#define LOUDSPEAKER_MUTE_INIT()         loudspeaker_mute_init()
#define LOUDSPEAKER_MUTE_DIS()          loudspeaker_disable()
#define LOUDSPEAKER_MUTE()              loudspeaker_mute()
#define LOUDSPEAKER_UNMUTE()            loudspeaker_unmute()
#define LOUDSPEAKER_UNMUTE_DELAY        6           //UNMUTE延时配置，单位为5ms

#define AMPLIFIER_SEL_INIT()            amp_sel_cfg_init(0)
#define AMPLIFIER_SEL_D()               amp_sel_cfg_d()
#define AMPLIFIER_SEL_AB()              amp_sel_cfg_ab()


/*****************************************************************************
 * Module    : 提示音 功能选择
 *****************************************************************************/
#define WARNING_TONE_EN                 1            //是否打开提示音功能, 总开关
#define WARING_MAXVOL_MP3               0            //最大音量提示音WAV或MP3选择， 播放WAV可以与MUSIC叠加播放。
#define WARNING_MP3_PLAY                1            //是否支持MP3提示音播放
#define WARNING_WAV_PLAY                0            //是否支持WAV提示音播放
#define WARNING_ESBC_PLAY               0            //是否支持esbc提示音播放
#define WARNING_VOLUME                  xcfg_cb.warning_volume   //播放提示音的音量级数
#define LANG_SELECT                     LANG_EN      //提示音语言选择

#define WARNING_POWER_ON                0
#define WARNING_POWER_OFF               0
#define WARNING_FUNC_MUSIC              0

#define WARNING_FUNC_CLOCK              0
#define WARNING_FUNC_FMRX               0
#define WARNING_FUNC_AUX                0
#define WARNING_FUNC_USBDEV             0
#define WARNING_FUNC_SPEAKER            0
#define WARNING_LOW_BATTERY             0
#define WARNING_USB_SD                  0
#define WARNING_MAX_VOLUME              0
#define WARNING_MIN_VOLUME              0
#define WARNING_UPDATE_DONE             0

#include "config_extra.h"

#endif // USER_CONFIG_H
