#include "include.h"
#include "zbit_kws/kws_task.h"
#include "xcfg.h"
#include "sha256.h"
#include "zbitkws.h"
#include "led_ctrl.h"
#include "wake_ctrl.h"
#include "sound_res.h"
#include "uart_cmd.h"

#include "../modules/zbit_kws/libkws_AB169_634_180k_V3.1.0_cn_wd81_108_83e389a_20260817_154457.c"

#if ASR_RECOG_EN

#define TRACE_EN                    0

#if TRACE_EN
#define TRACE(...)                  printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#define KWS_SAMPLES                 512         //KWS一帧的样点数，固定值

#define VAD_DMA_DEFAULT_SIZE        1280
#define ASR_PCM_LEN                 (1280*2)
#define ASR_PCM_BUFF_NUM            (5)
#define ASR_PRE_READ                (3)
#define VAD_START_THRESHOLD_VALUE   (1500000)
#define VAD_STOP_THRESHOLD_VALUE    (2000000)

uint8_t cgf_asr_recog_en = ASR_RECOG_EN;
void dump_putbuf(u8 *buf,  int buf_len, u8 file_idx);

struct asr_buf_t {
    volatile s16 wptr;
    volatile s16 rptr;
} asr_buf AT(.ws_asr.data);

typedef struct {
    uint8_t *rptr;
    uint8_t *wptr;
    uint8_t *start;
    uint8_t *end;
    uint16_t total;
    uint16_t trig;
} ring_buf_t;

typedef enum {
    ASR_128 = 0,
    ASR_256,
    ASR_512,
    ASR_1024,
} ASR_LEN;

typedef enum {
    VAD_IDLE = 0,
    VAD_START,
    VAD_CONTINUE,
} VAD_STA;

typedef enum {
    VAD_NORMAL = 0,
    VAD_SLEEP,
    VAD_W4_WAKE,
} VAD_SLEEP_STA;

typedef struct {
    volatile u8 sta;
    u32 samples;                //VAD的样点数
    volatile u32 vad_reboot_cnt;

    //DMA有关的
    volatile u16 offset;        //VADDMACON0的高16位，转成u16的样点数再计算
    u16* speech_start;          //语音在adcdma上对应的起始地址
    u32 dma_len;                //DMA总长度
    u8* dma_ptr;                //DMA数据的起始地址
    u8* dma_ptr_t;
    u32 dma_addr;
    u32 dma_addr_end;

} vad_cb_t;

void asr_kick_start(void);
void asr_vad_callback(int16_t *ptr, u16 samples, u8 voice_flag);
void asr_huart_init(void);
bool asr_huart_putcs(const void *buf, uint len);
void sys_enter_sleep_vad(void);
void vad_irq_init(void);
void spk_ains3_init(ains3_cb_t *ains3_cb);
void spk_ains3_plus_process(s16 *buf);
u32 asr_alg_process(short *ptr);
void asr_alg_stop(void);
void asr_alg_start(void);
void asr_alg_init(void);
int vad_process(int16_t *ptr);
int vad_init(u32 start_threshold_value, u32 stop_threshold_value);
void ring_buf_init(ring_buf_t *pbuf, void *start, uint16_t size, uint16_t trig);
bool ring_buf_put(ring_buf_t *pbuf, uint8_t *ptr, uint16_t len);
bool ring_buf_get(ring_buf_t *pbuf, void (*callback)(uint8_t *, uint16_t), uint16_t len);
void adpcm_encode_process(void);

#if ASR_AINS_EN
static ains3_cb_t ains3_cb AT(.ains3_buf.sta);
#endif
#if ASR_VAD_EN
volatile int vad_state AT(.ws_asr.vad);
u8 ws_sysclk AT(.ws_asr.vad);
#endif
#if FUNC_REC_EN
u8 mpa_is_encoding(void);
int mpa_encode_frame(void);
extern u8 mp3enc_flag;
extern u8 adpcm_enc_flag;
#endif

volatile u8 asr_kick_kws_flag AT(.buf.asrkick);
// static u8 asr_pcm_cache[ASR_PCM_LEN] AT(.ws_asr.test);
// static ring_buf_t asr_pcm AT(.ws_asr.test);

asr_cfg_t asr_cfg = {
    .huart_dump_en      = 0,                //是否通过HUART dump音频数据，使用bluetrum_voice_record工具接收
};

AT(.asr_text.asr_deal)
void asr_pcm_callback(u8 *ptr, u16 len)
{
        printf("[%s]\r\n", __func__);

    asr_alg_process((int16_t *)ptr);
}

AT(.vad_text.kws.proc)
void asr_kws_process(void)
{
        printf("[%s]\r\n", __func__);

    if(asr_kick_kws_flag){
        // ring_buf_get(&asr_pcm, asr_pcm_callback, 640 * 2);
        asr_kick_kws_flag = 0;
    }
}

AT(.com_text.thread.asr)
void asr_kick_start(void)
{
    printf("[%s]\r\n", __func__);

    asr_kick_kws_flag = 1;
}



//extern void kws_load_input(int16_t *audio, int32_t audio_len);

//AT(.com_rodata.bat)
//const char mic_pow_str[] = "%d\n";
///SDADC DMA中断起的AUPCM线程

bool gpio_set = 0;



#define KWS_WAKE_UP_SEC        15
static int32_t kws_wake_tick = 0;

static void kws_sleep(void)
{
    if (kws_wake_tick > 0)
    {
        kws_wake_tick--;
        if (kws_wake_tick == 0)
        {
//            uart_com_write_pkt(0x11);
        }
    }
}

static void kws_refresh(void)
{
    kws_wake_tick = 15*1000/96;
}

static bool auth_succ_flag = false;
volatile bool asr_pause_flag = false;   /* 播报时暂停识别帧(sound_res.c 控制) */

/* 错误/不清指令检测: 唤醒态 + 有语音 + 没认出来(top<0 或 prob不过该词阈值)
 *   能量>ERR_VOICE_THR    → 清晰说话但词表外/词模糊 → "暂不支持此指令"(resp41)
 *   能量>ERR_VOICE_THR_LOW → 有语音但不清晰/环境吵 → "没听清请再说一遍"(resp42)
 * 阈值调高防误触; 防抖连续帧才播一次 */
#define ERR_VOICE_THR        4000        /* 清晰说话阈值, 调高降误触 */
#define ERR_VOICE_THR_LOW    2000        /* 有语音(但不清)阈值, 调高降误触 */
#define ERR_VOICE_FRAMES     60          /* 连续有语音无匹配帧数(~3s), 等词说完再判, 命令不被抢先 */
static int  err_voice_cnt = 0;
static bool err_voice_played = false;
static int  unclear_cnt = 0;
static bool unclear_played = false;

/* 计算音频峰值能量(峰值绝对值), 判断是否有人说话 */
static int audio_peak(u8 *ptr, u32 samples)
{
    int16_t *p = (int16_t *)ptr;
    int max_abs = 0;
    u32 i;
    for (i = 0; i < samples; i++) {
        int v = p[i];
        if (v < 0) {
            v = -v;
        }
        if (v > max_abs) {
            max_abs = v;
        }
    }
    return max_abs;
}

AT(.com_text.vad.proc)
void asr_sdadc_process(u8 *ptr, u32 samples, int ch_mode)
{
    if (!auth_succ_flag)
    {
        return;
    }
    if (asr_pause_flag)   /* 播报时跳过识别帧, 不喂KWS(防播报声误识别), 引擎不重启 */
    {
        return;
    }

//    printf("samples:%d\n", samples);
//    putchar('p');
#if HUART_DEUMP_EN
    dump_putbuf(ptr, samples, 0);
    dump_putbuf(&ptr[samples], samples, 0);
#endif

    kws_sleep();

    kws_load_input((int16_t*)ptr, 128*3);

    int16_t top=-1,prob;
    kws_classify(&top,&prob);
    /* 调试：无论是否识别都打印 */
    if(top >= 0) {
        my_printf("DBG: top=%d, prob=%d, thr=%d\n", top, prob, WW_TRG_MODE[top]);
        if ((u32)prob < (u32)WW_TRG_MODE[top]) {
            /* 未命中候选帧: 遥测帧发PA10(逻辑分析仪调参用); 命中帧走uart_cmd_send, 同帧互斥 */
            uart_debug_frame(top, prob, WW_TRG_MODE[top]);
        }
    }
    if((top>=0) && (prob >= WW_TRG_MODE[top]))
    {
        my_printf("Cmd:%d\nOutput: %s (%d)\n", top, kw_string2[top], prob);
        /* 唤醒机制：唤醒词组合判定+命令门控。
         * 未唤醒时命令词只打印不动作;唤醒成功后命令才生效 */
        if (wake_ctrl_process((int16_t)top)) {
            /* 命令生效(唤醒态): 串口发命令给主控(ID=top) + 请求播对应回应语 + LED
             * 词表 top 1~36 → 回应语 resp 3~38 (顺序对齐) */
            uart_cmd_send(top);
            set_timer_pwm_duty(0, 100);   /* 诊断灯: 蓝灯亮 = 识别到命令 */
            sound_request((int)top + 2);
            led_ctrl((uint16_t)top);
        }
        /* 唤醒词(0=你好小柯): 串口发唤醒(ID=0) + 请求播唤醒回应(异步) */
        if (top == 0) {
            uart_cmd_send(0);
            sound_request(RESP_ID_WAKE);
        }
    }

    /* 错误/不清指令检测: 唤醒态 + 有语音 + "没认出来"
     *   没认出来 = top<0(无匹配) 或 top>=0但prob不过该词阈值(匹配到但不够自信, 如词表外的
     *              "九档/一小时"会被最近的模板认走但分数可能不过线)
     *   高能量→"暂不支持此指令"(resp41); 中等能量(说话不清)→"没听清请再说一遍"(resp42)
     * 防抖: 连续 ERR_VOICE_FRAMES 帧才播一次, 识别到命令/唤醒词后重置 */
    if (wake_ctrl_is_on()) {
        if ((top < 0) || (prob < WW_TRG_MODE[top])) {
            int en = audio_peak(ptr, samples);
            if (en > ERR_VOICE_THR) {
                if ((++err_voice_cnt >= ERR_VOICE_FRAMES) && !err_voice_played) {
                    sound_request(41);   /* resp41 = "暂不支持此指令" */
                    err_voice_played = true;
                }
            } else if (en > ERR_VOICE_THR_LOW) {
                if ((++unclear_cnt >= ERR_VOICE_FRAMES) && !unclear_played) {
                    sound_request(42);   /* resp42 = "没听清请再说一遍" */
                    unclear_played = true;
                }
            } else {
                err_voice_cnt = 0;       /* 安静, 不播 */
                unclear_cnt = 0;
            }
        } else {
            err_voice_cnt = 0;
            unclear_cnt = 0;
            err_voice_played = false;
            unclear_played = false;
        }
    }

    /* 唤醒状态机：红灯闪烁/10秒超时 */
    wake_ctrl_loop();
    led_loop();


//    uint8_t test_read[5];
//    if (uart_com_read(test_read, sizeof(test_read)) == 5)
//    {
//        my_print_r(test_read, sizeof(test_read));
//    }
}

int8_t raw_buffer[128*16*2] __attribute__((section(".kws.ram_buf"))) = {0};
int32_t raw_buffer_size = 128*16*2;
int8_t med_buffer[512*4] __attribute__((section(".kws.ram_buf"))) = {0};
int32_t med_buffer_size = 512*4;
///ASR启动
void asr_start(void)
{
//    int auth_ret = check_auth();

//    my_printf("auth%d\n", auth_ret);

    int32_t buf_size, buf_size2;
    getBufferSize(&buf_size, &buf_size2);
    printf("\r\nbuf_size:%d buf_size2:%d\r\n", buf_size,  buf_size2);

    int ret = kws_init(raw_buffer, raw_buffer_size, med_buffer, med_buffer_size);
    printf("\r\nkws_init ret:%d\r\n",  ret);
    if (ret == 0)
    {
        auth_succ_flag = true;
    }

    printf("kws version:%s\n", kws_get_version());
    /* ============ set_kw_offset 逐词得分补偿(排名层调参) ============
     * 作用: 给某词识别得分加/减分, 决定"几个近音词竞争时谁赢"
     *   正值 = 更容易抢过邻居 | 负值 = 让位
     * 与词表.c的 WW_TRG_MODE(触发阈值)是两个独立旋钮:
     *   阈值 = 认出来后"执不执行"(管误触) | offset = 竞争时"被认成谁"(管串扰)
     * 调参口诀: 漏识→该词offset调大或阈值调小 | 误触→该词阈值调大
     *           串扰(说X认成Y)→X的offset调大 + Y的offset调小
     * 近音对(火力2↔6档/预约2↔6、4↔10小时)是跷跷板: 一边升另一边必须降, 切勿同升! */
    set_kw_offset(0, 200);    /* 唤醒词: 保证唤醒可靠(勿动) */
    set_kw_offset(22, 900);   /* 预约二小时(六小时已降至500, 900够用) */
    set_kw_offset(25, 200);   /* 预约五小时: 用户要求高一滴滴(2026-09-01) */
    set_kw_offset(24, 200);   /* 预约四小时: 用户要求调高一点点(2026-09-01) */
    set_kw_offset(26, 500);   /* 预约六小时 */
    set_kw_offset(30, 500);   /* 预约十小时 */
    set_kw_offset(32, 500);   /* 预约十二小时 */
    set_kw_offset(35, 500);   /* 打开保温 */
    /* 火力二档(7)跷跷板实测: +700说二仍→六 | +2500说一/五/六全被二抢。
     * 平衡区在中间, 当前取1600(自调指南):
     *   说二还→六      → 7 每次+200往上试(上限2500)
     *   说一/五/六→二  → 7 每次-200往下试(下限1000)
     * 7与3的差值就是"二档压制力"; 六档也闹就同步降3 */
    set_kw_offset(7, 1600);   /* 火力调整为二档(平衡试探值, 可自调1000~2500) */
    set_kw_offset(3, 400);    /* 火力调整为六档 */
    /* 火力五档(4): 二档+1600后抢五的音频(说五→二/六)。实测五的音频天然比二高700~1600分,
     * 给五+900压回二档; 副作用风险: 五可能反抢四/六的音频, 复测连着测四/六 */
    set_kw_offset(4, 900);    /* 火力调整为五档 */
    wake_ctrl_init();
    led_ctrl_init();
//    uart_com_init(HUART_BAUD);

//#define TEST_GPIO          4            // PA10
//    GPIOAFEN &= ~BIT(TEST_GPIO);
//    GPIOADE  |=  BIT(TEST_GPIO);
//    GPIOADIR &= ~BIT(TEST_GPIO);
//    GPIOASET = BIT(TEST_GPIO);
//    GPIOACLR = BIT(TEST_GPIO);
}

///ASR停止
void asr_stop(void)
{

}

///ASR初始化
void asr_init(asr_cfg_t* cfg)
{

}

u32 asr_get_sdadc_dma_addr(void)
{
    return 0;
}

u32 asr_get_sdadc_dma_len(void)
{
    return 0;
}

void bsp_asr_restart(void)
{
    /* 播报后重置KWS引擎: 采音中断~1.7s后识别需重建(流式引擎断帧会失效)。
     * 只重置识别引擎, 不动 wake_ctrl(保持唤醒态, 支持唤醒后连续命令)/LED。 */
    kws_init(raw_buffer, raw_buffer_size, med_buffer, med_buffer_size);
    printf("bsp_asr_restart: kws re-init done\n");
}

void bsp_asr_start(void)
{
    asr_start();
}

void bsp_asr_stop(void)
{

}

void bsp_asr_init(void)
{

}
#endif // ASR_RECOG_EN
