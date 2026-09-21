/* 唤醒回应语 "我在，请问有什么需要" - 16kHz mono 64kbps MP3 (ffmpeg重编码, 音质修复) */


#include "include.h"
#include "bsp_music.h"
#include "sound_res.h"

extern void bsp_asr_restart(void);   /* bsp_asr.c: 播报后重置KWS识别引擎 */
extern void set_timer_pwm_duty(u8 num, u32 duty);   /* 诊断灯: led_yw.c BLUE_CH=0 */
extern volatile bool asr_pause_flag;          /* bsp_asr.c: 播报暂停识别标志 */

/* 上电自动播报开机语"欢迎使用colorman智能产品"
 * res 资源路径: res/mp3/resp01.mp3 (16kHz mono 8kbps, 由 xmaker 打包)
 * 在 main() 中 bsp_sys_init() 后、func_run() 前调用 (参考工程 main.c:31) */
void sound_play_boot_welcome(void)
{
    dac_spr_set(SPR_16000);   /* 播放前设 DAC 采样率 16kHz(匹配 MP3) */
    mp3_res_play(RES_BUF_MP3_RESP01_MP3, RES_LEN_MP3_RESP01_MP3);
}

/* 回应语资源索引: res 打包后每个资源占 0x20 字节(地址+长度两字段),
 * 首项(第1号)偏移 0x38。由 xmaker 生成的 res.h 宏可验证:
 *   resp01 0x38 / resp02 0x58 / ... / resp45 0x5b8 (每项 +0x20)
 * 用偏移公式避免写 45 个宏 case。 */
#define RESP_BUF_OFFSET(id)  (0x11000038u + ((u32)(id) - 1) * 0x20u)

/* 播指定编号的回应语 (1~45 对应 res/mp3/resp01~45.mp3)
 * 命令→回应语 映射: resp_id = top + 2 (词表顺序/Excel顺序/资源顺序三对齐) */
void sound_play_resp(int resp_id)
{
    u32 addr, len;

    if (resp_id < 1 || resp_id > 45) {
        return;
    }
    addr = *(volatile u32 *)RESP_BUF_OFFSET(resp_id);
    len  = *(volatile u32 *)(RESP_BUF_OFFSET(resp_id) + 4);

    asr_pause_flag = true;      /* 播报时暂停识别帧(不重启引擎/不停采音, 防播报声误识别) */
    dac_spr_set(SPR_16000);   /* 播放前设 DAC 采样率 16kHz(匹配 MP3) */
    mp3_res_play(addr, len);
    asr_pause_flag = false;     /* 播完恢复识别 */
    bsp_asr_restart();          /* 重置KWS引擎到干净态: 立即恢复识别(不用积累几秒) */
}

void sound_play_wake_response(void)
{
    /* 唤醒改走res(resp02), 释放内嵌code, 与其他词统一 */
    sound_play_resp(2);
}

/* ================= 异步播报: 中断只请求, 主循环播放 =================
 * 背景: 播报(阻塞~2s)若直接在识别中断(SDADC DMA回调,每48ms)里做,
 *       会长时间占用中断 → DMA缓冲溢出 → 识别永久失效。
 * 方案: 中断里只记录待播ID(sound_request), 主循环里播放(sound_poll)。*/
static volatile int resp_pending = -1;   /* 待播ID: -1=无, 0=唤醒(resp02), 1~45=res资源; 中断写/主循环读 */

void sound_request(int resp_id)
{
    resp_pending = resp_id;   /* 中断上下文: 只记标志, 立即返回, 不阻塞识别 */
}

void sound_poll(void)
{
    int id = resp_pending;

    if (id < 0) {
        return;
    }
    resp_pending = -1;        /* 取走待播ID, 防止重复播放 */

    if (id == RESP_ID_WAKE) {
        sound_play_wake_response();   /* 唤醒回应(resp02) */
    } else if (id >= 1 && id <= 45) {
        sound_play_resp(id);          /* 命令回应(res) */
    }
    set_timer_pwm_duty(0, 0);   /* 诊断灯: 播报完成 → 蓝灯灭 */
}