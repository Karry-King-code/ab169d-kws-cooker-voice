#ifndef _TKEY_H
#define _TKEY_H

typedef struct {
    u16 tkcd_pr;
} tk_init_typedef;

///按键多击处理，结构体勿修改, Maskrom有使用
typedef struct {
    u8  strikes;
    u8  counter;
    u16 val;
    u8  max_times;                                  //最大检测次数
    u8  resv;
    u16 delay;                                      //多击按键的delay, 单位5ms
    bool (*check_key_return)(u16 key_return);
    const u16 *tbl;                                 //多击按键的table
} key_multi_cb_t;

void bsp_tkey_init(tk_init_typedef *tk_cfg);
void tkey_gpio_config(u8 io_num);
void tkey_gpio_exit(u8 io_num);
void tkey_off(void);
void tkey_cir_scan_en(void);
void tkey_press_update(void);
void tkey_tkbuf_down(void);
extern volatile u32 tk_buf[16];
extern u8 tkey_array[];
#endif
