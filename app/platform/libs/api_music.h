#ifndef _API_MUSIC_H
#define _API_MUSIC_H

#define SEEK_SET    0
#define SEEK_CUR    1

#define AVFMT_INVALID              0                        //音频格式无效
#define AVFMT_CORRECT              BIT(0)                   //音频格式有效
#define AVFMT_NOT_SUPPORT         (BIT(0) | BIT(1))         //音频格式有效，但超出解码能力

typedef enum {
    NONE_TYPE = 0,
    WAV_TYPE,
    MP3_TYPE,
    VSBC_TYPE,
    ESBC_TYPE,
} enum_msc_type_t;

enum {
    MUSIC_STOP = 0,
    MUSIC_PAUSE,
    MUSIC_PLAYING,
};

enum {
    //decode msg
    MUSIC_MSG_STOP = 0,
    MUSIC_MSG_PAUSE,
    MUSIC_MSG_PLAY,
    MUSIC_MSG_FRAME,

    //encode msg
    ENC_MSG_MP3 = 32,
    ENC_MSG_AEC,
    ENC_MSG_ALC,
    ENC_MSG_PLC,
    ENC_MSG_SBC,
    ENC_MSG_WAV,            //pcm wave
    ENC_MSG_ADPCM,          //adpcm-ima wave
    ENC_MSG_SPDIF,
    ENC_MSG_I2S,
};

typedef struct {
    u8  sta;
    u8  type;
    u8  sysclk;                    //保存解码前的系统时钟
    u16 pcm_samples;
    u32 *pcm_buf;
    u16 total_time;
    u16 cur_time_ms;
} music_cb_t;
extern music_cb_t music_cb;

//codec common info
typedef struct  {
    u32 frame_count;                //current frame count
    u32 file_ptr;                   //file ptr of current frame
    u16 samples;                    //sample pionts of frame
    u8  spr;                        //sample rate of enum number
    u32 sample_rate;                //sample rate
    u32 bitrate;                    //bit rate
} codec_info_t;
extern codec_info_t *codec_info;

extern unsigned char avio_buf[556];

int wav_decode_init(void);
int mp3_decode_init(void);
int spi_mp3_decode_init(void);
int music_decode_init(void);
void music_control(u8 sta);
bool music_decode(void);
u8 get_music_dec_sta(void);
void music_pcm_gpdma_kick(void);

u16 music_get_total_time(void);
u16 music_get_cur_time(void);
void music_set_cur_time(u32 cur_time);
void music_set_jump(void *brkpt);
void music_get_breakpiont(void *brkpt);
u32 get_music_bitrate(void);

bool mp3_res_is_play(void);
bool mp3_res_play_kick(u32 addr, u32 len);
void mp3_res_play_exit(void);
void esbc_res_play_kick(u32 addr, u32 len);
void esbc_res_play_exit(void);
void esbc_link_play_init(void);
void esbc_link_play_kick(u32 addr, u32 len, u16 sta_skip_frame, u16 end_skip_frame);

bool wav_res_is_play(void);
bool wav_res_decode(u16 gSpeedIn);
bool wav_res_play_kick(u32 addr, u32 len);
void wav_res_play_exit(void);
void wav_dec_stop(void);

void res_play_loop_back(u8 enable);                 //spiflash循环播放接口
void res_play_loop_back_skip_size(u16 skip_star_size, u16 skip_end_size); //外接FLASH音乐循环播放开关,设置循环时候跳过前后多少字节音频数据，用于部分应用避开静音区数
void spiflash_rec_mp3_filelen_reduce(u16 ms);       //剪掉REC尾部 ms(spiflash)

#endif // _API_MUSIC_H
