#ifndef _BSP_IR_H
#define _BSP_IR_H

#define TMR2_RCLK               3000            //xosc24m_div8 3M

///硬件IR receiver参数
#define IR32KSEL_EN         0                     //IR clock source select 32K
#if IR32KSEL_EN
    #define RPTERR_CNT      33                    //配置11.25ms ± (RPTERR_CNT + 1)*32K的repeat code允许范围
    #define DATERR_CNT      33                    //配置13.5ms ± (DATERR_CNT + 1)*32K引导码允许范围
    #define ONEERR_CNT      7                     //配置2.25ms ± (ONEERR_CNT + 1)*32K的logic "1"允许范围
    #define ZEROERR_CNT     3                     //配置1.12ms ± (ONEERR_CNT + 1)*32K数logic "0"允许范围
    #define TOPR_CNT        55                    //IR time out length = (TOPR + 1) * 64 *32K
#else
    #define RPTERR_CNT      1000                  //配置11.25ms ± (RPTERR_CNT + 1)us的repeat code允许范围
    #define DATERR_CNT      1000                  //配置13.5ms ± (DATERR_CNT + 1)us引导码允许范围
    #define ONEERR_CNT      250                   //配置2.25ms ± (ONEERR_CNT + 1)us的logic "1"允许范围
    #define ZEROERR_CNT     125                   //配置1.12ms ± (ONEERR_CNT + 1)us数logic "0"允许范围
    #define TOPR_CNT        1718                  //IR time out length = (TOPR + 1) * 64 us
#endif // IR32KSEL_EN

///红外遥控器地址码
#define IR_ADDR_FF00      0xFF00
#define IR_ADDR_BF00      0xBF00
#define IR_ADDR_FD02      0xFD02
#define IR_ADDR_FE01      0xFE01
#define IR_ADDR_7F80      0x7F80
#define IR_ADDR_DF20      0xDF20        //test

#define IR_NULL           0xffff

extern const u8 ir_tbl_FF00[96];
extern const u8 ir_tbl_BF00[32];
extern const u8 ir_tbl_FD02[32];
extern const u8 ir_tbl_FE01[32];
extern const u8 ir_tbl_7F80[32];
extern const u8 ir_tbl_DF20[96];

//IRTXCON
#define IRTX_EN_SHF         0
#define IRTX_LOADIE_EN_SHF  1
#define IRTX_FRMIE_EN_SHF   2
#define IRTX_KST_SHF        3
#define IRTX_LOAD_PND_SHF   4
#define IRTX_FRM_PND_SHF    5
#define IRTX_INVERT_SHF     6
#define IRTX_CARR_EN        7
#define IRTX_CARR_DIV_SHF   8
#define IRTX_CARR_DUTY_SHF  16
#define IRTX_BASEPR_SHF     24

enum{
    method_data = 0,
    method_level = 1
};

typedef struct{
    u32 frame_time;    //ms,帧长度
    u32 base_time;     //us,最小逻辑长度
    u32 first_bits;    //连接码前的位数
    u32 conn_bits;     //连接码，长度，单位为红外发射最小单位
    u32 second_bits;   //连接码后的位数
    u32 start_bit_time[2];  //startbit 高低电平持续时间,us
    u32 frame_bits;    //发送数据一帧的bit数
} irtx_info_t;

//选择空调品牌
enum{
    gree = 0,
    media,
    stander_nec,
    stander_nec_repeat
};

typedef struct {
    u16 cnt;                            //ir data bit counter
    u16 rpt_cnt;                        //ir repeat counter
    u16 addr;                           //address,  inverted address   Extended NEC: 16bits address
    u16 cmd;                            //command,  inverted command
    u16 RPTERR;
    u16 DATERR;
    u16 ONEERR;
    u16 ZEROERR;
    u16 TOPR;
    u32 tmrlast;
} ir_cb_t;

u32 bsp_irtx_data_encode_from_data(u8 *out, u8 *in, irtx_info_t *info);
u32 bsp_irtx_data_encode_from_levels(u8 *out, u16 *in, u32 length);
void irtx_irq_init(void);
void irrx_hw_init(void);
void irrx_irq_init(void);
void irrx_sw_init(void);
void timer1_irq_init(void);
void ir_key_clr(void);
u8 get_irkey(void);
#endif // _BSP_IR_H
