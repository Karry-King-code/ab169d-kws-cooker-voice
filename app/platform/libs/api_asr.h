#ifndef _API_ASR_H
#define _API_ASR_H


typedef struct {
    u8 vad_en;
    u8 huart_dump_en;
} asr_cfg_t;


void asr_init(asr_cfg_t* cfg);
void asr_start(void);
void asr_stop(void);
void vad_start(void);
void vad_stop(void);
bool is_vad_start(void);            ///返回VAD是否已经开始检测
bool is_vad_wake(void);             ///返回VAD是否已经唤醒
void vad_sleep_config(bool enter);

void lock_code_vad(void);
void unlock_code_vad(void);

void bsp_tdnn_set_dimwnsion(uint32_t outdim, uint32_t indim);
void bsp_tdnn_start(void);
void bsp_tdnn_fini(void);
void bsp_tdnn_kick(int32_t *out, int8_t *in);
void bsp_tdnn_wait(void);

extern const unsigned short sqrt_fixHanning[257];
void fft_user_init(PreprocessState *m_st);                      //256 samples 512 FFT
void ains3_rdft_forward_256(s32 *buf, PreprocessState *m_st);   
void ains3_rdft_inverse_256(s32* buf, PreprocessState *m_st);
#endif
