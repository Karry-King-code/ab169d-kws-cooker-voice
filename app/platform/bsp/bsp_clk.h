#ifndef _BSP_CLK_H
#define _BSP_CLK_H

typedef enum{
    CLK32K_RTC_RC2M_RTC         = 0x1,
    CLK32K_RTC_X24MDIV12_RTC    = 0x3,
}clk32k_rtc_source_e;

void clk_clk32k_rtc_set(clk32k_rtc_source_e clk_sel);

#endif  //_BSP_CLK_H
