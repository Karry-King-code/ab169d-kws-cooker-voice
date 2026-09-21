#include "include.h"
#include "key_ctrl.h"
#include "led_ctrl.h"

/* ============================================================
 * 触摸按键状态机（帧周期约48ms，SDADC回调经 led_loop 调用）
 *   双击 : 电源开关（开=白光100%+唤醒可用 / 关=全灭+唤醒禁用+按键无效）
 *   短按 : 循环切色（关灯→白光L1→黄光L2→白黄L1+L2→关灯，继承亮度；关灯档≠关机）
 *   长按 : 连续调光（按住1.2s触发；100↔20 每帧2%约2秒走完；到边界闪烁；
 *          再次长按反向；松手停当前亮度）
 * 接线: 一根线插在 PA4/5 (pin5), 线的另一端悬空, 碰 VBAT=按下(高电平),
 *       悬空/碰GND=未按(内部强下拉)。
 * (2026-08-14: 触摸按键换 PA4/5——蓝灯已挪到 PA10/11(pin1), PA4/5 空出) */

#define KEY_IO_MASK         (BIT(IO_PA4 - 1) | BIT(IO_PA5 - 1))   /* PA4/5 同脚(pin5) */

#define KEY_DEBOUNCE_CNT    2        /* 连续2帧同电平防抖 */

#define KEY_LONG_FRAMES     25       /* 长按阈值 ~1.2s (用户: 长按1.2秒触发调光) */
#define KEY_LONG_RELEASE_CNT 5       /* LONG状态松开防抖: 连续5帧低电平才退出 */
#define KEY_DOUBLE_FRAMES   7        /* 双击窗口 ~336ms */
#define KEY_SHORT_PRESS_FRAMES 12    /* 短按/长按候选分界 ~600ms */
#define KEY_RELEASE_FAST_CNT 3       /* 短按候选松开防抖 ~150ms */
#define KEY_RELEASE_SLOW_CNT 8       /* 长按候选松开防抖 ~400ms (滤掉手摸抖动) */

typedef enum {
    KEY_ST_IDLE = 0,
    KEY_ST_PRESSED,       /* 按下中(可转长按/松开判短按) */
    KEY_ST_LONG,          /* 长按调光中 */
    KEY_ST_DOUBLE_WAIT,   /* 松开后等待双击窗口 */
} key_state_t;

static key_state_t key_st = KEY_ST_IDLE;
static u8  key_raw = 0;              /* 原始电平 0/1 */
static u8  key_deb_cnt = 0;          /* 防抖连续帧 */
static bool key_pressed = false;     /* 防抖后电平: true=按下 */
static u16 press_cnt = 0;            /* 按住帧数 */
static u8  release_cnt = 0;          /* 松开防抖连续帧(抖动不打断状态机) */
static u16 double_wait_cnt = 0;      /* 双击窗口帧计数 */
static bool second_press = false;    /* 双击的第二击(松开不再触发) */
static bool long_blocked = false;    /* 本次长按被禁止(关机/关灯档) */
static bool dim_dir_up = true;       /* 调光方向: true=变亮 */
static bool dim_boundary = false;    /* 已到边界, 本次长按停止调光 */
static bool long_ever = false;       /* 是否发生过长按(用于方向翻转) */

static bool key_io_read(void)
{
    return (GPIOA & KEY_IO_MASK) != 0;
}

static void key_io_init(void)
{
    /* 纯GPIO数字输入 + 内部强下拉(10K) + 关闭所有上拉:
     * 不摸/碰GND = 低(未按), VBAT模拟触摸高电平 = 按下。
     * 关键: 必须清 GPIOAPU/GPIOAPU500K, 否则复位后上拉残留会让
     * 悬空的 PA4/5 读高, 碰GND经人体电容就误触发("碰GND有效果") */
    GPIOAFEN    &= ~KEY_IO_MASK;   /* 关复用 */
    GPIOADE     |=  KEY_IO_MASK;   /* 数字使能 */
    GPIOAPU     &= ~KEY_IO_MASK;   /* 关普通上拉(10K) */
    GPIOAPU500K &= ~KEY_IO_MASK;   /* 关弱上拉(500K) */
    GPIOAPD500K &= ~KEY_IO_MASK;   /* 关弱下拉(500K), 只用强下拉 */
    GPIOAPD     |=  KEY_IO_MASK;   /* 开强下拉(10K) */
    GPIOADIR    |=  KEY_IO_MASK;   /* 输入方向 */
}

void key_ctrl_init(void)
{
    key_st = KEY_ST_IDLE;
    key_raw = 0;
    key_deb_cnt = 0;
    key_pressed = false;
    press_cnt = 0;
    release_cnt = 0;
    double_wait_cnt = 0;
    second_press = false;
    long_blocked = false;
    dim_dir_up = true;
    dim_boundary = false;
    long_ever = false;
    key_io_init();
}

/* 双击: 电源开关 */
static void key_action_double(void)
{
    my_printf("KEY: double\n");
    led_key_power_toggle();
}

/* 短按: 循环切色 (打印按了多少帧, 定位阈值是否错位) */
static void key_action_short(void)
{
    my_printf("KEY: short press=%u\n", (u32)press_cnt);
    led_key_cycle_color();
}

/* 进入长按调光: 每次新长按翻转方向(再次长按反向);
 * 已到边界自动反向(首次100%时长按自动变暗) */
static void key_long_start(void)
{
    if (long_ever) {
        dim_dir_up = !dim_dir_up;
    }
    long_ever = true;

    if (dim_dir_up && led_key_dim_at_max()) {
        dim_dir_up = false;
    } else if (!dim_dir_up && led_key_dim_at_min()) {
        dim_dir_up = true;
    }
    dim_boundary = false;
    my_printf("KEY: long %s\n", dim_dir_up ? "UP" : "DOWN");
}

void key_ctrl_loop(void)
{
    bool raw = key_io_read();
    static bool prev_deb = false;    /* 上一次防抖后电平(调试打印用) */
    static u16 hold_frames = 0;      /* 本次按下已持续帧数(调试打印用) */

    /* 防抖: 连续 KEY_DEBOUNCE_CNT 帧同电平才确认 */
    if (raw != key_raw) {
        key_raw = raw;
        key_deb_cnt = 0;
        my_printf("KEY_IO=%u\n", (u32)raw);   /* 原始电平变化即打印: 定位极性/接线 */
    } else if (key_deb_cnt < KEY_DEBOUNCE_CNT) {
        key_deb_cnt++;
        if (key_deb_cnt >= KEY_DEBOUNCE_CNT) {
            key_pressed = raw;
        }
    }

    /* 调试: 打印按下/松开边沿 + 原始电平 + 按住时长(帧×48ms) */
    if (key_pressed != prev_deb) {
        prev_deb = key_pressed;
        if (key_pressed) {
            hold_frames = 0;
            my_printf("KEY: DOWN raw=%u\n", (u32)key_raw);
        } else {
            my_printf("KEY: UP   hold=%u帧(~%ums)\n",
                      (u32)hold_frames, (u32)(hold_frames * 48));
        }
    }
    if (key_pressed) {
        hold_frames++;
    }

    switch (key_st) {
    case KEY_ST_IDLE:
        if (key_pressed) {
            key_st = KEY_ST_PRESSED;
            press_cnt = 1;
        }
        break;

    case KEY_ST_PRESSED:
        if (key_pressed) {
            release_cnt = 0;
            press_cnt++;
            if ((press_cnt >= KEY_LONG_FRAMES) && !long_blocked) {
                if (led_key_dim_enabled()) {
                    second_press = false;    /* 不再作为双击第二击处理 */
                    key_st = KEY_ST_LONG;
                    key_long_start();
                } else {
                    long_blocked = true;     /* 关机/关灯档长按无反应 */
                }
            }
        } else {
            /* 动态松开防抖: 按下时间短(短按候选)快速响应松手,
             * 按下时间长(长按候选)要求更长稳定低电平才算松开,
             * 避免手摸抖动让"按住"被误判成"松开→短按切色" */
            u8 release_need = (press_cnt < KEY_SHORT_PRESS_FRAMES)
                            ? KEY_RELEASE_FAST_CNT : KEY_RELEASE_SLOW_CNT;
            release_cnt++;
            if (release_cnt >= release_need) {
                release_cnt = 0;
                if (second_press) {
                    second_press = false;    /* 双击第二击松开, 不触发 */
                    key_st = KEY_ST_IDLE;
                } else if (press_cnt < KEY_LONG_FRAMES) {
                    key_st = KEY_ST_DOUBLE_WAIT;
                    double_wait_cnt = 0;
                } else {
                    key_st = KEY_ST_IDLE;
                }
                long_blocked = false;
            }
        }
        break;

    case KEY_ST_DOUBLE_WAIT:
        double_wait_cnt++;
        if (key_pressed) {
            /* 双击窗口内再按下: 判为双击的第二击 */
            second_press = true;
            key_st = KEY_ST_PRESSED;
            press_cnt = 1;
            key_action_double();
        } else if (double_wait_cnt >= KEY_DOUBLE_FRAMES) {
            /* 窗口内无第二击: 判定为短按 */
            key_st = KEY_ST_IDLE;
            key_action_short();
        }
        break;

    case KEY_ST_LONG:
        if (key_pressed) {
            release_cnt = 0;
            if (!dim_boundary) {
                /* 连续平滑调光: 每帧步进2%, 100%↔20%, 到边界闪烁后停 */
                if (led_key_dim_step(dim_dir_up ? 1 : -1) == 0) {
                    dim_boundary = true;
                }
            }
        } else {
            /* 松开防抖: 连续5帧低电平才退出长按, 防止手摸抖动把长按打断
             * (否则长按中 KEY 掉0一帧就退出→重判成双击/短按) */
            release_cnt++;
            if (release_cnt >= KEY_LONG_RELEASE_CNT) {
                release_cnt = 0;
                key_st = KEY_ST_IDLE;          /* 松开: 停在当前档 */
            }
        }
        break;

    default:
        key_st = KEY_ST_IDLE;
        break;
    }
}
