#include "include.h"
#include "asr.h"

#if (WEIGHT_MINI_EN == 1)
    #define BASE_ADDR   WEIGHT_MINI_BIN_ADDR     //与app.xm地址相对应
    #if (ASR_LIBS_SEL == ASR_ZH)
    #define BASE_LEN    WEIGHT_MINI_DATA_LEN
    #define W7OD        392
    #else
    #define BASE_LEN    WEIGHT_EN_DATA_LEN
    #define W7OD        448
    #endif
#else
    #define BASE_ADDR   WEIGHT_BIN_ADDR     //与app.xm地址相对应
    #if (ASR_LIBS_SEL == ASR_ZH)
    #define BASE_LEN    WEIGHT_DATA_LEN
    #define W7OD        488
    #else
    #define BASE_LEN    WEIGHT_EN_DATA_LEN
    #define W7OD        448
    #endif
#endif

// extern const int16_t tdnn_mean_1[128];
// extern const int16_t tdnn_bias_1[128];
// extern const float tdnn_var_1[128];
// extern const int16_t tdnn_mean_2[128];
// extern const int16_t tdnn_bias_2[128];
// extern const float tdnn_var_2[128];
// extern const int16_t tdnn_mean_3[128];
// extern const int16_t tdnn_bias_3[128];
// extern const float tdnn_var_3[128];
// extern const int16_t tdnn_mean_4[128];
// extern const int16_t tdnn_bias_4[128];
// extern const float tdnn_var_4[128];
// extern const int16_t tdnn_mean_5[128];
// extern const int16_t tdnn_bias_5[128];
// extern const float tdnn_var_5[128];
// extern const int16_t tdnn_mean_6[128];
// extern const int16_t tdnn_bias_6[128];
// extern const float tdnn_var_6[128];
// extern const int16_t tdnn_bias_7[W7OD];
// extern const float tdnn_scale_7[W7OD];

#if (TDNN_HW_EN && ASR_RECOG_EN)

typedef struct {
    u16 all_frame;
    u32 load_addr;
} tdnn_pretetch_t;

#if ASR_SKIP_SAMPLES
volatile u8 asr_prefetch_kisck;
#endif

tdnn_pretetch_t tdnn_pretetch AT(.ws_asr.sum);

int32_t matrix_sum AT(.ws_asr.sum);
int32_t sum_buffer[0x1e8] AT(.ws_asr.sum);
static u8 tdnn_spi_buf[1] AT(.ws_asr.sum);

#define tdnn_round(x) ((x)>=0?((x)+0.5):((x)-0.5))

//tdnn init
AT(.text.tdnn)
void bsp_tdnn_init(void)
{
    tdnn_pretetch_t *t = &tdnn_pretetch;
    memset(t, 0, sizeof(tdnn_pretetch_t));
    t->load_addr = BASE_ADDR;
#if ASR_SKIP_SAMPLES
    #if WEIGHT_MINI_EN
    asr_prefetch_kisck = 4;
    #else
    asr_prefetch_kisck = 5;
    #endif
#endif
}

AT(.com_text.tdnn)
void tdnn_compute(int8_t *in_buf, float in_scale, int in_dim, int out_dim,
                  const int8_t *tdnn_weight, const int16_t *tdnn_bias,
                  const int16_t *tdnn_mean, const float *tdnn_var, const float *tdnn_scale,
                  float *out_buf, int last_layer) {
    int i;

#if ASR_SKIP_SAMPLES
    if (asr_prefetch_kisck) {
        return;
    }
#endif

    tdnn_pretetch_t *t = &tdnn_pretetch;
    u32 frame_len = (out_dim * in_dim);

#if TDNN_HW_EN  //硬件tdnn,需要使用外部资源文件bin获取绝对地址给到tdnn
    memset(sum_buffer, 0,sizeof(sum_buffer));

    bsp_tdnn_start();//tdnn使能后，不能使用DMA发送SPI命令、地址
    bsp_tdnn_set_dimwnsion(out_dim, in_dim);
    bsp_tdnn_kick(sum_buffer, in_buf);

    spiflash_read_kick((u32 *)((u32)tdnn_spi_buf | BIT(31)), t->load_addr, frame_len);

    bsp_tdnn_wait();
    bsp_tdnn_fini();
    spiflash_read_wait();

	t->load_addr += frame_len;
    if (t->load_addr >= (BASE_ADDR + BASE_LEN)) {
        t->load_addr = BASE_ADDR;
    }
#endif

#if !TDNN_HW_EN //软件算tdnn，需要先去掉app.xm资源bin文件，使用内部虚拟flash地址,strong声明要关闭(测试使用)
    int j,k;
    k = 0;
    memset(sum_buffer, 0,sizeof(sum_buffer));
    for(i = 0; i < out_dim; i++){
        const int8_t *weight =  tdnn_weight + i*in_dim;
        for(j = 0; j < in_dim; j++){
            sum_buffer[k] += in_buf[j] * weight[j];
        }
        k++;
    }
    my_printf("sum_buffer software:\n");
    my_print_r32(sum_buffer, out_dim);
    my_printf("in_buf:\n");
    my_print_r(in_buf, in_dim);
#endif

    for (i = 0; i < out_dim; i++) {
        matrix_sum = sum_buffer[i];
        int32_t q = tdnn_round(matrix_sum * in_scale);
        int32_t z = q + tdnn_bias[i];
        if (!last_layer) {
            if (z <= 0) {
                z = 0;
            }
            out_buf[i] = (z - tdnn_mean[i]) * tdnn_var[i];
        } else {
            out_buf[i] = z * tdnn_scale[i];
        }
    }
}
#endif

