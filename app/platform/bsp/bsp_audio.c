#include "include.h"

void speaker_sdadc_process(u8 *ptr, u32 samples, int ch_mode);
void asr_sdadc_process(u8 *ptr, u32 samples, int ch_mode);

#if (FUNC_SPEAKER_EN && ASR_RECOG_EN)
    #define speaker_sdadc_callback  asr_sdadc_process
#elif FUNC_SPEAKER_EN || FFT_PROC_SAMPLE
    #define speaker_sdadc_callback  speaker_sdadc_process
#else
    #define speaker_sdadc_callback  sdadc_dummy
#endif // FUNC_SPEAKER_EN

#define SDADC_DMA_SIZE           (128*2*3)         //DMASIZE = 最大样点数

u8 sdadc_buf[SDADC_DMA_SIZE * 2] AT(.mic_buf);

//MIC analog gain: 0~3(共4级), 0-9dB,1-12dB,2-18dB,3-21dB
//MIC digital gain = GAIN/2^10,
const sdadc_cfg_t rec_cfg_tbl[] = {
/*   通道,              采样率,     模拟增益, 数字增益,    通路控制,       样点数,   回调函数*/
#if ASR_RECOG_EN
    {MIC_CHANNEL_CFG,   SPR_16000,    3, MIC_DIG_P18DB, 0,              SDADC_DMA_SIZE,       asr_sdadc_process},    /* ASR     */
#elif MIC_ESBC_PROC_EN
    {MIC_CHANNEL_CFG,   SPR_8000,     3, MIC_DIG_P18DB, ADC2DAC_EN,     640,         speaker_sdadc_callback},
#else
    {MIC_CHANNEL_CFG,   SPR_16000,    3, MIC_DIG_N0DB , ADC2DAC_EN,     512,     	 speaker_sdadc_callback},       /* SPEAKER */
#endif
};

u32 mic_getcfg_bias_method(u16 channel)
{
    if (channel & 0x0f) {
//        return xcfg_cb.micl_bias_method;
        return 0;
    }
    if (channel & 0xf0) {
//        return xcfg_cb.micr_bias_method;
        return 0;
    }
    return 0;
}

u16 get_sdadc_dma_size(void)
{
    return SDADC_DMA_SIZE;
}

AT(.rodata.audio)
const u16 mic_dig_tbl[37] = {
    MIC_DIG_N0DB,  MIC_DIG_P1DB,  MIC_DIG_P2DB,  MIC_DIG_P3DB,  MIC_DIG_P4DB,  MIC_DIG_P5DB,  MIC_DIG_P6DB,  MIC_DIG_P7DB,
    MIC_DIG_P8DB,  MIC_DIG_P9DB,  MIC_DIG_P10DB, MIC_DIG_P11DB, MIC_DIG_P12DB, MIC_DIG_P13DB, MIC_DIG_P14DB, MIC_DIG_P15DB,
    MIC_DIG_P16DB, MIC_DIG_P17DB, MIC_DIG_P18DB, MIC_DIG_P19DB, MIC_DIG_P20DB, MIC_DIG_P21DB, MIC_DIG_P22DB, MIC_DIG_P23DB,
    MIC_DIG_P24DB, MIC_DIG_P25DB, MIC_DIG_P26DB, MIC_DIG_P27DB, MIC_DIG_P28DB, MIC_DIG_P29DB, MIC_DIG_P30DB, MIC_DIG_P31DB,
    MIC_DIG_P32DB, MIC_DIG_P33DB, MIC_DIG_P34DB, MIC_DIG_P35DB, MIC_DIG_P36DB,
};

void audio_path_init(u8 path_idx)
{
    sdadc_cfg_t cfg;
    memcpy(&cfg, &rec_cfg_tbl[path_idx], sizeof(sdadc_cfg_t));
    if (path_idx == AUDIO_PATH_SPEAKER) {
        cfg.anl_gain = (u8)xcfg_cb.mic_anl_gain;
        cfg.dig_gain = (u16)mic_dig_tbl[xcfg_cb.mic_dig_gain];
        // my_printf("anl_gain: %d, dig_gain: %d\n", cfg.anl_gain, cfg.dig_gain);
    }
    my_printf("sample_rate: %d, samples: %d\n", cfg.sample_rate, cfg.samples);
    sdadc_init(&cfg);
}

void audio_path_start(u8 path_idx)
{
    sdadc_cfg_t cfg;
    memcpy(&cfg, &rec_cfg_tbl[path_idx], sizeof(sdadc_cfg_t));
    sdadc_start(cfg.channel);
}

void audio_path_exit(u8 path_idx)
{
    sdadc_cfg_t cfg;
    memcpy(&cfg, &rec_cfg_tbl[path_idx], sizeof(sdadc_cfg_t));

    sdadc_exit(cfg.channel);

    // adpll_spr_set(0);
}
