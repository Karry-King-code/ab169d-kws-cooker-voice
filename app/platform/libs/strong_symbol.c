/**********************************************************************
*
*   strong_symbol.c
*   定义库里面部分WEAK函数的Strong函数，动态关闭库代码
***********************************************************************/
#include "include.h"

#if (!DAC_EN) || HUART_DEUMP_EN
AT(.com_text.dac)
void dac_gpdma_kick(s16 *ptr, u32 samples){}
#else
s16 gpdma_buf[576*2] AT(.gpd_buf);
#endif

#if (!WARNING_WAV_PLAY)
int wav_spi_stream_read(void *buf, unsigned int size){return 0;}
bool wav_spi_stream_seek(unsigned int ofs, int whence){return false;}
void wav_res_play_loop_back(u8 enable){}
void wav_res_play_init_do(u32 addr, u32 len){}
#endif

#if (FUNC_REC_EN && (REC_TYPE_SEL == REC_MP3))
u8 temp_buf[2304] AT(.mp2dec_buf);
#else
u8 temp_buf;
#endif

#if (!ASR_MIC_MAXPOW_EN)
u16 dnr_buf_maxpow(void *ptr, u16 len){return 0;}
#endif

#if !FFT_PROC_SAMPLE
void fft_init(const s32 *rdft, u16 fft_bits){}
void fft_user_init(PreprocessState *m_st){}
#endif

#if !EXSPI_REC_AUTOTUNE_PLAY
void mp3_autotune_process(void *audmabuf, u32 frame_len){};
#endif

#if !WARNING_ESBC_PLAY
void esbc_link_play_kick(u32 addr, u32 len, u16 sta_skip_frame, u16 end_skip_frame){}
void esbc_link_play_init(void){}
int spi_esbc_link_decode_init(void){return 0;}

void esbc_res_play_kick(u32 addr, u32 len){}
void esbc_res_play_exit(void){}
int spi_esbc_decode_init(void){return 0;}
bool esbc_decode_frame(esbc_dec_cb_t *esbc_d_vars){return false;}
int esbc_init(esbc_dec_cb_t *esbc_d_vars){return 0;}
#endif

#if !SPK_DNN_EN
void dnn_fre_process(s32 *f) {}
void dnn_init(dnn_cb_t *dnn_cb) {}
void spk_dnn_init(dnn_cb_t *dnn_cb) {}
void spk_dnn_process(s16 *buf) {}
void dnn_sm_process(void) {}
#endif
