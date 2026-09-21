#ifndef _BSP_FREQDET_H
#define _BSP_FREQDET_H

#define XOSC24M_FREQ       24000
#define FREQDET_MINERR_VAL 3000

enum{
    RTC_RC2M,
    CLKOUT_PIN,
    XOSC24M_CLK,
    PLLDIV4_CLK,
};

void freqdet_irq_init(void);
#endif // _BSP_FREQDET_H
