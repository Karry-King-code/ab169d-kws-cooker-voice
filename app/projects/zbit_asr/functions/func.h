#ifndef _FUNC_H
#define _FUNC_H

#include "func_idle.h"
#include "func_music.h"
#include "func_speaker.h"
#include "sfunc_record.h"
#include "sfunc_record_play.h"
#include "func_lowpwr.h"
#include "func_exspiflash_music.h"

//task number
enum {
    FUNC_NULL = 0,
    FUNC_MUSIC,
    FUNC_FMRX,
    FUNC_CLOCK,
    FUNC_USBDEV,
    FUNC_AUX,
    FUNC_SPDIF,
    FUNC_SPEAKER,
    FUNC_PWROFF,
    FUNC_SLEEPMODE,
    FUNC_EXSPIFLASH_MUSIC,
    FUNC_I2S,
    FUNC_IDLE,
};

//task control block
typedef struct {
    u8 sta;                                         //cur working task number
    u8 last;                                        //lask task number
    void (*mp3_res_play)(u32 addr, u32 len);        //各任务的语音播报函数接口
    void (*set_vol_callback)(u8 dir);               //设置音量的回调函数，用于各任务的音量事件处理。
} func_cb_t;
extern func_cb_t func_cb;
extern const u8 func_sort_table[];     //任务切换排序table

ALWAYS_INLINE void func_mp3_res_play(u32 addr, u32 len)
{
    if (func_cb.mp3_res_play) {
        func_cb.mp3_res_play(addr, len);
    }
}

ALWAYS_INLINE void func_set_vol_callback(u8 dir)
{
    if (func_cb.set_vol_callback) {
        func_cb.set_vol_callback(dir);
    }
}

u8 get_funcs_total(void);
void func_process(void);
void func_message(u16 msg);

void func_run(void);
void func_music(void);
void func_idle(void);
void func_clock(void);
void func_fmrx(void);
void func_usbdev(void);
void func_aux(void);
void func_spdif(void);
void func_speaker(void);
void func_exspiflash_music(void);

#endif // _FUNC_H
