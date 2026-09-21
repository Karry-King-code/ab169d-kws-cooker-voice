#ifndef _API_SYSCLK_H_
#define _API_SYSCLK_H_

enum {
    SYS_RC2M,
    SYS_24M,
    SYS_48M,
    SYS_80M,
    SYS_96M,
    SYS_120M,
    SYS_160M,
    SYS_192M,
};

u8 get_cur_sysclk(void);                    //获取当前系统时钟设置
void set_sys_clk(u32 clk_sel);              //设置系统时钟
u32 get_sysclk_nhz(void);                   //获取系统时钟(单位：Hz)
u32 rc2m_clk_nhz_get(u8 type);              //Get RC2M clk frequency, type: 0: system rc2m  1:rtc rc2m 

#endif
