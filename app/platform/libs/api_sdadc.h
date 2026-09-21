#ifndef _API_SDADC_H
#define _API_SDADC_H

#define CHANNEL_L           0x0F
#define CHANNEL_R           0xF0
#define CHANNEL_M           0xF00

#define ADC2DAC_EN          0x01        //ADC-->DAC
#define ADC2SRC_EN          0x02        //ADC-->SRC
#define ADC2IRQ_EN          0x04        //ADC-->IRQ（测试用）
#define ADC2DIR_EN          0x08        //ADC-->DAC（测试用）
#define ADC2ASR_EN          0x10        //ADC-->ASR

#define NB_BANDS   			(72)
#define PART_LEN            (256)
#define PART_LEN2           (PART_LEN << 1)
#define G_range		        (8)

#define AEC_SPL_SAT(A, B, C)    (B > A ? A : B < C ? C : B)
#define AEC_SPL_WORD16_MAX      (32767)
#define AEC_SPL_WORD16_MIN      (-32768)
#define HANN_SHIFT              (16)

#define MZR(z)              do {(z) = 0;} while(0)
#define ML0(x, y, z)        ((z)  =  (__int64)(x) * (__int64)(y))
#define MS0(x, y, z)        ((z)  = -(__int64)(x) * (__int64)(y))
#define MLA(x, y, z)        ((z) +=  (__int64)(x) * (__int64)(y))
#define MSB(x, y, z)        ((z) -=  (__int64)(x) * (__int64)(y))
#define MLZ(n, z)           (int)(((z) + (1<<(n-1)))>>(n))

typedef void (*pcm_callback_t)(u8 *ptr, u32 samples, int ch_mode);

typedef struct {
    u16 channel;
    u8 sample_rate;
    u8 anl_gain;            //0-9dB,1-12dB,2-18dB,3-21dB
    u16 dig_gain;           //0-15bit ADC digital gain
    u8 out_ctrl;
    u16 samples;
    pcm_callback_t callback;
} sdadc_cfg_t;

typedef struct {
	s32 ps[PART_LEN+NB_BANDS];
	s32 noise[PART_LEN+NB_BANDS];
    s16 gain2[PART_LEN+NB_BANDS];
    s32 old_ps[NB_BANDS];
    s16 prior[NB_BANDS];
    s16 post[NB_BANDS];
	s16 zeta[NB_BANDS];

    s32 alp1;
	u16 num_ab;

	u32 PH1_s[PART_LEN];
	u8 prior_opt_idx;
	u8 ns_ps_rate;

	s32 min_count;
	u32 nb_adapt;
	s32 YSmin[G_range];
	s32 YStmp[G_range];
	s32 SY[G_range];
	s32 update_prob_yk[G_range];

    s8 noise_suppress;
    s16 gain_ratio;

    u16 work_ip[8];
    __int64 mu_ains3;
} PreprocessState;

typedef struct {
	int fft[PART_LEN2];
	s16 xbuf[PART_LEN2];
	int outBuf[PART_LEN];
} nr_buf_t;

typedef struct {
    u32 nt;
    u8  prior_opt_idx;
	u8  ns_ps_rate;
} ains3_cb_t;

void set_mic_analog_gain(u16 level, u8 channel);            //0~23(共24级), step 3DB (-6db ~ +63db)
void set_aux_analog_gain(u8 level, u8 channel);             //level: 0~4, 000(-6DB), 001(-3DB), 010(0DB), 011(+3DB), 100(+6DB)
void sdadc_dummy(u8 *ptr, u32 samples, int ch_mode);
void sdadc_pcm_2_dac(u8 *ptr, u32 samples, int ch_mode);
void sdadc_var_init(void);
void sdadc_ttp_var_init(void);

int sdadc_init(const sdadc_cfg_t *p_cfg);
int sdadc_start(u16 channel);
int sdadc_exit(u16 channel);

void pwroff_mic_bias_on(void);                              //单独开启mic bias电压

#endif //_API_SDADC_H
