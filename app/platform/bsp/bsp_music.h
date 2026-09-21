#ifndef _BSP_MUSIC_H
#define _BSP_MUSIC_H

typedef enum{
    MSC_NONE,
    MSC_MP3,    //MP3/VMP3
    MSC_WAV,
    MSC_VSBC,
    MSC_ESBC,
    MSC_MIDI,
}msc_mode_e;

typedef struct{
    msc_mode_e msc_mode;  
}msc_cb_t;

enum {
    NORMAL_MODE,
    SINGLE_MODE,
    FLODER_MODE,
    RANDOM_MODE,
};

typedef struct{
    u32 *addr;
    u32 *len;
    u16 sta_skip_frame;     //开头跳过的帧数
    u16 end_skip_frame;     //末尾跳过的帧数
}esbc_link_t;

void mp3_res_play(u32 addr, u32 len);
void mp3_res_play_do(u32 addr, u32 len, bool sync);
void esbc_res_play(u32 addr, u32 len);
void esbc_res_play_do(u32 addr, u32 len, bool sync);
void esbc_link_play_do(const esbc_link_t *elink, const u8 e_num);
void wav_res_play(u32 addr, u32 len);
#endif // _BSP_MUSIC_H
