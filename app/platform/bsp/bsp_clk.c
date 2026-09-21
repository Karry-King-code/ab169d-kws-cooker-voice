#include "include.h"

//CLK32K时钟源设置
void clk_clk32k_rtc_set(clk32k_rtc_source_e clk_sel)
{
    u32 rtccon0 = RTCCON0;
    rtccon0 = (rtccon0 & ~(0x03 << 8)) | ((clk_sel & 0x03) << 8);
    RTCCON0 = rtccon0;
}
