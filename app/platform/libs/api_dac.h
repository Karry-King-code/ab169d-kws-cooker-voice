#ifndef _API_DAC_H_
#define _API_DAC_H_

enum {
    SPR_48000   = 0,
    SPR_32000,
    SPR_24000,
    SPR_16000,
    SPR_12000,
    SPR_8000,
};

void dac_cb_init(void);
void dac_spr_set(uint spr);                             //设置dac采样率
void dac_set_dvol(u16 vol, u8 vol_step, bool is_direct_set_flag);//设置dac数字音量
void dac_restart(void);                                 //重启dac模块
void dac_power_on(void);                                //开启dac
void dac_power_off(void);                               //关闭dac
void dac_io_map(u8 ch0_map_sel,u8 ch1_map_sel);         //dac引脚重映射
void loudspk_mute_init(u8 io_sel, bool sta);

bool dac_gpdma_is_ready(void);                          //获取dac pending
void dac_gpdma_kick(s16 *ptr, u32 samples);             //输入dac数据, ptr:指向音频数据, samples:16bit位宽的样点数
void dac_gpdma_reset(void);                             //复位dac数据
u16 dnr_buf_maxpow(void *ptr, u16 len);                 //计算ptr指向的音频数据能量值并返回, ptr:指向音频数据, len:8bit位宽的数据长度

//fade time(s) = (digitat_vol - 0)/(2^step)/dac_spr
//for example: 0.17s = (0x7fff - 0)/(2^2)/48000
void dac_fade_in(u8 step);                              //dac淡入
void dac_fade_out(u8 step);                             //dac淡出
#endif
