#include "include.h"
#include "func.h"
#include "func_speaker.h"
#include "sfr.h"
#include "sound_res.h"

#if FUNC_SPEAKER_EN

func_speaker_t f_spk;
volatile bool flag_spk_data_kick = false;
#if ASR_AINS_EN && (!ASR_RECOG_EN)
static ains3_cb_t ains3_cb AT(.ains3_buf.sta);
#endif

#if FUNC_REC_EN
extern u8 mp3enc_flag;
u8 mpa_is_encoding(void);
int mpa_encode_frame(void);
#endif

#if MIC_ESBC_PROC_EN
static u8 esbc_enbuf[AUDIO_TX_FRAME] AT(.esbc_buf);
static u8 esbc_debuf[AUDIO_RX_FRAME] AT(.esbc_buf);
static s32 esbc_tmp_debuf[(AUDIO_RX_FRAME/2)] AT(.esbc_buf);

AT(.com_text.iis_ext_cst)
const char buf_error[] = "*%d";
#endif

#if SPK_DNN_EN
extern u32 __aram_dnn_comm_vma, __aram_dnn_comm_lma, __aram_dnn_comm_size;
static dnn_cb_t dnn_cb AT(.dnn_data.sta);

void spk_dnn_param_init(u8 *sysclk)
{
    memset(&dnn_cb, 0, sizeof(dnn_cb_t));
    dnn_cb.nt                   = SPK_DNN_LEVEL;
//	  dnn_cb.nt_post              = 0;
    dnn_cb.exp_range_H          = 1;
//	  dnn_cb.exp_range_L          = 0;
//    dnn_cb.param_printf         = 1;
//    dnn_cb.min_value            = 50;
//    dnn_cb.nostation_floor      = 0;
//    dnn_cb.wind_thr             = 5;
//    dnn_cb.noise_ps_rate        = 1;
//    dnn_cb.prior_opt_idx        = 3;
    dnn_cb.prior_opt_ada_en       = 1;
//    dnn_cb.wind_level           = 0;						//0：不打开风噪
//	  dnn_cb.wind_range           = 0;
	dnn_cb.low_fre_range          = 16;                     //范围0~16
//	dnn_cb.low_fre_range0         = 1;
//  dnn_cb.pitch_filter_en        = 1;
    dnn_cb.mask_floor             = 1600;
	dnn_cb.mask_floor_r           = 0;
	dnn_cb.music_lev              = 11;
	dnn_cb.gain_expand            = 1024;
	dnn_cb.nn_only                = 1;
	dnn_cb.nn_only_len            = 16;
	dnn_cb.gain_assign            = 16666;
    dnn_cb.sin_gain_post_en	      = 1;
	dnn_cb.sin_gain_post_len	  = 0;
	dnn_cb.sin_gain_post_len_f    = 256;
    dnn_cb.preem_en               = 0;

    *sysclk = SYS_192M;
}
#endif

#if MIC_REC_EN
AT(.text.bsp.mic)
void mic_rec_start(void)
{
    f_spk.rec_en = 1;
}

AT(.text.bsp.mic)
void mic_rec_stop(void)
{
    f_spk.rec_en = 0;
}

AT(.text.bsp.mic)
void mic_rec_init(void)
{
    rec_src.spr = SPR_16000;
    rec_src.nchannel = 1;
    rec_src.source_start = mic_rec_start;
    rec_src.source_stop  = mic_rec_stop;
    f_spk.rec_en = 0;
}
#endif // MIC_REC_EN

AT(.text.bsp.speaker)
void func_speaker_stop(void)
{
    // dac_fade_out();
    // dac_fade_wait();                    //等待淡出完成
    audio_path_exit(AUDIO_PATH_SPEAKER);
}

AT(.text.bsp.speaker)
void func_speaker_start(void)
{
#if ASR_RECOG_EN
    bsp_asr_start();
#endif
    // dac_fade_wait();                    //等待淡出完成
    dac_gpdma_reset();
    audio_path_init(AUDIO_PATH_SPEAKER);
    audio_path_start(AUDIO_PATH_SPEAKER);
    // dac_fade_in();
}

AT(.text.bsp.speaker)
void func_speaker_pause_play(void)
{
    if (f_spk.pause) {
        func_speaker_start();
    } else {
        func_speaker_stop();
    }
    f_spk.pause ^= 1;
}

AT(.text.bsp.speaker)
void func_speaker_mp3_res_play(u32 addr, u32 len)
{
    if (len == 0) {
        return;
    }

#if MIC_REC_EN
    sfunc_record_pause();
#endif // MIC_REC_EN

    if (!f_spk.pause) {
        func_speaker_stop();
#if WARNING_MP3_PLAY
        mp3_res_play(addr, len);
#endif
        func_speaker_start();
    } else {
#if WARNING_MP3_PLAY
        mp3_res_play(addr, len);
#endif
    }

#if MIC_REC_EN
    sfunc_record_continue();
#endif // MIC_REC_EN
}

AT(.text.bsp.speaker)
void func_speaker_setvol_callback(u8 dir)
{
    if (f_spk.pause) {
        func_speaker_pause_play();
    }
}

#if MIC_EQ_EN
AT(.com_text.func.speaker)
void speaker_mic_eq(s16 *ptr, u32 samples)
{
    // int i;
    // s16 pcm;

    // for (i = 0; i < samples; i++) {
    //     pcm = ptr[i];
    //     ptr[i] = mic_eq_proc(pcm);
    // }
}
#endif

#if FFT_PROC_SAMPLE
static nr_buf_t nr_buf AT(.fft_buf.buf);
static PreprocessState m_st_sta AT(.fft_buf.m_st);

AT(.com_text.func.fft)
void speaker_process_256(s16 *data)
{
    int i;
	s16 *output = data;
    nr_buf_t *p = &nr_buf;
    PreprocessState *m_st = &m_st_sta;
    for (i = 0; i < PART_LEN; i++) {
		p->xbuf[i + PART_LEN] = data[i];
	}
    // Windowed near fft
	for(i = 0; i < PART_LEN; i++) {
		ML0(p->xbuf[i], sqrt_fixHanning[i], m_st->mu_ains3);
		p->fft[i] = MLZ(HANN_SHIFT, m_st->mu_ains3);
		ML0(p->xbuf[PART_LEN + i], sqrt_fixHanning[PART_LEN - i], m_st->mu_ains3);
		p->fft[PART_LEN + i] = MLZ(HANN_SHIFT, m_st->mu_ains3);
	}

    ains3_rdft_forward_256((s32*)p->fft, m_st);

    ains3_rdft_inverse_256((s32*)p->fft, m_st);

    // Overlap and add to obtain output.
	for(i = 0; i < PART_LEN; i++) {
		// Do we need saturation ?
		p->fft[i] = AEC_SPL_SAT(AEC_SPL_WORD16_MAX, p->fft[i], AEC_SPL_WORD16_MIN);
        ML0(p->fft[i], sqrt_fixHanning[i], m_st->mu_ains3);
		p->fft[i] = MLZ(HANN_SHIFT, m_st->mu_ains3)+p->outBuf[i];

        output[i] = (short)AEC_SPL_SAT(AEC_SPL_WORD16_MAX, p->fft[i], AEC_SPL_WORD16_MIN);

		ML0(p->fft[PART_LEN + i], sqrt_fixHanning[PART_LEN - i], m_st->mu_ains3);
		p->outBuf[i] = MLZ(HANN_SHIFT, m_st->mu_ains3);
	}
    memcpy(p->xbuf, p->xbuf + PART_LEN, 2 * PART_LEN);
}
#endif

#if DAC_MIC_MAXPOW_EN
AT(.com_rodata.bat)
const char mic_pow_str[] = "mic_max_pow: %d\n";
#endif

#define sin_output_test     0
#if sin_output_test
//16K 1KHz
AT(.com_rodata.sin_data)
const s8 sin1khz_16k_data[512] = {
    0x00, 0x00, 0x8D, 0x18, 0x5C, 0x2D, 0x44, 0x3B, 0x26, 0x40, 0x45, 0x3B, 0x5C, 0x2D, 0x8D, 0x18,
    0x00, 0x00, 0x73, 0xE7, 0xA3, 0xD2, 0xBC, 0xC4, 0xDA, 0xBF, 0xBC, 0xC4, 0xA4, 0xD2, 0x74, 0xE7,
    0x00, 0x00, 0x8D, 0x18, 0x5C, 0x2D, 0x44, 0x3B, 0x26, 0x40, 0x45, 0x3B, 0x5C, 0x2D, 0x8D, 0x18,
    0x00, 0x00, 0x73, 0xE7, 0xA3, 0xD2, 0xBC, 0xC4, 0xDA, 0xBF, 0xBC, 0xC4, 0xA4, 0xD2, 0x74, 0xE7,
    0x00, 0x00, 0x8D, 0x18, 0x5C, 0x2D, 0x44, 0x3B, 0x26, 0x40, 0x45, 0x3B, 0x5C, 0x2D, 0x8D, 0x18,
    0x00, 0x00, 0x73, 0xE7, 0xA3, 0xD2, 0xBC, 0xC4, 0xDA, 0xBF, 0xBC, 0xC4, 0xA4, 0xD2, 0x74, 0xE7,
    0x00, 0x00, 0x8D, 0x18, 0x5C, 0x2D, 0x44, 0x3B, 0x26, 0x40, 0x45, 0x3B, 0x5C, 0x2D, 0x8D, 0x18,
    0x00, 0x00, 0x73, 0xE7, 0xA3, 0xD2, 0xBC, 0xC4, 0xDA, 0xBF, 0xBC, 0xC4, 0xA4, 0xD2, 0x74, 0xE7,

    0x00, 0x00, 0x8D, 0x18, 0x5C, 0x2D, 0x44, 0x3B, 0x26, 0x40, 0x45, 0x3B, 0x5C, 0x2D, 0x8D, 0x18,
    0x00, 0x00, 0x73, 0xE7, 0xA3, 0xD2, 0xBC, 0xC4, 0xDA, 0xBF, 0xBC, 0xC4, 0xA4, 0xD2, 0x74, 0xE7,
    0x00, 0x00, 0x8D, 0x18, 0x5C, 0x2D, 0x44, 0x3B, 0x26, 0x40, 0x45, 0x3B, 0x5C, 0x2D, 0x8D, 0x18,
    0x00, 0x00, 0x73, 0xE7, 0xA3, 0xD2, 0xBC, 0xC4, 0xDA, 0xBF, 0xBC, 0xC4, 0xA4, 0xD2, 0x74, 0xE7,
    0x00, 0x00, 0x8D, 0x18, 0x5C, 0x2D, 0x44, 0x3B, 0x26, 0x40, 0x45, 0x3B, 0x5C, 0x2D, 0x8D, 0x18,
    0x00, 0x00, 0x73, 0xE7, 0xA3, 0xD2, 0xBC, 0xC4, 0xDA, 0xBF, 0xBC, 0xC4, 0xA4, 0xD2, 0x74, 0xE7,
    0x00, 0x00, 0x8D, 0x18, 0x5C, 0x2D, 0x44, 0x3B, 0x26, 0x40, 0x45, 0x3B, 0x5C, 0x2D, 0x8D, 0x18,
    0x00, 0x00, 0x73, 0xE7, 0xA3, 0xD2, 0xBC, 0xC4, 0xDA, 0xBF, 0xBC, 0xC4, 0xA4, 0xD2, 0x74, 0xE7,

    0x00, 0x00, 0x8D, 0x18, 0x5C, 0x2D, 0x44, 0x3B, 0x26, 0x40, 0x45, 0x3B, 0x5C, 0x2D, 0x8D, 0x18,
    0x00, 0x00, 0x73, 0xE7, 0xA3, 0xD2, 0xBC, 0xC4, 0xDA, 0xBF, 0xBC, 0xC4, 0xA4, 0xD2, 0x74, 0xE7,
    0x00, 0x00, 0x8D, 0x18, 0x5C, 0x2D, 0x44, 0x3B, 0x26, 0x40, 0x45, 0x3B, 0x5C, 0x2D, 0x8D, 0x18,
    0x00, 0x00, 0x73, 0xE7, 0xA3, 0xD2, 0xBC, 0xC4, 0xDA, 0xBF, 0xBC, 0xC4, 0xA4, 0xD2, 0x74, 0xE7,
    0x00, 0x00, 0x8D, 0x18, 0x5C, 0x2D, 0x44, 0x3B, 0x26, 0x40, 0x45, 0x3B, 0x5C, 0x2D, 0x8D, 0x18,
    0x00, 0x00, 0x73, 0xE7, 0xA3, 0xD2, 0xBC, 0xC4, 0xDA, 0xBF, 0xBC, 0xC4, 0xA4, 0xD2, 0x74, 0xE7,
    0x00, 0x00, 0x8D, 0x18, 0x5C, 0x2D, 0x44, 0x3B, 0x26, 0x40, 0x45, 0x3B, 0x5C, 0x2D, 0x8D, 0x18,
    0x00, 0x00, 0x73, 0xE7, 0xA3, 0xD2, 0xBC, 0xC4, 0xDA, 0xBF, 0xBC, 0xC4, 0xA4, 0xD2, 0x74, 0xE7,

    0x00, 0x00, 0x8D, 0x18, 0x5C, 0x2D, 0x44, 0x3B, 0x26, 0x40, 0x45, 0x3B, 0x5C, 0x2D, 0x8D, 0x18,
    0x00, 0x00, 0x73, 0xE7, 0xA3, 0xD2, 0xBC, 0xC4, 0xDA, 0xBF, 0xBC, 0xC4, 0xA4, 0xD2, 0x74, 0xE7,
    0x00, 0x00, 0x8D, 0x18, 0x5C, 0x2D, 0x44, 0x3B, 0x26, 0x40, 0x45, 0x3B, 0x5C, 0x2D, 0x8D, 0x18,
    0x00, 0x00, 0x73, 0xE7, 0xA3, 0xD2, 0xBC, 0xC4, 0xDA, 0xBF, 0xBC, 0xC4, 0xA4, 0xD2, 0x74, 0xE7,
    0x00, 0x00, 0x8D, 0x18, 0x5C, 0x2D, 0x44, 0x3B, 0x26, 0x40, 0x45, 0x3B, 0x5C, 0x2D, 0x8D, 0x18,
    0x00, 0x00, 0x73, 0xE7, 0xA3, 0xD2, 0xBC, 0xC4, 0xDA, 0xBF, 0xBC, 0xC4, 0xA4, 0xD2, 0x74, 0xE7,
    0x00, 0x00, 0x8D, 0x18, 0x5C, 0x2D, 0x44, 0x3B, 0x26, 0x40, 0x45, 0x3B, 0x5C, 0x2D, 0x8D, 0x18,
    0x00, 0x00, 0x73, 0xE7, 0xA3, 0xD2, 0xBC, 0xC4, 0xDA, 0xBF, 0xBC, 0xC4, 0xA4, 0xD2, 0x74, 0xE7,
};

//1K
AT(.com_rodata.ude)
const s16 sin_48k_tbl[48] = {
    0x0000, 0x085F, 0x109A, 0x188D, 0x2013, 0x270E, 0x2D5C, 0x32E4,
    0x378F, 0x3B45, 0x3DF7, 0x3F99, 0x4026, 0x3F9B, 0x3DF7, 0x3B45,
    0x378E, 0x32E4, 0x2D5C, 0x270E, 0x2014, 0x188D, 0x109B, 0x085F,
    0x0000, 0xF7A0, 0xEF65, 0xE774, 0xDFED, 0xD8F2, 0xD2A4, 0xCD1C,
    0xC872, 0xC4BC, 0xC208, 0xC067, 0xBFD9, 0xC067, 0xC209, 0xC4BC,
    0xC872, 0xCD1B, 0xD2A4, 0xD8F3, 0xDFEC, 0xE773, 0xEF65, 0xF7A1,
};
#endif

#if MIC_ESBC_PROC_EN
//近端mic数据处理
AT(.com_text.func.speaker)
void mic_esbc_encode_process(u8 *ptr, u32 samples)
{
//近端mic数据进行esbc压缩
    for(u32 i = 0; i < ESBC_CAL_TIME; i++){
        esbc_encode_8kbps(ptr, &esbc_enbuf[i*UART_AUDIO_ESBC], UART_AUDIO_FRAME);
        ptr += UART_AUDIO_FRAME;
        #if 0
        esbc_decode_8kbps(&esbc_enbuf[i*UART_AUDIO_ESBC], &esbc_debuf[i*UART_AUDIO_FRAME], UART_AUDIO_ESBC, &esbc_dec_cb);
        my_print_r(&esbc_debuf[i*UART_AUDIO_FRAME],160);
        #endif
    }

//近端mic数据发送,写入发送buf
#if UART_AUDIO_EN && MIC_ESBC_PROC_EN
    if(spk_write_to_cirbuf_process(esbc_enbuf, AUDIO_TX_FRAME)){
        flag_spk_data_kick = true;//kick spi send proc
    }
#endif
}
#endif

AT(.com_text.func.speaker)
void speaker_pcm_2_send(u8 *ptr, u32 samples)
{
#if UART_AUDIO_EN
//mic pcm数据发送
#if 0
    for (u32 i = 0; i < samples; i++) {
        my_spi1_putc(ptr[i * 2]);
        my_spi1_putc(ptr[(i * 2) + 1 ]);
    }
#endif
    if(/*1 == xcfg_cb.spi1_slave_master*/0){    //gzm
        if(spk_write_to_cirbuf_process(ptr, samples*2)){
            //after DMA send cmd03
            // my_printf("+");
            flag_spk_data_kick = true;//kick spi send proc
        }
        // bsp_send_uart2_data(ptr, samples*2);
    }else{
        if(spk_write_to_cirbuf_process(ptr, samples*2)){
            flag_spk_data_kick = true;//kick spi send proc
        }
    }
#endif
}

AT(.com_text.func.speaker)
void speaker_sdadc_process(u8 *ptr, u32 samples, int ch_mode)
{
#if MIC_ESBC_PROC_EN
    u32 i;
#endif

#if MIC_REC_EN
    if (f_spk.rec_en) {
        puts_rec_obuf(ptr, (u16)(samples << (1 + ch_mode)));
        mpa_is_encoding();
        if(mp3enc_flag){
            mp3enc_flag = 0;
            mpa_encode_frame();
        }
    }
#endif //MIC_REC_EN
#if MIC_EQ_EN
    speaker_mic_eq((s16 *)ptr,samples);
#endif // MIC_EQ_EN
#if DAC_MIC_MAXPOW_EN//mic能量检测
    u16 mic_max_pow = dnr_buf_maxpow(ptr, samples);
    my_printf(mic_pow_str, mic_max_pow);
#endif
#if SPK_SPI_DUMP
    for (u32 i = 0; i < (samples*2); i++) {
        my_spi_putc(ptr[i]);
    }
#endif
#if HUART_DEUMP_EN
    dump_putbuf(ptr, (samples*2), 0);
#endif

#if UART_AUDIO_EN && MIC_ESBC_PROC_EN
    u8 *esbc_de_pt;

    memset(esbc_tmp_debuf, 0, sizeof(esbc_tmp_debuf));

    if(far_mic1_r_cirbuf_process(esbc_enbuf, AUDIO_TX_FRAME)){
        for(i = 0; i < ESBC_CAL_TIME; i++){
            esbc_decode_8kbps(&esbc_enbuf[i*UART_AUDIO_ESBC], &esbc_debuf[i*UART_AUDIO_FRAME], UART_AUDIO_ESBC, &esbc_dec_cb);
        }
        esbc_de_pt = esbc_debuf;
    }else{
        memset(esbc_debuf, 0, sizeof(esbc_debuf));
        esbc_de_pt = esbc_debuf;
        //my_printf(buf_error, 0);
    }

    if (dac_gpdma_is_ready()) {
        dac_gpdma_kick((s16*)esbc_de_pt, samples);
    }
#endif

#if MIC_ESBC_PROC_EN
    mic_esbc_encode_process(ptr, samples);
#endif

#if ASR_AINS_EN
    spk_ains3_plus_process((s16 *)ptr);
#endif
#if FFT_PROC_SAMPLE
    speaker_process_256((s16 *)ptr);		//fft->ifft 每次处理512byte
#endif
#if SPK_DNN_EN
    spk_dnn_process((s16 *)ptr);
#endif
#if HUART_DEUMP_EN
    dump_putbuf(ptr, (samples*2), 1);
#endif
#if sin_output_test
    memcpy(ptr, sin1khz_16k_data, samples*2);
#endif
    if (dac_gpdma_is_ready()) {
        dac_gpdma_kick((s16 *)ptr, samples);
    }
}

AT(.text.func.speaker)
void func_speaker_process(void)
{
    func_process();
}

static void func_speaker_enter(void)
{
    memset(&f_spk, 0, sizeof(f_spk));
    func_cb.mp3_res_play = func_speaker_mp3_res_play;
    func_cb.set_vol_callback = func_speaker_setvol_callback;

    func_speaker_enter_display();
#if WARNING_FUNC_SPEAKER
    mp3_res_play(RES_BUF_SPK_MODE_MP3, RES_LEN_SPK_MODE_MP3);
#endif // WARNING_FUNC_SPEAKER

#if MIC_REC_EN
    mic_rec_init();
#endif // MIC_REC_EN

#if MIC_ESBC_PROC_EN
    memset(esbc_enbuf, 0, sizeof(esbc_enbuf));
    memset(esbc_debuf, 0, sizeof(esbc_debuf));
    audio_esbc_encode_decode_init(&esbc_dec_cb);
#endif

#if MIC_EQ_EN
    mic_set_eq_by_res(&RES_BUF_EQ_MIC_16K_EQ, &RES_LEN_EQ_MIC_16K_EQ);
#endif

#if ASR_RECOG_EN
    bsp_asr_init();
#endif

#if LED_DISP_EN
    led_aux_play();
#endif // LED_DISP_EN

#if FFT_PROC_SAMPLE
    fft_user_init(&m_st_sta);
#endif

#if ASR_AINS_EN && (!ASR_RECOG_EN)
	memset(&ains3_cb, 0, sizeof(ains3_cb_t));
	ains3_cb.nt             = ASR_AINS3_LEVEL;
	ains3_cb.prior_opt_idx  = 6;
    ains3_cb.ns_ps_rate     = 1;
    spk_ains3_init(&ains3_cb);
#endif

#if SPK_DNN_EN
    memcpy(&__aram_dnn_comm_vma, &__aram_dnn_comm_lma, (u32)&__aram_dnn_comm_size);    //复用aram

    u8 sysclk;
    sysclk = get_cur_sysclk();
    spk_dnn_param_init(&sysclk);
    set_sys_clk(sysclk);
    spk_dnn_init(&dnn_cb);
#endif
    func_speaker_start();
}

static void func_speaker_exit(void)
{
#if MIC_REC_EN
    sfunc_record_stop();
#endif // MIC_REC_EN

    func_speaker_exit_display();
    func_speaker_stop();
    func_cb.last = FUNC_SPEAKER;
}

#define   TEST_GPIO     4

//#pragma pack(1)
//struct KW_hyp {
//  int32_t score;          // total logit score, including blank scores
//  int16_t out_label;      // FSA output label
//  int32_t nb_score;       // non-blank score, only non-blank token
//  uint8_t nb_tokens;      // number of non-blank token added
//  uint8_t last_nb_steps;  // counter for how many steps has passed since last
//                          // non-blank token
//};


AT(.text.func.speaker)
void func_speaker(void)
{
    printf("%s\n", __func__);

//    printf("size %d\n", sizeof(struct KW_hyp));

    func_speaker_enter();

//    GPIOAFEN &= ~BIT(TEST_GPIO);
//    GPIOADE  |=  BIT(TEST_GPIO);
//    GPIOADIR &= ~BIT(TEST_GPIO);


    while (func_cb.sta == FUNC_SPEAKER) {
        func_speaker_process();
        func_speaker_message(msg_dequeue());
        func_speaker_display();
        /* 异步播报: 处理识别中断请求的回应语(在识别回调外播放, 避免阻塞中断) */
        sound_poll();

//        int top=-1,prob;
//
//    //    uart_putchar('i');
////        GPIOASET = BIT(TEST_GPIO);
//        kws_classify(&top,&prob,NULL,NULL);
////        GPIOACLR = BIT(TEST_GPIO);
//    //    uart_putchar('o');
//
//        if(top>=0)
//        {
//            printf("\r\ntop:%d prob:%d\r\n",top,  prob);
//        }

    }

    func_speaker_exit();
}

#endif // FUNC_SPEAKER_EN

