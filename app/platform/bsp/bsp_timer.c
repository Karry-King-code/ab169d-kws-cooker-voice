#include "include.h"

#if TMR2_US_EN

#define US_1S_TEST      0   //测试10us定时,累计到达1s的时间,通过download检查打印时间
#define US_IO_TEST      0   //PA9翻转IO测试us定时

#if US_1S_TEST
u32 us10_tick;
AT(.com_rodata.isr)
const char str_t3[] = "10us_tick: %d\n";
#endif//test

AT(.com_text.timer)
void usr_tmr1us_isr(void)
{

}

AT(.com_text.isr)
void timer2_isr(void)
{
    if(TMR2CON & BIT(9)){
        TMR2CPND = BIT(9);
    }
    // usr_tmr1us_isr();

#if US_1S_TEST
    us10_tick++;
    if(100000 == us10_tick)
    {//1000 000us test
        my_printf(str_t3, us10_tick);
        us10_tick = 0;
    }
#endif//US_1S_TEST
#if US_IO_TEST
    GPIOASET = BIT(9);
    asm("nop");asm("nop");asm("nop");
    GPIOACLR = BIT(9);
#endif
}

void timer2_init(void)
{
    printf("timer2_init\n");
    TMR2CON = 0;
    TMR2CNT = 0;
    TMR2PR  = 60 - 1;                                   //60/6000000 == 0.000001s
    TMR2CON = (2 << 1) | (2 << 4) | BIT(7) | BIT(0);    //timer2 clk = xosc24m, div 4
    sys_irq_init(IRQ_TMR2_VECTOR, 0, timer2_isr);
#if US_1S_TEST
    us10_tick = 0;
#endif//US_1S_TEST
#if US_IO_TEST
    GPIOAFEN &= ~BIT(9);
    GPIOADE  |=  BIT(9);
    GPIOADIR &= ~BIT(9);
#endif
}

#endif
