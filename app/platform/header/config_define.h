/*****************************************************************************
 * Module    : Configs
 * File      : config_define.h
 * Function  : 定义用户参数常量
 *****************************************************************************/
#ifndef CONFIG_DEFINE_H
#define CONFIG_DEFINE_H

/*****************************************************************************
 * Module    : 显示相关配置选择列表
 *****************************************************************************/
//显示驱动屏选择
#define DISPLAY_NO                      0                                       //无显示模块
#define DISPLAY_LCD                     0x100                                   //选用LCD点阵屏做为显示驱动
#define DISPLAY_LCDSEG                  0x200                                   //选用断码屏做为显示驱动
#define DISPLAY_LEDSEG                  0x400                                   //选用数码管做为显示驱动

#define GUI_NO                          DISPLAY_NO                              //无主题，无显示
#define GUI_LCD                         (DISPLAY_LCD | 0x00)                    //LCD点阵屏(128*64)
#define GUI_LCDSEG                      (DISPLAY_LCDSEG | 0x00)                 //断码屏默认主题
#define GUI_LEDSEG_5C7S                 (DISPLAY_LEDSEG | 0x00)                 //5C7S 数码管
#define GUI_LEDSEG_7P7S                 (DISPLAY_LEDSEG | 0x01)                 //7PIN 数码管 按COM方式扫描
#define GUI_LEDSEG_3P7S                 (DISPLAY_LEDSEG | 0x02)                 //3PIN 7段数码管
#define GUI_LEDSEG_6C6S                 (DISPLAY_LEDSEG | 0x04)                 //6C6S 6段数码管

/*****************************************************************************
 * Module    : FLASH大小定义
 *****************************************************************************/
#define FSIZE_32M                       0x2000000   //MAX flash size
#define FSIZE_16M                       0x1000000
#define FSIZE_5M                        0x500000
#define FSIZE_4M                        0x400000
#define FSIZE_3M                        0x300000
#define FSIZE_2M                        0x200000
#define FSIZE_1M                        0x100000
#define FSIZE_512K                      0x80000
#define FSIZE_256K                      0x40000
#define FSIZE_128K                      0x20000

/*****************************************************************************
 * Module    : 提示音语言配置选择列表
 *****************************************************************************/
#define LANG_EN         0               //英文提示音
#define LANG_ZH         1               //中文提示音
#define LANG_EN_ZH      2               //英文、中文双语提示音

/*****************************************************************************
 * Module    : ADC通路选择列表
 *****************************************************************************/
#define ADCCH_PA0       0               //SARADC channel 0
#define ADCCH_PA1       1               //SARADC channel 1
#define ADCCH_PA2       2               //SARADC channel 2
#define ADCCH_PA3       3               //SARADC channel 3
#define ADCCH_PA4       4               //SARADC channel 4
#define ADCCH_PA5       5               //SARADC channel 5
#define ADCCH_PA6       6               //SARADC channel 6
#define ADCCH_PA7       7               //SARADC channel 7
#define ADCCH_PA8       8               //SARADC channel 8
#define ADCCH_PA9       9               //SARADC channel 9
#define ADCCH_PA10      10              //SARADC channel 10
#define ADCCH_PA11      11              //SARADC channel 11
#define ADCCH_PG4       12              //SARADC channel 12
#define ADCCH_PG5       13              //SARADC channel 13
#define ADCCH_ANA       14              //SARADC channel 14     BG/Tsensor/Vcore
#define ADCCH_VBAT      15              //SARADC channel 15     VBATDIV2


/*****************************************************************************
 * Module    : Timer Capture Mapping选择列表
 *****************************************************************************/
#define TMR0MAP_PA9     (1 << 0)        //TMR0CAP G1  capture mapping: PA9
#define TMR1MAP_PA2     (1 << 0)        //TMR1CAP G1  capture mapping: PA2
#define TMR2MAP_PA7     (1 << 0)        //TMR2CAP G1  capture mapping: PA7


/*****************************************************************************
 * Module    : IRRX Mapping选择列表
 *****************************************************************************/
#define IRMAP_PA2       (1 << 0)        //G1  capture mapping: PA2
#define IRMAP_PA4       (2 << 0)        //G2  capture mapping: PA4
#define IRMAP_PA10      (3 << 0)        //G3  capture mapping: PA10
#define IRMAP_PA12      (4 << 0)        //G4  capture mapping: PA12


/*****************************************************************************
 * Module    : Clock output Mapping选择列表
 *****************************************************************************/
#define CLKOMAP_PA2     (1 << 0)        //G1 Clock output mapping: PA2
#define CLKOMAP_PA9     (2 << 0)        //G2 Clock output mapping: PA9
#define CLKOMAP_PA13    (3 << 0)        //G3 Clock output mapping: PA13


/*****************************************************************************
 * Module    : sd0 Mapping选择列表
 *****************************************************************************/
#define SD0MAP_G1       (1 << 0)       //G1  SDCLK(PA3), SDCMD(PA4), SDDAT0(PA5)
#define SD0MAP_G2       (2 << 0)       //G2  SDCLK(PA8), SDCMD(PA9), SDDAT0(PA10)


/*****************************************************************************
 * Module    : spi1 Mapping选择列表 (FUNCMCON1)
 *****************************************************************************/
#define SPI1MAP_G1     (1 << 12)        //G1 SPI1CLK(PA3),  SPI1DI(PA5),  SPI1DO(PA4)
#define SPI1MAP_G2     (2 << 12)        //G2 SPI1CLK(PA8),  SPI1DI(PA10), SPI1DO(PA9)
#define SPI1MAP_G3     (3 << 12)        //G3 SPI1CLK(PA12), SPI1DI(PA14), SPI1DO(PA13)
#define SPI1MAP_G4     (4 << 12)        //G4 SPI1CLK(PA1),  SPI1DI(PA3),  SPI1DO(PA2)


/*****************************************************************************
 * Module    : uart0 Mapping选择列表
 *****************************************************************************/
#define UTX0MAP_PA3     (1 << 8)        //G1 uart0 tx: PA3
#define UTX0MAP_PA8     (2 << 8)        //G2 uart0 tx: PA8
#define UTX0MAP_PA2     (3 << 8)        //G3 uart0 tx: PA2
#define UTX0MAP_PA12    (4 << 8)        //G4 uart0 tx: PA12
#define UTX0MAP_PA14    (5 << 8)        //G5 uart0 tx: PA14


#define URX0MAP_PA4     (1 << 12)       //G1 uart0 rx: PA4
#define URX0MAP_PA9     (2 << 12)       //G2 uart0 rx: PA9
#define URX0MAP_PA1     (3 << 12)       //G3 uart0 rx: PA1
#define URX0MAP_PA13    (4 << 12)       //G4 uart0 rx: PA13
#define URX0MAP_PA15    (5 << 12)       //G4 uart0 rx: PA15
#define URX0MAP_TX      (7 << 12)       //G7 uart0 map to TX pin by UT0TXMAP select(1线模式)


/*****************************************************************************
* Module    : hsuart select table
*****************************************************************************/
#define HSTX_PA5        (1 << 16)       //HSTRX G1: PA5
#define HSTX_PA6        (2 << 16)       //HSTRX G2: PA6 
#define HSTX_PA2        (3 << 16)       //HSTRX G3: PA2
#define HSTX_PA10       (4 << 16)       //HSTRX G4: PA10 
#define HSTX_PA11       (5 << 16)       //HSTRX G5: PA11

#define HSRX_PA5        (1 << 20)       //HSTRX G1: PA5
#define HSRX_PA6        (2 << 20)       //HSTRX G2: PA6 
#define HSRX_PA2        (3 << 20)       //HSTRX G3: PA2
#define HSRX_PA10       (4 << 20)       //HSTRX G4: PA10 
#define HSRX_PA11       (5 << 20)       //HSTRX G5: PA11

/*****************************************************************************
 * Module    : 录音文件类型列表
 *****************************************************************************/
#define REC_NO          0
#define REC_WAV         1              //PCM WAV
#define REC_ADPCM       2              //ADPCM WAV
#define REC_MP3         3


/*****************************************************************************
* Module    : TouchKey通路
*****************************************************************************/
#define TK_PA0_CH0          0
#define TK_PA1_CH1          1
#define TK_PA2_CH2          2
#define TK_PA3_CH3          3
#define TK_PA4_CH4          4
#define TK_PA5_CH5          5
#define TK_PA6_CH6          6
#define TK_PA7_CH7          7
#define TK_PA8_CH8          8
#define TK_PA9_CH9          9
#define TK_PA10_CH10        10
#define TK_PA11_CH11        11

/*****************************************************************************
* Module    : DAC SELECT
*****************************************************************************/
#define DAC_MONO        0              //DAC单声道输出
#define DAC_DUAL        1              //DAC双声道输出
#define DAC_VCMBUF_MONO 2              //DAC VCMBUF单声道输出
#define DAC_VCMBUF_DUAL 3              //DAC VCMBUF双声道输出
#define DAC_MIX_MONO2   4              //DAC_R MIX to VCMBUF(省电容)
#define DAC_MIX_MONO_INVERT     5      //DACL /DACR 反推喇叭

/*****************************************************************************
* Module    : DAC OUT Sample Rate
*****************************************************************************/
#define DAC_OUT_44K1    0               //dac out sample rate 44.1K
#define DAC_OUT_48K     1               //dac out sample rate 48K

/*****************************************************************************
* Module    : DAC LDOH Select
*****************************************************************************/
#define AU_LDOH_2V4     0               //VDDAUD LDO voltage 2.4V
#define AU_LDOH_2V5     1               //VDDAUD LDO voltage 2.5V
#define AU_LDOH_2V7     2               //VDDAUD LDO voltage 2.7V
#define AU_LDOH_2V9     3               //VDDAUD LDO voltage 2.9V
#define AU_LDOH_3V1     4               //VDDAUD LDO voltage 3.1V
#define AU_LDOH_3V2     5               //VDDAUD LDO voltage 3.2V

/*****************************************************************************
* Module    : 电池低电电压列表
*****************************************************************************/
#define VBAT_2V8            0       //2.8v
#define VBAT_2V9            1       //2.9v
#define VBAT_3V0            2       //3.0v
#define VBAT_3V1            3       //3.1v
#define VBAT_3V2            4       //3.2v
#define VBAT_3V3            5       //3.3v
#define VBAT_3V4            6       //3.4v
#define VBAT_3V5            7       //3.5v
#define VBAT_3V6            8       //3.6v
#define VBAT_3V7            9       //3.7v
#define VBAT_3V8            10      //3.8v


/*****************************************************************************
* Module    : AUX or MIC Left&Right channel list
* AUX: 可以任意左与右搭配，或只选择左，或只选择右
*****************************************************************************/
#define CH_MIC_PA0         0x01    //MIC0(PA0)  -> mic  -> sdadc channel


/*****************************************************************************
* Module    : uart0 printf IO列表
*****************************************************************************/
#define PRINTF_NONE         0           //关闭UART0打印信息
#define PRINTF_PA3          1
#define PRINTF_PA8          2
#define PRINTF_PA2          3
#define PRINTF_PA12         4
#define PRINTF_PA14         5


/*****************************************************************************
* Module    : GPIO list
*****************************************************************************/
#define IO_NONE             0
#define IO_PA0              1
#define IO_PA1              2
#define IO_PA2              3
#define IO_PA3              4
#define IO_PA4              5
#define IO_PA5              6
#define IO_PA6              7
#define IO_PA7              8
#define IO_PA8              9
#define IO_PA9              10
#define IO_PA10             11
#define IO_PA11             12
#define IO_PA12             13
#define IO_PA13             14
#define IO_PA14             15
#define IO_PA15             16
#define IO_MAX_NUM          16

#define IO_MUX_SDCLK        26
#define IO_MUX_SDCMD        27

/*****************************************************************************
* Module    : wakeup circuit port
*****************************************************************************/
#define WKU_PA1             0
#define WKU_PA2             1
#define WKU_PA4             2
#define WKU_PA9             3
#define WKU_PA10            4
#define WKU_PA13            5

/*****************************************************************************
* Module    : Function output select table
*****************************************************************************/
#define FUNCO_TMR2PWM0      (0 << 0)
#define FUNCO_TMR2PWM1      (1 << 0)
#define FUNCO_TMR2PWM2      (2 << 0)
#define FUNCO_TMR2PWM3      (3 << 0)
#define FUNCO_TMR2PWM4      (4 << 0)
#define FUNCO_LEDC_OUT      (5 << 0)
#define FUNCO_UARTOTX       (6 << 0)
#define FUNCO_HSUTTX        (7 << 0)
#define FUNCO_IIC_SDA       (8 << 0)
#define FUNCO_IIC_SCL       (9 << 0)
#define FUNCO_SPI1_DO       (10 << 0)
#define FUNCO_SPI1_DI       (11 << 0)
#define FUNCO_SPI1_CLK      (12 << 0)
#define FUNCO_DAC_PWM_P     (13 << 0)
#define FUNCO_DAC_PWM_N     (14 << 0)
#define FUNCO_CLKOUT        (15 << 0)

/*****************************************************************************
* Module    : EX_SPIFLASH_SUPPORT
*****************************************************************************/
#define EXSPI_NOT_SUPPORT   0
#define EXSPI_REC           (1<<0)    //外接SPIFLASH支持录音
#define EXSPI_MUSIC         (1<<1)    //外接SPIFLASH支持MP3音乐镜像文件播放

/*****************************************************************************
 * Module    : I2C Mapping选择列表 FUNCMCON2[27:24]
 *****************************************************************************/
#define I2CMAP_PA3PA4       (1 << 24)        //G1 i2c clk: PA3  dat: PA4
#define I2CMAP_PA8PA9       (2 << 24)        //G2 i2c clk: PA8  dat: PA9
#define I2CMAP_PA12PA13     (3 << 24)        //G3 i2c clk: PA12 dat: PA13
#define I2CMAP_PA1PA2       (4 << 24)        //G4 i2c clk: PA1  dat: PA2

/*****************************************************************************
* Module    : mic数字增益调节表
*****************************************************************************/
#define MIC_0DB_VAL         1024
#define MIC_MAX_VAL         65535

#define MIC_DIG_P36DB       64610   //(MIC_0DB_VAL * 63.09573)
#define MIC_DIG_P35DB       57584   //(MIC_0DB_VAL * 56.23413)
#define MIC_DIG_P34DB       51322   //(MIC_0DB_VAL * 50.11872)
#define MIC_DIG_P33DB       45740   //(MIC_0DB_VAL * 44.66835)
#define MIC_DIG_P32DB       40766   //(MIC_0DB_VAL * 39.81071)
#define MIC_DIG_P31DB       36333   //(MIC_0DB_VAL * 35.48133)
#define MIC_DIG_P30DB       32382   //(MIC_0DB_VAL * 31.62277)
#define MIC_DIG_P29DB       28860   //(MIC_0DB_VAL * 28.18382)
#define MIC_DIG_P28DB       25722   //(MIC_0DB_VAL * 25.11886)
#define MIC_DIG_P27DB       22925   //(MIC_0DB_VAL * 22.38721)
#define MIC_DIG_P26DB       20431   //(MIC_0DB_VAL * 19.95262)
#define MIC_DIG_P25DB       18210   //(MIC_0DB_VAL * 17.78279)
#define MIC_DIG_P24DB       16229   //(MIC_0DB_VAL * 15.84893)
#define MIC_DIG_P23DB       14464   //(MIC_0DB_VAL * 14.12537)
#define MIC_DIG_P22DB       12891   //(MIC_0DB_VAL * 12.58925)
#define MIC_DIG_P21DB       11489   //(MIC_0DB_VAL * 11.22018)
#define MIC_DIG_P20DB       10240   //(MIC_0DB_VAL * 10)

#define MIC_DIG_P19DB       9126    //(MIC_0DB_VAL * 8.912509)
#define MIC_DIG_P18DB       8134    //(MIC_0DB_VAL * 7.943282)
#define MIC_DIG_P17DB       7249    //(MIC_0DB_VAL * 7.079458)
#define MIC_DIG_P16DB       6461    //(MIC_0DB_VAL * 6.309574)
#define MIC_DIG_P15DB       5758    //(MIC_0DB_VAL * 5.623413)
#define MIC_DIG_P14DB       5132    //(MIC_0DB_VAL * 5.011872)
#define MIC_DIG_P13DB       4574    //(MIC_0DB_VAL * 4.466835)
#define MIC_DIG_P12DB       4077    //(MIC_0DB_VAL * 3.981072)
#define MIC_DIG_P11DB       3633    //(MIC_0DB_VAL * 3.548134)
#define MIC_DIG_P10DB       3238    //(MIC_0DB_VAL * 3.162278)
#define MIC_DIG_P9DB        2886    //(MIC_0DB_VAL * 2.818383)
#define MIC_DIG_P8DB        2572    //(MIC_0DB_VAL * 2.511886)
#define MIC_DIG_P7DB        2292    //(MIC_0DB_VAL * 2.238721)
#define MIC_DIG_P6DB        2043    //(MIC_0DB_VAL * 1.995262)
#define MIC_DIG_P5DB        1821    //(MIC_0DB_VAL * 1.778279)
#define MIC_DIG_P4DB        1623    //(MIC_0DB_VAL * 1.584893)
#define MIC_DIG_P3DB        1446    //(MIC_0DB_VAL * 1.412538)
#define MIC_DIG_P2DB        1289    //(MIC_0DB_VAL * 1.258925)
#define MIC_DIG_P1DB        1148    //(MIC_0DB_VAL * 1.122018)

#define MIC_DIG_N0DB        1024    //(MIC_0DB_VAL * 1.000000)  //0db
#define MIC_DIG_N1DB        912     //(MIC_0DB_VAL * 0.891251)
#define MIC_DIG_N2DB        813     //(MIC_0DB_VAL * 0.794328)
#define MIC_DIG_N3DB        724     //(MIC_0DB_VAL * 0.707946)
#define MIC_DIG_N4DB        646     //(MIC_0DB_VAL * 0.630957)
#define MIC_DIG_N5DB        575     //(MIC_0DB_VAL * 0.562341)
#define MIC_DIG_N6DB        513     //(MIC_0DB_VAL * 0.501187)
#define MIC_DIG_N7DB        457     //(MIC_0DB_VAL * 0.446684)
#define MIC_DIG_N8DB        407     //(MIC_0DB_VAL * 0.398107)
#define MIC_DIG_N9DB        363     //(MIC_0DB_VAL * 0.354813)
#define MIC_DIG_N10DB       324     //(MIC_0DB_VAL * 0.316228)      
#define MIC_DIG_N11DB       289     //(MIC_0DB_VAL * 0.281838)
#define MIC_DIG_N12DB       257     //(MIC_0DB_VAL * 0.251189)
#define MIC_DIG_N13DB       229     //(MIC_0DB_VAL * 0.223872)
#define MIC_DIG_N14DB       204     //(MIC_0DB_VAL * 0.199526)
#define MIC_DIG_N15DB       182     //(MIC_0DB_VAL * 0.177828)
#define MIC_DIG_N16DB       162     //(MIC_0DB_VAL * 0.158489)
#define MIC_DIG_N17DB       145     //(MIC_0DB_VAL * 0.141254)
#define MIC_DIG_N18DB       129     //(MIC_0DB_VAL * 0.125893)
#define MIC_DIG_N19DB       115     //(MIC_0DB_VAL * 0.112202)
#define MIC_DIG_N20DB       102     //(MIC_0DB_VAL * 0.100000)

#define MIC_DIG_N21DB       91      //(MIC_0DB_VAL * 0.089125)
#define MIC_DIG_N22DB       81      //(MIC_0DB_VAL * 0.079433)
#define MIC_DIG_N23DB       72      //(MIC_0DB_VAL * 0.070795)
#define MIC_DIG_N24DB       65      //(MIC_0DB_VAL * 0.063096)
#define MIC_DIG_N25DB       58      //(MIC_0DB_VAL * 0.056234)
#define MIC_DIG_N26DB       51      //(MIC_0DB_VAL * 0.050119)
#define MIC_DIG_N27DB       46      //(MIC_0DB_VAL * 0.044668)
#define MIC_DIG_N28DB       41      //(MIC_0DB_VAL * 0.039811)
#define MIC_DIG_N29DB       36      //(MIC_0DB_VAL * 0.035481)
#define MIC_DIG_N30DB       32      //(MIC_0DB_VAL * 0.031623)
#define MIC_DIG_N31DB       29      //(MIC_0DB_VAL * 0.028184)
#define MIC_DIG_N32DB       26      //(MIC_0DB_VAL * 0.025119)
#define MIC_DIG_N33DB       23      //(MIC_0DB_VAL * 0.022387)
#define MIC_DIG_N34DB       20      //(MIC_0DB_VAL * 0.019953)
#define MIC_DIG_N35DB       18      //(MIC_0DB_VAL * 0.017783)
#define MIC_DIG_N36DB       16      //(MIC_0DB_VAL * 0.015849)
#define MIC_DIG_N37DB       14      //(MIC_0DB_VAL * 0.014125)
#define MIC_DIG_N38DB       13      //(MIC_0DB_VAL * 0.012589)
#define MIC_DIG_N39DB       11      //(MIC_0DB_VAL * 0.011220)

/*****************************************************************************
 * Module    : 语音识别库类型
 *****************************************************************************/
#define ASR_ZH              0               //中文库
#define ASR_EN              1               //英文库

#endif //CONFIG_DEFINE_H
