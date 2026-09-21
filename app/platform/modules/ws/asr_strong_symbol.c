#include "include.h"
#include "asr.h"

#if ASR_PREFETCH_EN || TDNN_HW_EN
AT(.ws_text.strong)
const int8_t tdnn_weight_1[1] = {0};
AT(.ws_text.strong)
const int8_t tdnn_weight_2[1] = {0};
AT(.ws_text.strong)
const int8_t tdnn_weight_3[1] = {0};
AT(.ws_text.strong)
const int8_t tdnn_weight_4[1] = {0};
AT(.ws_text.strong)
const int8_t tdnn_weight_5[1] = {0};
AT(.ws_text.strong)
const int8_t tdnn_weight_6[1] = {0};
AT(.ws_text.strong)
const int8_t tdnn_weight_7[1] = {0};
#endif

#if ASR_RECOG_EN

#define tdnn_abs(x)   ((x)>=0?(x):(-x))
#define tdnn_round(x) ((x)>=0?((x)+0.5):((x)-0.5))

void tdnn_quant(float *in_buf, int in_dim, int T, int skip,
                     int8_t *out_buf, float *out_scale) {
    int i, j;
    float abs_val, abs_max = 0.0f;
    float scale;
    int len = in_dim * T;

    for (i = 0; i < len; i++) {
        if (skip && i == in_dim) i = in_dim << 1;
        abs_val = tdnn_abs(in_buf[i]);
        if (abs_val > abs_max) abs_max = abs_val;
    }

    scale = 127.0f / abs_max;

    for (i = 0, j = 0; i < len; i++, j++) {
        if (skip && i == in_dim) i = in_dim << 1;
        out_buf[j] = (int8_t)(tdnn_round(in_buf[i] * scale));
    }

    *out_scale = 1.0f / scale;
}
#endif

#if 0   //测试tdnn

#define AINS3_TEST_DATA     1       //ains3对数据测试

void bsp_asr_init(void);
void bsp_asr_start(void);
u32 asr_alg_process(short *ptr);

#if !AINS3_TEST_DATA
#define SAMPLES_LEN         640     //asr长度
#else
#define SAMPLES_LEN         256
#endif
u8 tpcm_buf[SAMPLES_LEN*2] AT(.ws_asr.sum);

void kws_test(void)
{
    u8 *data = (u8 *)RES_BUF_FILE_ASR_PCM;
    u32 len = RES_LEN_FILE_ASR_PCM;
    // my_printf("len: 0x%X\n", len);
#if AINS3_TEST_DATA
    for(u32 i=0; i < len; i+= (SAMPLES_LEN*2)){
        WDT_CLR();
        memcpy(tpcm_buf, &data[i], sizeof(tpcm_buf));
        my_printf("in:\n");
        my_print_r16(tpcm_buf, SAMPLES_LEN);
        #if ASR_AINS_EN
            spk_ains3_plus_process((s16 *)&tpcm_buf);
        #endif
        my_printf("out:\n");
        my_print_r16(tpcm_buf, SAMPLES_LEN);
    }
#else
    for(u32 i=0; i < len; i+= (SAMPLES_LEN*2)){
        WDT_CLR();
        memcpy(tpcm_buf, &data[i], sizeof(tpcm_buf));
        my_printf("tpcm_buf\n");
        my_print_r(tpcm_buf, 10);
        #if ASR_AINS_EN
            for(u32 k=0; k < 2; k++){
                spk_ains3_plus_process((s16 *)&tpcm_buf[k*512]);//只用过前面两次,asr只用前400p数据
            }
        #endif
        asr_alg_process((int16_t *)tpcm_buf);
    }
#endif
}

void tdnn_cal_test(void)
{
    bsp_asr_init();
    bsp_asr_start();
    WDT_DIS();
    kws_test();
    while(1){
        WDT_CLR();
    }
}
#endif

