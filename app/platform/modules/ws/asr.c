#include "include.h"
#include "asr.h"
#include "fst_graph.h"
#include "fst_words.h"

void asr_prefetch_init(void);
void bsp_tdnn_init(void);

extern volatile u8 asr_prefetch_kisck;
void asr_result_deal(const char *result);

#define TRACE_EN                    1

#define ASR_2_LEVEL_DEAL_EN         0       //唤醒词是否分两级处理
#define ASR_WARNNINGS_EN            1       //是否打开唤醒的提示音
#define ASR_CMD_LEN                 20      //唤醒词的最大长度（字节）

#if TRACE_EN
#define TRACE(...)                  my_printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

AT(.asr_text.asr_deal)
u32 asr_alg_process(short *ptr)
{
    const char *text;
    float score;

    ///asr recog
    int ret = Wanson_ASR_Recog(ptr, 400, &text, &score);

#if ASR_PREFETCH_EN || ASR_SKIP_SAMPLES
    if (asr_prefetch_kisck) {
        asr_prefetch_kisck --;
    }
#endif

    if (ret) {
#if ASR_PREFETCH_EN || ASR_SKIP_SAMPLES
    #if WEIGHT_MINI_EN
        asr_prefetch_kisck = 4;
    #else
        asr_prefetch_kisck = 5;
    #endif
#endif
        if (ret == 1) {
            TRACE("ASR Result: %s\n", text);
//            asr_result_deal(text);
        }
    }

    return ret;
}

void asr_alg_stop(void)
{
    Wanson_ASR_Release();
}

void asr_alg_start(void)
{
    if (Wanson_ASR_Init() < 0) {
        my_printf("Wanson_ASR_Init Failed!\n");
        while (1) WDT_CLR();
    }
    Wanson_ASR_Reset();
    my_printf("Wanson_ASR_Init OK!\n");
#if ASR_PREFETCH_EN
    asr_prefetch_init();
#endif
#if TDNN_HW_EN
    bsp_tdnn_init();
#endif
}

void asr_alg_init(void)
{
//weight standard
    // extern u8 tdnn_weight_1[0x3c00];
    // extern u8 tdnn_weight_2[0x8000];
    // extern u8 tdnn_weight_3[0x8000];
    // extern u8 tdnn_weight_4[0x8000];
    // extern u8 tdnn_weight_5[0x8000];
    // extern u8 tdnn_weight_6[0x4000];
    // extern u8 tdnn_weight_7[0xf400];

    // print_r(tdnn_weight_1, 0x3c00);
    // print_r(tdnn_weight_2, 0x8000);
    // print_r(tdnn_weight_3, 0x8000);
    // print_r(tdnn_weight_4, 0x8000);
    // WDT_CLR();
    // print_r(tdnn_weight_5, 0x8000);
    // print_r(tdnn_weight_6, 0x4000);
    // print_r(tdnn_weight_7, 0xf400);

//weight mini
    // extern u8 tdnn_weight_1[0x3840];
    // extern u8 tdnn_weight_2[0x7080];
    // extern u8 tdnn_weight_3[0x7080];
    // extern u8 tdnn_weight_4[0x7080];
    // extern u8 tdnn_weight_5[0x3840];
    // extern u8 tdnn_weight_6[0xB7C0];

    // print_r(tdnn_weight_1, 0x3840);
    // print_r(tdnn_weight_2, 0x7080);
    // print_r(tdnn_weight_3, 0x7080);
    // print_r(tdnn_weight_4, 0x7080);
    // WDT_CLR();
    // print_r(tdnn_weight_5, 0x3840);
    // print_r(tdnn_weight_6, 0xB7C0);

//weight English
    // extern u8 tdnn_weight_1[0x3c00];
    // extern u8 tdnn_weight_2[0x8000];
    // extern u8 tdnn_weight_3[0x8000];
    // extern u8 tdnn_weight_4[0x8000];
    // extern u8 tdnn_weight_5[0x8000];
    // extern u8 tdnn_weight_6[0x4000];
    // extern u8 tdnn_weight_7[0xE000];

    // print_r(tdnn_weight_1, 0x3c00);
    // print_r(tdnn_weight_2, 0x8000);
    // print_r(tdnn_weight_3, 0x8000);
    // print_r(tdnn_weight_4, 0x8000);
    // WDT_CLR();
    // print_r(tdnn_weight_5, 0x8000);
    // print_r(tdnn_weight_6, 0x4000);
    // print_r(tdnn_weight_7, 0xE000);
}
