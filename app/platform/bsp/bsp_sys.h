#ifndef _BSP_SYS_H
#define _BSP_SYS_H

bool rtc_init(void);
void timer1_irq_init(void);
void bsp_sys_mute(void);
void bsp_sys_unmute(void);
void bsp_clr_mute_sta(void);
void bsp_loudspeaker_mute(void);
void bsp_loudspeaker_unmute(void);
void uart0_mapping_sel(void);
void linein_detect(void);
bool linein_micl_is_online(void);
void tws_lr_xcfg_sel(void);
void tws_get_lr_channel(u8 tws_status);
void get_usb_chk_sta_convert(void);
void sd_soft_cmd_detect(u32 check_ms);
void sys_ram_info_dump(void);
void tmr2pwm_cfg_init(void);
void huart_dump_init(void);
void freqdet_init(void);
#endif // _BSP_SYS_H

