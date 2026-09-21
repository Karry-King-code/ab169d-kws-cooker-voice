#ifndef FUNC_EXSPIFLASH_MUSIC_H
#define FUNC_EXSPIFLASH_MUSIC_H


#include "spiflash/spiflash1.h"


typedef struct {
    u8  msc2_en;            //V2版本music打包标志
    u16 total_num;
    u16 cur_num;
    u8  pause;
    u8  rec_no_file;
    #if SPIFLASH_MUSIC_BREAK_MEMORY
    msc_breakpiont_t brkpt;//用于保存和恢复播放位置的结构体
    u16 save_time;//用于退出flash模式时保存的时间
    u16 save_num;//用于退出时保存当前播放歌曲的序号
    #endif // SPIFLASH_MUSIC_BREAK_MEMORY
} exspiflash_msc_t;
extern exspiflash_msc_t exspi_msc;


void register_spi_read_function(void (* read_func)(void *buf, u32 addr, u32 len));
void func_exspiflash_music_message(u16 msg);
void exspiflash_music_switch_file(u8 direction);
void exspiflash_rec_switch_file(u8 direction);
extern bool mp3_res_play_kick(u32 addr, u32 len);
void non_volatile_memory(void);

//fade time(s) = (digitat_vol - 0)/(2^step)/dac_spr
//for example: 0.17s = (0x7fff - 0)/(2^2)/48000
void dac_fade_in(u8 step);
void dac_fade_out(u8 step);

void set_auto_param(u32 sample_rate, s32 rate_set);  
void lock_code_autotune(void);
void unlock_code_autotune(void);
void mp3_autotune_play_set(bool flag);
#endif


