#include "include.h"

//tx部分
#if IRTX_HW_EN

//IRTX时钟选择
#define IRTX_CLK_RC2M     0
#define IRTX_CLK_X26MDIV  1
#define IRTX_CLK_RTC3M    2
#define IRTX_CLK_X26DIV8  3
#define IRTX_CLK          IRTX_CLK_X26MDIV

//不同时钟下的实际频率
#if     IRTX_CLK == IRTX_CLK_RC2M
#define IRTX_TIMING_FREQ  3450000
#elif   IRTX_CLK == IRTX_CLK_X26MDIV
#define IRTX_TIMING_FREQ  1000000
#elif   IRTX_CLK == IRTX_CLK_RTC3M
#define IRTX_TIMING_FREQ  3000000
#elif   IRTX_CLK == IRTX_CLK_X26DIV8
#define IRTX_TIMING_FREQ  400000
#else
#define IRTX_TIMING_FREQ  46000
#endif

#define IRTX_CARRIER_TARGET_FREQ    38000   //目标载波频率
//载波分频系数,计算带四舍五入更准确
#define IRTX_CARRIER_DIV            ((int)(IRTX_TIMING_FREQ / IRTX_CARRIER_TARGET_FREQ) >= (int)(IRTX_TIMING_FREQ / IRTX_CARRIER_TARGET_FREQ + 0.5f) ? \
                                    (int)(IRTX_TIMING_FREQ / IRTX_CARRIER_TARGET_FREQ) : (int)(IRTX_TIMING_FREQ / IRTX_CARRIER_TARGET_FREQ + 1))
#define IRTX_CARRIER_EXACT_FREQ     (IRTX_TIMING_FREQ / IRTX_CARRIER_DIV)   //实际载波频率
#define IRTX_CARRIER_DUTY 4

// 电平编码检测阈值
#define IRTX_LEVEL_ENCODE_THRESHOLD 160

u16 irtx_level_encode_threshold = IRTX_LEVEL_ENCODE_THRESHOLD;

static u8 buffer[50]; // 根据编码出来的bytes调节
static u8 *buffer_ptr, *buffer_end;
static volatile u8 buffer_done = 0;
u32 irtx_base_time = 0; // 计算出来分频后实际的basetime

// 小米遥控数据
u16 test_levels[]={8541,4109,543,492,621,1513,515,1559,575,462,603,1548,590,441,605,1547,534,503,594,469,605,1544,574,1553,589,472,609,1556,570,1584,597,1575,597,476,598,480,630,438,615,453,593,469,599,464,603,465,609,469,609,460,602,453,599,464,602,471,593,475,603,462,609,457,607,472,590,476,602,465,609,457,598,480,619,458,609,469,606,1564,597,480,607,468,607,469,607,1563,571,480,599,453,607,472,597,456,628,434,602,407,609,464,598,462,605,452,595,461,609,469,591,475,571,492,554,495,603,467,597,462,566,496,571,503,577,469,577,501,598,476,598,489,575,500,597,480,598,479,598,489,566,511,577,501,598,480,570,496,595,471,591,468,595,469,598,479,587,460,590,479,597,472,593,488,590,473,598,461,586,477,603,471,589,464,594,465,594,469,591,487,598,480,598,480,598,485,586,491,593,479,598,479,598,489,587,511,577,501,577,475,577,504,547,509,566,488,571,495,577,507,560,507,564,493,571,491,562,501,566,512,559,504,577,497,573,479,563,501,550,511,551,512,562,1596,534,1607,566,1595,564,512,564,522,564,511,566,0};

u8 gree_data[][9] = {
    {0x12, 0xf1, 0x56, 0x24, 0xb9, 0xf2, 0x11, 0x2e, 0x35 & 0x7},
    {0x5a, 0xf7, 0x55, 0x2b, 0xe9, 0x2d, 0x61, 0x0b, 0x87 & 0x7},
    {0x33, 0x58, 0x3b, 0x4e, 0x57, 0x9e, 0x26, 0x38, 0xdf & 0x7},
};
u8 media_data[][6] = {
    {0x12, 0xf1, 0x56, 0x24, 0xb9, 0xf2},
    {0x5a, 0xf7, 0x55, 0x2b, 0xe9, 0x2d},
    {0x33, 0x58, 0x3b, 0x4e, 0x57, 0x9e},
};
u8 stander_data[][4] = {
    {0x12, 0xf1, 0x56, 0xa9},
    {0x5a, 0xf7, 0x55, 0x2b},
    {0x33, 0x58, 0x3b, 0x4e},
};

static irtx_info_t irtx_info_lst[]={
    { 350, 480, 35, 35, 32, {9000, 4500}, 67},
    { 350, 520, 48,  0,  0, {4500, 4500}, 48},
    { 110, 562, 32,  0,  0, {9000, 4500}, 32},
    { 110, 562,  0,  0,  0, {9000, 2250},  0},
};

//NEC地址扩展
#define IRTX_ADDR_EXTENDED  0

AT(.text.bsp.ir)
void irtx_gpio_config(u8 io_num)
{
    gpio_t gpio;

    gpio_cfg_init(&gpio, io_num);
    gpio.sfr[GPIOxDE] |= BIT(gpio.num);
    gpio.sfr[GPIOxFEN] |= BIT(gpio.num);
    gpio.sfr[GPIOxDIR] &= ~BIT(gpio.num);
}

void bsp_irtx_io_init(u8 flag)
{
    if(flag == IO_PA2) {
        FUNCMCON2 = 1;
    } else if (flag == IO_PA4) {
        FUNCMCON2 = 2;
    } else if (flag == IO_PA10) {
        FUNCMCON2 = 3;
    } else if (flag == IO_PA12) {
        FUNCMCON2 = 4;
    }

    irtx_gpio_config(IRTX_MAPPING);
}

void bsp_irtx_clk_init(u8 clksel)
{
    CLKGAT1 |= 1<<4;            //irtx gate
	CLKGAT1 |= 3<<1;
    CLKGAT0 |= 1<<28 | 1<<19;
    if(clksel == 2) {
        RTCCON0 |= BIT(2);      //RTCCLK ON
    }
    CLKCON1 = (CLKCON1 & ~(BIT(12)*0x0f)) | BIT(12) * clksel;    // 0:rc2m 1:-- 2:RTC3M 3:X26DIV8
}

void bsp_irtx_dump_sfr(void)
{
    printf ("\n");
    printf ("IRTXCON0       = %08X\n", IRTXCON0);
    printf ("IRTXFRMTMRPR   = %d\n",   IRTXFRMTMRPR);
    printf ("IRTXDATA0      = %08X\n", IRTXDATA0);
}

void bsp_irtx_enable(u8 flag)
{
    if(flag) {
        IRTXCON0 |= BIT(0);
    } else {
        IRTXCON0 &= ~BIT(0);
    }
}

//一帧长度设置,毫秒单位,NEC标准为110ms
void bsp_irtx_frame_time_init(u32 ms)
{
    IRTXFRMTMRPR = ms * 1000 / irtx_base_time - 1;
}

//逻辑单位周期，纳秒单位，0的一半时间或1的四分之一时间,NEC标准为562.5us
void bsp_irtx_base_time_init(u32 us)
{
    u32 div = ((u64)IRTX_CARRIER_EXACT_FREQ * us / 1000000 -1);
    if (div > 0xff) {
        printf("base time div over range!!!\n");
        div &= 0xff;
    }
    IRTXCON0 = (IRTXCON0 & ~(BIT(24) * 0xff))
             | BIT(24) * (u8)div;
    // 计算实际的min_time，为后续提供精准计算基础
    irtx_base_time = (u32)((u64)(div + 1) * 1000000 / IRTX_CARRIER_EXACT_FREQ);
    // printf("exact base time is %dus\n", irtx_base_time);
    // 防止min_time为零，后续除法出错
    irtx_base_time = irtx_base_time ? irtx_base_time : 1;
}

void bsp_irtx_carrier_enable(u8 flag)
{
    if(flag) {
        IRTXCON0 |= BIT(7);
    } else {
        IRTXCON0 &= ~BIT(7);
    }
}

//载波设置
void bsp_irtx_carrier_init(u8 flag)
{
    IRTXCON0 = (IRTXCON0 & ~(0xffff << 8))
            | ((IRTX_CARRIER_DIV / IRTX_CARRIER_DUTY - 1) << 16)
            | (((IRTX_CARRIER_DIV) - 1) << 8);
    bsp_irtx_carrier_enable(flag);
}

//输入编码之后的数组
AT(.com_text.irq)
void bsp_irtx_data_set(u8 data)
{
    IRTXDATA0 = data;
}

void bsp_irtx_frame_ie_enable(u8 flag)
{
    if(flag) {
        IRTXCON0 |= BIT(2);
    } else {
        IRTXCON0 &= ~BIT(2);
    }
}

AT(.com_text.irq)
void bsp_irtx_load_ie_enable(u8 flag)
{
    if(flag) {
        IRTXCON0 |= BIT(1);
    } else {
        IRTXCON0 &= ~BIT(1);
    }
}

void bsp_irtx_invert(u8 flag)
{
    if(flag) {
        IRTXCON0 |= BIT(6);
    }
    else {
        IRTXCON0 &= ~BIT(6);
    }
}

//KICK
void bsp_irtx_kick_do(void)
{
    IRTXCON0 |= BIT(3);
}

void bsp_irtx_wait(void)
{
    while (!(IRTXCON0 & BIT(4)));
    IRTXCON0 |= BIT(4);
}

void bsp_irtx_wait_frame(void)
{
    while (! (IRTXCON0 & BIT(5)));
    IRTXCON0 |= BIT(5);
}

void bsp_irtx_kick(u8 data)
{
    bsp_irtx_data_set(data);
    bsp_irtx_kick_do();
}

// 配合中断使用，在中断reload buffer内的数据
void bsp_irtx_kick_buffer(u8 *data, u32 bytes)
{

    buffer_ptr = data;
    buffer_end = data + bytes;
    bsp_irtx_kick(*buffer_ptr++);
    while(!buffer_done);

    buffer_done = 0;
    bsp_irtx_load_ie_enable(1);
}

void bsp_irtx_kick_wait(u8 *buf, u32 bytes)
{
    WDT_DIS();
    u8 *ptr = buf;
    bsp_irtx_kick (*ptr++);
    while (--bytes) {
        bsp_irtx_wait();
        bsp_irtx_data_set(*ptr++);
    }
    bsp_irtx_wait();
    bsp_irtx_data_set(0);
    bsp_irtx_wait_frame();
}

AT(.com_text.irq)
void irtx_isr(void)
{
	if(IRTXCON0 & BIT(5)) {
	    IRTXCON0 |= BIT(5);
        buffer_done = 1;
	}
    if(IRTXCON0 & BIT(4)) {
	    IRTXCON0 |= BIT(4);
        if(buffer_ptr < buffer_end){
            bsp_irtx_data_set(*buffer_ptr++);
        }else{
            bsp_irtx_data_set(0);
            bsp_irtx_load_ie_enable(0);
        }
	}
}
////////////////////////////////////////////////////////////////////////////////////

void bsp_irtx_nec_init(irtx_info_t *info)
{
    bsp_irtx_carrier_init(1);
    bsp_irtx_invert(1);
    bsp_irtx_base_time_init(info->base_time);
    bsp_irtx_frame_time_init(info->frame_time);
    delay_5ms(60);  //等同步完再使能
    bsp_irtx_enable(1);
}

void bsp_irtx_single(void)
{
    my_printf("%s\n",__func__);
    bsp_irtx_clk_init(IRTX_CLK);
    bsp_irtx_io_init(IRTX_MAPPING);
    bsp_irtx_nec_init(&irtx_info_lst[stander_nec]);
    bsp_irtx_dump_sfr();
    // 编码
    u32 bytes = bsp_irtx_data_encode_from_data(buffer, stander_data[1], &irtx_info_lst[stander_nec]);
    // u32 bytes = bsp_irtx_data_encode_from_data(buffer, &gree_data[0], &irtx_info_lst[gree]);
    // u32 bytes = bsp_irtx_data_encode_from_data(buffer, &media_data[0], &irtx_info_lst[media]);
    // u32 bytes = bsp_irtx_data_encode_from_levels(buffer, test_levels, sizeof(test_levels) / sizeof(u16));

    bsp_irtx_kick_wait(buffer, bytes);
}


void bsp_irtx_singal_ie(void)
{
    my_printf("%s\n",__func__);
    bsp_irtx_clk_init (IRTX_CLK);
    bsp_irtx_io_init(IRTX_MAPPING);
    irtx_irq_init();
    bsp_irtx_nec_init(&irtx_info_lst[stander_nec]);
    bsp_irtx_load_ie_enable(1);
    bsp_irtx_frame_ie_enable(1);
    bsp_irtx_dump_sfr();
    // bsp_irtx_dump_sfr();

    // 编码
     u32 bytes = bsp_irtx_data_encode_from_data(buffer, stander_data[0], &irtx_info_lst[stander_nec]);
    // u32 bytes = bsp_irtx_data_encode_from_data(buffer, gree_data[0], &irtx_info_lst[gree]);
    // u32 bytes = bsp_irtx_data_encode_from_data(buffer, media_data[0], &irtx_info_lst[media]);
    // u32 bytes = bsp_irtx_data_encode_from_levels(buffer, test_levels, sizeof(test_levels) / sizeof(uint16_t));

    bsp_irtx_kick_buffer(buffer, bytes);
}

void bsp_irtx_init(u8 method)
{
    if(method_data) {
        bsp_irtx_single();
    }
    else if(method_level) {
        bsp_irtx_singal_ie();
    }
}
////////////////////////////////////////////////////////////////////////////////
#endif // IRTX_HW_EN

//rx部分
#if IRRX_SW_EN
ir_cb_t ir_cb AT(.buf.ir.cb);

AT(.com_text.str)
const char ir_str[] = "get_irkey: %04x, %04x %x\n";

AT(.com_text.ir)
u8 get_irkey(void)
{
    u8 key_val = NO_KEY;
    if (ir_cb.cnt != 32) {
        return NO_KEY;
    }

    if (ir_cb.addr == IR_NULL && ir_cb.cmd == IR_NULL)
        return 0xff;

#if IR_ADDR_DF20_EN
    if (ir_cb.addr == IR_ADDR_DF20) {
        key_val = ir_tbl_DF20[ir_cb.cmd & 0xff];
    }
#endif // IR_ADDR_FF00_EN

#if IR_ADDR_FF00_EN
    if (ir_cb.addr == IR_ADDR_FF00) {
        key_val = ir_tbl_FF00[ir_cb.cmd & 0xff];
    }
#endif // IR_ADDR_FF00_EN

#if IR_ADDR_BF00_EN
    if (ir_cb.addr == IR_ADDR_BF00) {
        key_val = ir_tbl_BF00[ir_cb.cmd & 0xff];
    }
#endif // IR_ADDR_BF00_EN

#if IR_ADDR_FD02_EN
    if (ir_cb.addr == IR_ADDR_FD02) {
        key_val = ir_tbl_FD02[ir_cb.cmd & 0xff];
    }
#endif // IR_ADDR_FD02_EN

#if IR_ADDR_FE01_EN
    if (ir_cb.addr == IR_ADDR_FE01) {
        key_val = ir_tbl_FE01[ir_cb.cmd & 0xff];
    }
#endif // IR_ADDR_FE01_EN

#if IR_ADDR_7F80_EN
    if (ir_cb.addr == IR_ADDR_7F80) {
        key_val = ir_tbl_7F80[ir_cb.cmd & 0xff];
    }
#endif

    my_printf(ir_str, ir_cb.cmd, ir_cb.addr, key_val);
    return key_val;
}

#if IRRX_SW_TMR_SWITCH
AT(.com_text.isr.timer)
void software_ir_encode(u32 tmrcnt)
{
    if (ir_cb.cnt == 32) {
        TMR2CNT = 0;
    if ((tmrcnt >= 10) && (tmrcnt <= 12)) {
        //repeat code is simply 9ms+2.25ms
        ir_cb.rpt_cnt = 0;
    } else {
        ir_cb.rpt_cnt += tmrcnt;

        if (ir_cb.rpt_cnt > 108) {
            ir_cb.rpt_cnt = 0;
            ir_cb.cnt = 0;          //ir key release
        }
    }
    return;
    } else if ((tmrcnt > 7) || (tmrcnt == 0)) {     //A message is started by a 9ms AGC burst
        ir_cb.rpt_cnt = 0;
        ir_cb.cnt = 0;                  //ir key message started
        return;
    }
    ir_cb.cmd >>= 1;
    ir_cb.cnt++;
    if (tmrcnt == 2) {                  //Bit time of 1.125ms(0) or 2.25ms(1)
        ir_cb.cmd |= 0x8000;
    }

    if (ir_cb.cnt == 16) {
        ir_cb.addr = ir_cb.cmd;         //save address data
    } else if (ir_cb.cnt == 32) {
        //got ir key message
        if ((u8)ir_cb.cmd > 96) {
            ir_cb.cmd = NO_KEY;
        }
         //my_printf(ir_str, ir_cb.addr, (u8)ir_cb.cmd);
    }
}

AT(.com_text.isr.timer)
void timer2_isr(void)
{
    u32 tmrcnt;
    if ((ir_cb.cnt > 0)&&(ir_cb.cnt < 32)) {
        TMR2CPND = BIT(9);
        return;
    } else if (TMR2CON & BIT(9)) {
        TMR2CPND = BIT(9);
        tmrcnt = 110;                   //110ms overflow
    } else {
        return;
    }
    software_ir_encode(tmrcnt);
}

//timer1 capture
AT(.com_text.isr.timer)
void timer1_isr(void)
{
    u32 tmrcnt;
    u32 tmrpr;
    if(TMR1CON & BIT(10)) {
        //timer1 capture interrupt
        if (!ir_cb.cnt){
            TMR2CNT = 0;
        }
        tmrpr = TMR1CPT;
        if (ir_cb.tmrlast < tmrpr) {
            tmrcnt = tmrpr - ir_cb.tmrlast;
        } else {
            tmrcnt = (0xffff - ir_cb.tmrlast) + tmrpr;
        }
        ir_cb.tmrlast = tmrpr;
        TMR1CPND = BIT(10);
        tmrcnt /= TMR2_RCLK;                //convert to ms
    } else {
        return;
    }
    software_ir_encode(tmrcnt);
}

AT(.text.bsp.ir)
void ir_key_clr(void)
{
    ir_cb.cnt = 0;
}

AT(.text.bsp.ir)
void timer2_init(void)
{
    sys_irq_init(IRQ_TMR2_VECTOR, 0, timer2_isr);
    TMR2CON = 0;
	TMR2CNT = 0;
	TMR2PR  = TMR2_RCLK*110 - 1;                            //110ms Timer overflow interrupt
    TMR2CON = (2 << 1) | (3 << 4) | BIT(7) | BIT(0);        //timer2 clk = xosc24m / 8
    PICPR &= ~BIT(IRQ_TMR2_VECTOR);
	PICEN |= BIT(IRQ_TMR2_VECTOR);
}

AT(.text.bsp.ir)
void timer1_init(void)
{
    sys_irq_init(IRQ_TMR1_VECTOR, 0, timer1_isr);
    TMR1CON = 0;
	TMR1CNT = 0;
    TMR1CON = (2 << 1) | (3 << 4) | (2 << 14) | BIT(8) | BIT(0) ;
    PICPR &= ~BIT(IRQ_TMR1_VECTOR);
	PICEN |= BIT(IRQ_TMR1_VECTOR);
}

AT(.text.bsp.ir)
void irrx_sw_init(void)
{
    my_printf("TMR1&2 %s\n",__func__);  //同时使用TMR1和TMR2实现
    IR_CAPTURE_PORT();
    FUNCMCON1 = (2 << 16);              //enable timer1 map
    FUNCINCON = (IRRX_MAPPING - 1);

    memset(&ir_cb, 0, sizeof(ir_cb));
    timer1_init();
    timer2_init();
}
#else
AT(.com_text.isr.timer)
void timer2_isr(void)
{
    u32 tmrcnt;

    if (TMR2CON & BIT(10)) {
        //timer1 capture interrupt
        TMR2CNT  = TMR2CNT - TMR2CPT;
        tmrcnt = TMR2CPT;
        TMR2CPND = BIT(10);
        tmrcnt = tmrcnt /TMR2_RCLK;                //convert to ms
    } else if (TMR2CON & BIT(9)){
        //timer1 overflow interrupt
        TMR2CPND = BIT(9);
        tmrcnt = 110;                   //110ms overflow
    } else {
        return;
    }

    //processing repeat code
    if (ir_cb.cnt == 32) {
        if ((tmrcnt >= 10) && (tmrcnt <= 12)) {
            //repeat code is simply 9ms+2.25ms
            ir_cb.rpt_cnt = 0;
        } else {
            ir_cb.rpt_cnt += tmrcnt;
            if (ir_cb.rpt_cnt > 108) {
                ir_cb.rpt_cnt = 0;
                ir_cb.cnt = 0;          //ir key release
            }
        }
        return;
    } else if ((tmrcnt > 7) || (tmrcnt == 0)) {     //A message is started by a 9ms AGC burst
        ir_cb.rpt_cnt = 0;
        ir_cb.cnt = 0;                  //ir key message started
        return;
    }

    ir_cb.cmd >>= 1;
    ir_cb.cnt++;
    if (tmrcnt == 2) {                  //Bit time of 1.125ms(0) or 2.25ms(1)
        ir_cb.cmd |= 0x8000;
    }

    if (ir_cb.cnt == 16) {
        ir_cb.addr = ir_cb.cmd;         //save address data
    } else if (ir_cb.cnt == 32) {
        //got ir key message
        if ((u8)ir_cb.cmd > 96) {
            ir_cb.cmd = NO_KEY;
        }
//        printf("ir: %04x, %02x\n",ir_cb.addr, (u8)ir_cb.cmd);
    }
}

AT(.text.bsp.ir)
void ir_key_clr(void)
{
    ir_cb.cnt = 0;
}

AT(.text.bsp.ir)
void timer2_init(void)
{
    sys_irq_init(IRQ_TMR2_VECTOR, 0, timer2_isr);
    TMR2CON = 0;
	TMR2CNT = 0;
	TMR2PR  = TMR2_RCLK*110 - 1;                            //110ms Timer overflow interrupt
    TMR2CON = (2 << 1) | (3 << 4) | (2 << 14) | BIT(8) | BIT(7) |  BIT(0) ;       //timer2 clk = xosc24m / 8
    PICPR &= ~BIT(IRQ_TMR2_VECTOR);
	PICEN |= BIT(IRQ_TMR2_VECTOR);
}

AT(.text.bsp.ir)
void irrx_sw_init(void)
{
    my_printf("TMR2 %s\n",__func__);
    IR_CAPTURE_PORT();
    FUNCMCON1 = (2 << 16);              //enable timer1 map
    FUNCINCON = (IRRX_MAPPING - 1);
    memset(&ir_cb, 0, sizeof(ir_cb));
    timer2_init();
}
#endif // IRRX_SW_TMR_SWITCH

#endif // IRRX_SW_EN


