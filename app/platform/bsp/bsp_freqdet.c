#include "include.h"
#include "bsp_freqdet.h"

#if FREQ_DET_EN

#define FREQDET_MINERR_EN  0
#define FREQDET_CLKIN_SEL CLKOUT_PIN

AT(.com_text.rodata)
const char str_freq[] = "FREQDETCNT : %d\n";
AT(.com_text.rodata)
const char str_freq_err[] = "freq_det_err : %d\n";
AT(.com_text.rodata)
const char str_freq_clk[] = "freqdet  result is : %d\n";

u8 rc2m_cnt = 0;
u32 rc2m_freq_cal = 0;

AT(.com_text.irq)
void freqdet_isr(void)
{
    rc2m_cnt++;
    if(FREQDETCON & BIT(15)) {
        FREQDETCPND = BIT(15);
        //my_printf(str_freq,FREQDETCNT);                  //单位是k
    #if FREQDET_MINERR_EN
        //my_printf(str_freq_err,FREQDETTGT>>16);
    #endif // FREQDET_MINERR_EN
        if(rc2m_cnt <= 9) {
            rc2m_freq_cal = rc2m_freq_cal + FREQDETCNT;
        }
        if(rc2m_cnt == 9) {
            rc2m_freq_cal = rc2m_freq_cal / 9;
            my_printf(str_freq_clk,rc2m_freq_cal);
            FREQDETCON &= ~BIT(0);                                       //10次后中断截止
        }
    }
}

void freqdet_ie_enable(u8 flag)
{
    if(flag) {
        FREQDETCON |= BIT(1);                 //开中断bit
        freqdet_irq_init();                   //初始化freqdet中断
    } else {
        FREQDETCON &= ~BIT(1);
    }
}

void freqdet_init(void)
{
    my_printf("%s\n",__func__);

    CLKGAT1 |= BIT(26);
    PLL0CON |= BIT(0) | BIT(12);                                      //pll时钟初始化 LDO
    FREQDETCON = XOSC24M_FREQ << 16;                                  //模块时钟的频率
    RTCCON3 &= ~BIT(6);
    RTCCON0 |= BIT(0) | BIT(2);                                       //使能RC时钟

#if FREQDET_MINERR_EN
    FREQDETTGT = FREQDET_MINERR_VAL;                                  //与所测频率进行比较，在中断中打印差值
#endif // FREQDET_MINERR_EN

#if (FREQDET_CLKIN_SEL == CLKOUT_PIN)
    CLKCON0 = (CLKCON0 & ~(0xf<<13)) | (0x7<<13);                     //选择需要测量的时钟  可参照时钟图CLKCON0[16:13]，此处检测rc2m  0x7：RC2M  0x9:sys_clk
    CLKDIVCON0  = (CLKDIVCON0 & ~(0x1f<<17)) | ((0x0)<<17);           //时钟分频 N-1分频
#endif

    CLKCON0 = (CLKCON0 & ~ (3<<22)) | (0x2<<22);                      //模块的时钟

    CLKCON0 = (CLKCON0 & ~(3<<20)) | (FREQDET_CLKIN_SEL <<20);        //需要捕获的时钟

    freqdet_ie_enable(1);                                             //是否使能中断

    FREQDETCON |= BIT(0);                                             //Freqdet en
}
#endif // FREQ_DET_EN
