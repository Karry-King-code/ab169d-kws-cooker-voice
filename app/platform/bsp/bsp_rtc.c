#include "include.h"

//Set the RTC alarm register value.
AT(.com_periph.rtc.alarm_set)
void rtc_set_alarm(u32 alarm)
{
    RTCALM = (u32)alarm;
}

//Get the RTC alarm register value.
AT(.com_periph.rtc.alarm_get)
u32 rtc_get_alarm(void)
{
    return (u32)RTCALM;
}

void rtc_clr_alarm(void)
{
    RTCALM = 0;
}

//Config predivision factor for 1Hz counter.
void rtc_prescale_set(u32 psc)
{
    u32 rtccon2 = RTCCON2;
    rtccon2 = (rtccon2 & ~0x1ffff) | (psc & 0x1ffff);
    RTCCON2 = rtccon2;
}

//Get the RTC counter 1Hz prescale value.
u32 rtc_prescale_get(void)
{
    return RTCCON2 & 0x1ffff;
}

//获取RTC counter计数(用于获取闹钟关机时经过的时长)
u32 rtc_counter_get(void)
{
    return RTCCNT;
}
