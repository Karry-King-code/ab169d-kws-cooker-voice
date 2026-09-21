#include "include.h"
#include "wake_ctrl.h"
#include "led_ctrl.h"
#include "sound_res.h"   /* RESP_ID_TIMEOUT 等回应语编号 */

extern void sound_request(int resp_id);   /* sound_res.c: 异步播报请求 */

/* ============================================================
 * 唤醒状态机
 *   WAKE_OFF   : 未唤醒,命令词不执行
 *   WAKE_FLASH : 唤醒成功,主灯(L1/L2)同时快闪两下
 *   WAKE_ON    : 唤醒保持,10秒超时退出
 * (2026-08-14 红灯移除: PA10/11 让给触摸按键,
 *  唤醒提示改为主灯蓝绿快闪2下, 效果与红灯一致)
 * ============================================================ */
typedef enum {
    WAKE_OFF = 0,
    WAKE_FLASH,
    WAKE_ON,
} wake_state_t;

static wake_state_t wake_state = WAKE_OFF;
static int16_t state_cnt = 0;        /* 当前状态帧计数(每帧约48ms) */

/* ---------------- 唤醒触发 ---------------- */

static void wakeup_start(void)
{
    if (wake_state == WAKE_OFF) {
        wake_state = WAKE_FLASH;     /* 主灯快闪两下 */
        state_cnt = 0;
    }
    /* WAKE_ON 状态下由调用方刷新计时,不重新闪烁 */
}

/* ---------------- 公共接口 ---------------- */

void wake_ctrl_init(void)
{
    wake_state = WAKE_OFF;
    state_cnt = 0;
}

bool wake_ctrl_process(int16_t top)
{
    /* 电饭煲语音模块: 上电即待唤醒, 无电源门控(原电灯双击关机门控已移除) */

    /* 唤醒词 (0=你好小柯) — 634库仅 index 0 为唤醒词 */
    if (top == 0) {
        /* 唤醒保持中: 保持唤醒,刷新10秒计时,不闪烁 */
        if (wake_state == WAKE_ON) {
            state_cnt = 0;
            return false;
        }
        /* 未唤醒: 直接唤醒 (634 库"你好小柯"为完整词条,无需组合判定) */
        if (wake_state == WAKE_OFF) {
            wakeup_start();
        }
        return false;                /* 唤醒词不执行命令 */
    }

    /* 命令词 (1-36): 仅唤醒状态可执行; 唤醒状态下每识别到一条
     * 指令都刷新10秒计时,连续10秒无任何词才退出唤醒 */
    if (wake_state == WAKE_ON) {
        state_cnt = 0;
        return true;
    }
    return false;
}

void wake_ctrl_loop(void)
{
    switch (wake_state) {
    case WAKE_OFF:
        break;

    case WAKE_FLASH:
        /* 闪烁两下: 亮-灭-亮-灭, 每个半周期 WAKE_FLASH_HALF_FRAMES 帧 */
        if (state_cnt >= (WAKE_FLASH_HALF_FRAMES * 4)) {
            wake_state = WAKE_ON;    /* 闪烁完成,恢复主灯原状态 */
            led_key_wake_restore();
            state_cnt = 0;
        } else {
            int half = state_cnt / WAKE_FLASH_HALF_FRAMES;
            led_key_wake_hint((half % 2) == 0);   /* 亮-灭-亮-灭 */
            state_cnt++;
        }
        break;

    case WAKE_ON:
        /* 10秒保持: 无任何词刷新则超时退出(主灯不受影响) */
        if (state_cnt >= WAKE_HOLD_FRAMES) {
            wake_state = WAKE_OFF;
            state_cnt = 0;
            sound_request(RESP_ID_TIMEOUT);   /* 30秒无指令: 播"我先退下了有事请叫我" */
        } else {
            state_cnt++;
        }
        break;

    default:
        break;
    }
}

void wake_ctrl_shutdown(void)
{
    /* 双击关机时调用: 强制退出唤醒状态 (主灯由 led_key_power_off 全灭) */
    wake_state = WAKE_OFF;
    state_cnt = 0;
}

bool wake_ctrl_is_on(void)
{
    /* 查询是否处于唤醒保持态: 供 bsp_asr 错误指令检测用 */
    return (wake_state == WAKE_ON);
}
