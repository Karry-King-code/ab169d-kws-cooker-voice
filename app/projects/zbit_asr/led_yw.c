#include "include.h"
#include "led_ctrl.h"
#include "key_ctrl.h"
#include "wake_ctrl.h"
#include "sound_res.h"

// ============================================================
// 旧板（测试板）LED 定义
//   PA10/11 (pin1) → 蓝灯 = 暖色光（原 PA4，2026-08-14 蓝灯挪到
//                  pin1 红灯脚；红灯已拔, PA4/5 让给触摸按键）
//   PA7 (pin4)     → 绿灯 = 白色光
//   暖色+白色 = 自然光
// ============================================================

#define LED_FLASH_TIMES     240

#define LED_BLUE            IO_PA10  // 蓝灯 / 暖色 (pin1, PA10/11同脚)
#define LED_GREEN           IO_PA7   // 绿灯 / 白色 (pin4, PA7/8同脚)

#define BLUE_CH             0        // timer_pwm_init 第1路
#define GREEN_CH            1        // timer_pwm_init 第2路

#define TIMER_FREC          25000

extern void timer_pwm_init(u8 io1, u8 io2, u8 io3, u8 io4, u32 fre, u32 duty);
extern void set_timer_pwm_duty(u8 num, u32 duty);

enum BIIGHTNESS_LEVEL {
    BRIGHTNESS_OFF = 0,
    BRIGHTNESS_LEVEL_1,   // 20%
    BRIGHTNESS_LEVEL_2,   // 40%
    BRIGHTNESS_LEVEL_3,   // 60% 中等亮度
    BRIGHTNESS_LEVEL_4,   // 80%
    BRIGHTNESS_LEVEL_5,   // 100% 最高
    BRIGHTNESS_MAX
};

#if (LED_LIGHT_MODE == LED_LOW_LEVEL_LIGHT)
static const int brightness_level_arr[BRIGHTNESS_MAX] = {100, 80, 60, 40, 20, 0};
#else
static const int brightness_level_arr[BRIGHTNESS_MAX] = {0, 20, 40, 60, 80, 100};
#endif

enum COLOR_INDEX {
    COLOR_NONE = 0,
    COLOR_WARM,           // 暖色光 = 蓝灯
    COLOR_COOL,           // 白色光 = 绿灯
    COLOR_NATURAL,        // 自然光 = 蓝+绿
};

typedef struct {
    bool on;
    uint8_t color_index;
    uint8_t brightness_level;
    int brightness_val;
    bool music_beat_mode;        /* 音乐律动模式 */
} led_handle_t;

typedef enum {
    LED_TIMER_DEFAULT = 0,
    LED_TIMER_10MIN,
    LED_TIMER_30MIN,
    LED_TIMER_60S,          /* 睡眠模式: 60s 自动关灯 */
} led_timer_status;

static led_timer_status led_status = LED_TIMER_DEFAULT;
volatile bool time_set_flag = false;
static led_handle_t led_handle = {0};
static bool power_on = false;              /* 电源状态: 双击开机/关机 */

int led_tick = 0;

/* ============ 电饭煲烹饪流程状态机 ============
 * 识别"打开煮饭/煮粥/煮面/蒸煮" → 10s 倒计时 → 播"已结束,进入保温状态"(resp44) → 进入保温
 * 保温 5s 倒计时 → 播"保温已结束,欢迎下次使用"(resp45)
 * 识别"打开保温" → 直接进入保温(5s)
 * 倒计时可打断: 识别到其他命令词 → 取消流程, 命令照常执行
 * 定时用 SDK 毫秒 tick(tick_get/tick_check_expire), led_loop 主循环轮询,
 * 不依赖 led_tick(它被 time_set_flag 门控, 非定时状态冻结) */
typedef enum {
    COOK_IDLE = 0,
    COOK_COOKING,       /* 烹饪倒计时(煮饭/煮粥/煮面/蒸煮) */
    COOK_WARMING,       /* 保温倒计时 */
} cook_state_t;

#define COOK_PHASE_MS    10000   /* 烹饪→保温: 10s */
#define WARM_PHASE_MS    5000    /* 保温→结束: 5s */

static cook_state_t cook_state = COOK_IDLE;
static u32 cook_phase_tick = 0;
static u16 beat_step = 0;          /* 音乐律动步进计数（led_loop 每调用一次推进一步） */

static int duty_half(int val)
{
    return val / 2;
}

static void pwm_apply_color(led_handle_t *handle, int duty)
{
    int off = brightness_level_arr[BRIGHTNESS_OFF];

    if (!handle->on || duty <= 0) {
        set_timer_pwm_duty(BLUE_CH, off);
        set_timer_pwm_duty(GREEN_CH, off);
        return;
    }

    if (handle->color_index == COLOR_NATURAL) {
        int half = duty_half(duty);
        set_timer_pwm_duty(BLUE_CH, half);
        set_timer_pwm_duty(GREEN_CH, half);
    } else if (handle->color_index == COLOR_WARM) {
        set_timer_pwm_duty(BLUE_CH, duty);
        set_timer_pwm_duty(GREEN_CH, off);
    } else if (handle->color_index == COLOR_COOL) {
        set_timer_pwm_duty(BLUE_CH, off);
        set_timer_pwm_duty(GREEN_CH, duty);
    } else {
        set_timer_pwm_duty(BLUE_CH, off);
        set_timer_pwm_duty(GREEN_CH, off);
    }
}

static int led_update(led_handle_t *handle)
{
    if ((handle == NULL) || (handle->brightness_level >= BRIGHTNESS_MAX)) {
        return 1;
    }

    handle->brightness_val = brightness_level_arr[handle->brightness_level];
    if (handle->on) {
        pwm_apply_color(handle, handle->brightness_val);
    } else {
        pwm_apply_color(handle, 0);
    }
    return 0;
}

/* 直接按当前连续亮度值刷新PWM(不重算档位)。
 * 供触摸按键连续调光/切色继承连续亮度用, 避免 led_update 把
 * 连续调光后的 brightness_val 重新算回档位值 */
static int led_apply_current_val(led_handle_t *handle)
{
    if (handle == NULL) {
        return 1;
    }
    if (handle->on) {
        pwm_apply_color(handle, handle->brightness_val);
    } else {
        pwm_apply_color(handle, 0);
    }
    return 0;
}

/* 上电首次开灯默认：白光(绿通道) + 最大亮度(5档=100%)。
 * 之后开灯恢复关灯前状态(颜色+亮度)，不重置 */
static void led_apply_poweron_default(led_handle_t *handle)
{
    handle->color_index = COLOR_COOL;
    handle->brightness_level = BRIGHTNESS_LEVEL_5;
    handle->brightness_val = brightness_level_arr[BRIGHTNESS_LEVEL_5];
}

static int led_yw_on(led_handle_t *handle)
{
    if (handle == NULL) {
        return 1;
    }
    /* 开灯恢复关灯前状态: led_yw_off 只清 on 标志,颜色/亮度保留在
     * handle 中; 上电首次的状态由 led_ctrl_init 预设(白光+100%) */
    handle->on = true;
    return led_update(handle);
}

static int led_yw_off(led_handle_t *handle)
{
    if (handle == NULL) {
        return 1;
    }
    handle->on = false;
    handle->music_beat_mode = false;
    return led_update(handle);
}

static int led_change_color(led_handle_t *handle)
{
    if ((handle == NULL) || (!handle->on)) {
        return 1;
    }

    if (handle->color_index == COLOR_NATURAL) {
        handle->color_index = COLOR_WARM;
    } else if (handle->color_index == COLOR_WARM) {
        handle->color_index = COLOR_COOL;
    } else {
        handle->color_index = COLOR_NATURAL;
    }
    return led_update(handle);
}

static int led_set_color(led_handle_t *handle, uint8_t color)
{
    if ((handle == NULL) || (!handle->on)) {
        return 1;
    }
    handle->color_index = color;
    return led_update(handle);
}

static int led_set_level(led_handle_t *handle, uint8_t level)
{
    if ((handle == NULL) || (!handle->on)) {
        return 1;
    }
    if (level < BRIGHTNESS_LEVEL_1) {
        level = BRIGHTNESS_LEVEL_1;
    }
    if (level > BRIGHTNESS_LEVEL_5) {
        level = BRIGHTNESS_LEVEL_5;
    }
    handle->brightness_level = level;
    return led_update(handle);
}

static void led_limit_flash(led_handle_t *handle)
{
    int cur = handle->brightness_val;
    delay_ms(LED_FLASH_TIMES);
    pwm_apply_color(handle, 0);
    delay_ms(LED_FLASH_TIMES);
    pwm_apply_color(handle, cur);
}

static int led_dec_bright(led_handle_t *handle)
{
    if (!handle->on) {
        return 1;
    }

    if (handle->brightness_level > BRIGHTNESS_LEVEL_1) {
        handle->brightness_level--;
        return led_update(handle);
    }

    handle->brightness_level = BRIGHTNESS_LEVEL_1;
    handle->brightness_val = brightness_level_arr[BRIGHTNESS_LEVEL_1];
    led_limit_flash(handle);
    return 0;
}

static int led_inc_bright(led_handle_t *handle)
{
    if (!handle->on) {
        return 1;
    }

    if (handle->brightness_level < BRIGHTNESS_LEVEL_5) {
        handle->brightness_level++;
        return led_update(handle);
    }

    handle->brightness_level = BRIGHTNESS_LEVEL_5;
    handle->brightness_val = brightness_level_arr[BRIGHTNESS_LEVEL_5];
    led_limit_flash(handle);
    return 0;
}

static int led_warn_flash(led_handle_t *handle, int flash_num)
{
    int i;
    int cur;

    if (!handle->on) {
        return 1;
    }

    cur = handle->brightness_val;
    for (i = 0; i < flash_num; i++) {
        delay_ms(LED_FLASH_TIMES);
        pwm_apply_color(handle, 0);
        delay_ms(LED_FLASH_TIMES);
        pwm_apply_color(handle, cur);
    }
    return 0;
}

static void led_clear_timer(void)
{
    led_status = LED_TIMER_DEFAULT;
    time_set_flag = false;
    led_tick = 0;
}

void led_ctrl_init(void)
{
    /* 保留开灯默认参数(唤醒后"打开灯光"用),但初始状态为灭:
     * 烧录上电后蓝绿灯全灭, 双击触摸按键开机后才能通过命令点亮 */
    power_on = false;    /* 默认关机: 双击触摸按键开机(白光+100%) */
    led_apply_poweron_default(&led_handle);
    led_handle.on = false;
    led_tick = 0;
    led_clear_timer();

    /* io3/io4=0：跳过第3/4路 PWM 映射 */
    timer_pwm_init(LED_BLUE, LED_GREEN, 0, 0, TIMER_FREC, led_handle.brightness_val);

    GPIOADRV |= BIT(LED_BLUE - 1);
    GPIOADRV |= BIT(LED_GREEN - 1);

    led_update(&led_handle);
    my_printf("led_init: BLUE=PA10 GREEN=PA7\n");

    /* 触摸按键初始化(PA4/5 输入+强下拉):
     * 蓝灯已挪到 PA10/11, PA4/5 让给触摸按键(2026-08-14) */
    key_ctrl_init();
}

/* 识别到命令时更新烹饪流程状态 (可打断: 任何命令先取消, 再按新命令重新起) */
static void cook_flow_cmd(uint16_t cmd)
{
    switch (cmd) {
    case 10:  /* 打开煮饭 */
    case 12:  /* 打开煮粥 */
    case 14:  /* 打开煮面 */
    case 16:  /* 打开蒸煮 */
        cook_state = COOK_COOKING;
        cook_phase_tick = tick_get();
        break;
    case 35:  /* 打开保温: 直接进入保温 */
        cook_state = COOK_WARMING;
        cook_phase_tick = tick_get();
        break;
    default:  /* 其他命令(含关闭煮饭/关闭保温): 取消流程 */
        cook_state = COOK_IDLE;
        break;
    }
}

/* 主循环轮询: 倒计时到点播报 (led_loop 每帧调用, 不受 LED on 门控) */
static void cook_flow_process(void)
{
    if (cook_state == COOK_IDLE) {
        return;
    }
    if ((cook_state == COOK_COOKING) && tick_check_expire(cook_phase_tick, COOK_PHASE_MS)) {
        sound_request(RESP_ID_COOK_DONE);   /* resp44: 已结束,进入保温状态 */
        cook_state = COOK_WARMING;
        cook_phase_tick = tick_get();
    } else if ((cook_state == COOK_WARMING) && tick_check_expire(cook_phase_tick, WARM_PHASE_MS)) {
        sound_request(RESP_ID_WARM_DONE);   /* resp45: 保温已结束,欢迎下次使用 */
        cook_state = COOK_IDLE;
    }
}

int led_ctrl(uint16_t cmd)
{
    int retval = 0;

    /* 611 库 kw_string2[] 索引 (27条)：
     * 0=你好小镜(唤醒词) 1=小镜小镜(唤醒词)
     * 2=我回来了 3=打开灯光 4=我出去了 5=关闭灯光
     * 6=白光灯 7=白色灯光 8=打开白光
     * 9=黄光灯 10=黄色灯光 11=打开黄光
     * 12=中性灯 13=中性光 14=打开中性光
     * 15=暗一点 16=调暗一点 17=亮一点 18=调亮一点
     * 19=最大亮度 20=灯光最亮 21=最小亮度 22=灯光最暗
     * 23=睡觉模式 24=睡眠模式 25=定时模式 26=开始定时
     */
    /* 本轮实现以下 11 组功能；其余词条识别到但暂不动作（default 忽略），
     * 后续按用户要求逐步添加 */

    /* 睡眠(60s)/定时(30min)倒计时内：其他命令词(除23/24睡眠、25/26定时命令外)
     * 取消自动关灯定时, 命令本身照常执行——重新说定时命令才重新计时。
     * 唤醒词(0/1)不会流到此函数,天然不影响定时 */
    if ((led_status == LED_TIMER_60S) || (led_status == LED_TIMER_30MIN)) {
        if ((cmd != 23) && (cmd != 24) && (cmd != 25) && (cmd != 26)) {
            led_clear_timer();
        }
    }

    switch (cmd) {
    case 2:   /* 我回来了 */
    case 3:   /* 打开灯光 */
        retval = led_yw_on(&led_handle);
        break;

    case 4:   /* 我出去了 */
    case 5:   /* 关闭灯光 */
        retval = led_yw_off(&led_handle);
        led_clear_timer();
        break;

    case 6:   /* 白光灯 */
    case 7:   /* 白色灯光 */
    case 8:   /* 打开白光 */
        if (!led_handle.on) {
            led_yw_on(&led_handle);          /* 灭灯: 先开灯(恢复上次亮度)再切色 */
        }
        retval = led_set_color(&led_handle, COLOR_COOL);   /* 切白光=绿通道,亮度不变 */
        break;

    case 9:   /* 黄光灯 */
    case 10:  /* 黄色灯光 */
    case 11:  /* 打开黄光 */
        if (!led_handle.on) {
            led_yw_on(&led_handle);          /* 灭灯: 先开灯(恢复上次亮度)再切色 */
        }
        retval = led_set_color(&led_handle, COLOR_WARM);   /* 切黄光=蓝通道,亮度不变 */
        break;

    case 12:  /* 中性灯 */
    case 13:  /* 中性光 */
    case 14:  /* 打开中性光 */
        if (!led_handle.on) {
            led_yw_on(&led_handle);          /* 灭灯: 先开灯(恢复上次亮度)再切色 */
        }
        retval = led_set_color(&led_handle, COLOR_NATURAL); /* 切中性光=蓝绿,亮度不变 */
        break;

    case 15:  /* 暗一点 */
    case 16:  /* 调暗一点 */
        retval = led_dec_bright(&led_handle);
        break;

    case 17:  /* 亮一点 */
    case 18:  /* 调亮一点 */
        retval = led_inc_bright(&led_handle);
        break;

    case 19:  /* 最大亮度 */
    case 20:  /* 灯光最亮（与最大亮度相同效果） */
        if (led_handle.on && led_handle.brightness_level >= BRIGHTNESS_LEVEL_5) {
            led_limit_flash(&led_handle);
        } else {
            retval = led_set_level(&led_handle, BRIGHTNESS_LEVEL_5);
        }
        break;
    case 21:  /* 最小亮度 */
    case 22:  /* 灯光最暗（与最小亮度相同效果） */
        if (led_handle.on && led_handle.brightness_level <= BRIGHTNESS_LEVEL_1) {
            led_limit_flash(&led_handle);
        } else {
            retval = led_set_level(&led_handle, BRIGHTNESS_LEVEL_1);
        }
        break;

    case 23:  /* 睡觉模式 */
    case 24:  /* 睡眠模式 */
        /* 睡眠模式: 绿光(同"打开黄光"=COLOR_WARM/PA4) + 20%亮度, 60s后自动关灯。
         * 仅唤醒状态下可执行(命令门控在 bsp_asr.c);
         * 睡眠倒计时内听到其他命令词 → 取消定时且命令照常执行, 需重说本命令重新计时 */
        if (!led_handle.on) {
            led_yw_on(&led_handle);              /* 灭灯时先开灯(恢复上次状态) */
        }
        led_set_color(&led_handle, COLOR_WARM);
        led_set_level(&led_handle, BRIGHTNESS_LEVEL_1);
        led_status = LED_TIMER_60S;
        time_set_flag = true;
        led_tick = 0;
        led_limit_flash(&led_handle);    /* 闪烁1下: 提示已听进去并进入睡眠 */
        break;

    case 25:  /* 定时模式 */
    case 26:  /* 开始定时 */
        /* 定时模式: 保持当前灯光状态不变, 30分钟后自动关灯。
         * 仅亮灯状态可执行(灭灯无反应);
         * 倒计时内其他命令词 → 取消定时且命令照常执行, 重说本命令重新计30分钟 */
        if (!led_handle.on) {
            break;                       /* 灭灯: 无反应 */
        }
        led_status = LED_TIMER_30MIN;
        time_set_flag = true;
        led_tick = 0;
        led_limit_flash(&led_handle);    /* 闪烁1下: 提示已听进去并进入定时 */
        break;

    /* 唤醒词(0/1)：识别到但不动作 */
    default:
        break;
    }

    /* 烹饪流程: 命令更新状态(可打断) */
    cook_flow_cmd(cmd);

    return retval;
}

#if 0
/* ============================================================
 * 以下为 466 库（旧 35 词）时期的 led_ctrl() 原始 case 表，
 * 2026-08-12 因切换 548 库(28词)索引重写，此表已废弃。
 * 保留供回溯参考，不参与编译。
 * ============================================================ */
int led_ctrl_466_old(uint16_t cmd)
{
    int retval = 0;

    /* 466 库 kw_string2[] 索引 (35条)：
     * 0=小小(忽略) 1=开灯 2=打开电灯 3=打开小夜灯 4=回来了
     * 5=关灯 6=关闭电灯 7=关闭小夜灯 8=出去了 9=睡觉了
     * 10=变颜色 11=换颜色
     * 12=最大亮度 13=最小亮度 14=中等亮度
     * 15=设置定时 16=定时十分钟 17=定时半小时 18=定时一小时 19=取消定时
     * 20=音乐模式 21=音乐律动 22=七彩变色 23=呼吸变色
     * 24=白色 25=红色 26=蓝色 27=绿色 28=紫色 29=青色 30=黄色
     * 31=亮一点 32=暗一点 33=瓜灯 34=光灯
     */
    /* 任何非"音乐律动"命令都先退出律动模式（关灯/开灯/调色等立即生效，
     * 不需要新增中断——KWS 命令本身就在 SDADC 回调里即时分发） */
    if (cmd != 21) {
        led_handle.music_beat_mode = false;
    }

    switch (cmd) {
    case 1:   /* 开灯 */
    case 2:   /* 打开电灯 */
    case 3:   /* 打开小夜灯 */
    case 4:   /* 回来了 */
        retval = led_yw_on(&led_handle);
        break;

    case 5:   /* 关灯 */
    case 6:   /* 关闭电灯 */
    case 7:   /* 关闭小夜灯 */
    case 8:   /* 出去了 */
    case 9:   /* 睡觉了 */
    case 33:  /* 瓜灯 */
    case 34:  /* 光灯 */
        retval = led_yw_off(&led_handle);
        led_clear_timer();
        break;

    case 10:  /* 变颜色 */
    case 11:  /* 换颜色 */
    case 22:  /* 七彩变色 */
    case 23:  /* 呼吸变色 */
        retval = led_change_color(&led_handle);
        break;

    case 31:  /* 亮一点 */
        retval = led_inc_bright(&led_handle);
        break;
    case 32:  /* 暗一点 */
        retval = led_dec_bright(&led_handle);
        break;

    case 24:  /* 白色 = 冷色 */
        retval = led_set_color(&led_handle, COLOR_COOL);
        break;
    case 26:  /* 蓝色 = 暖色 */
    case 30:  /* 黄色 = 暖色 */
        retval = led_set_color(&led_handle, COLOR_WARM);
        break;
    case 14:  /* 中等亮度 = 3档 */
        retval = led_set_level(&led_handle, BRIGHTNESS_LEVEL_3);
        break;

    case 12:  /* 最大亮度 = 5档 */
        if (led_handle.on && led_handle.brightness_level >= BRIGHTNESS_LEVEL_5) {
            led_limit_flash(&led_handle);
        } else {
            retval = led_set_level(&led_handle, BRIGHTNESS_LEVEL_5);
        }
        break;
    case 13:  /* 最小亮度 = 1档 */
        if (led_handle.on && led_handle.brightness_level <= BRIGHTNESS_LEVEL_1) {
            led_limit_flash(&led_handle);
        } else {
            retval = led_set_level(&led_handle, BRIGHTNESS_LEVEL_1);
        }
        break;

    case 16:  /* 定时十分钟 */
        led_status = LED_TIMER_10MIN;
        time_set_flag = true;
        led_tick = 0;
        led_warn_flash(&led_handle, 1);
        break;
    case 17:  /* 定时半小时 */
        led_status = LED_TIMER_30MIN;
        time_set_flag = true;
        led_tick = 0;
        led_warn_flash(&led_handle, 2);
        break;
    case 18:  /* 定时一小时 (复用30分钟定时器) */
    case 15:  /* 设置定时 (复用30分钟定时器) */
        led_status = LED_TIMER_30MIN;
        time_set_flag = true;
        led_tick = 0;
        led_warn_flash(&led_handle, 2);
        break;

    case 21:  /* "音乐律动" → 进入律动模式 */
        led_handle.music_beat_mode = true;
        led_handle.on = true;
        beat_step = 0;                      /* 从头开始：先蓝灯 */
        break;

    case 19:  /* 取消定时 */
        led_clear_timer();
        led_warn_flash(&led_handle, 1);
        break;

    /* 暂不实现：0=小小(疑似) 20=音乐模式
     * 25=红色 27=绿色 28=紫色 29=青色
     */
    default:
        break;
    }

    return retval;
}
#endif

/* ============================================================
 * 触摸按键 LED 接口 (key_ctrl 调用)
 * ============================================================ */

void led_key_power_on(void)
{
    /* 双击开机: 固定白光+100%(每次开机重置, 不记忆上次亮度) */
    power_on = true;
    led_apply_poweron_default(&led_handle);
    led_handle.on = true;
    led_update(&led_handle);
    my_printf("KEY: power ON (white 100%%)\n");
}

void led_key_power_off(void)
{
    /* 双击关机: 全灭 + 唤醒禁用(强制退出唤醒状态, 红灯灭) */
    power_on = false;
    led_yw_off(&led_handle);
    led_clear_timer();
    wake_ctrl_shutdown();
    my_printf("KEY: power OFF\n");
}

void led_key_power_toggle(void)
{
    if (power_on) {
        led_key_power_off();
    } else {
        led_key_power_on();
    }
}

bool led_power_is_on(void)
{
    return power_on;
}

void led_key_cycle_color(void)
{
    /* 短按循环: 关灯→白光L1→黄光L2→白黄L1+L2→关灯 (继承当前连续亮度)
     * 关灯档仅灭灯, 设备仍开机, 唤醒词仍可用 */
    if (!power_on) {
        return;                      /* 关机状态按键无反应 */
    }
    if (!led_handle.on) {
        led_handle.on = true;                       /* 关灯档→白光 */
        led_handle.color_index = COLOR_COOL;
    } else if (led_handle.color_index == COLOR_COOL) {
        led_handle.color_index = COLOR_WARM;        /* 白→黄 */
    } else if (led_handle.color_index == COLOR_WARM) {
        led_handle.color_index = COLOR_NATURAL;     /* 黄→白黄 */
    } else {
        led_handle.on = false;                      /* 白黄→关灯档 */
    }
    led_apply_current_val(&led_handle);             /* 继承连续亮度, 不重算档位 */
}

/* 连续调光参数: 100%↔20%, 每帧步进2%, 约2秒走完全程 */
#define KEY_DIM_MIN_PERCENT   20
#define KEY_DIM_MAX_PERCENT   100
#define KEY_DIM_STEP_PERCENT  2

int led_key_dim_step(int dir)
{
    /* 连续平滑调光: 每帧步进2%, 100%↔20%; 到边界闪烁一下并返回0 */
    int next;
    if (!power_on || !led_handle.on) {
        return 1;                    /* 关机/关灯档不调光 */
    }
    if (dir > 0) {
        next = led_handle.brightness_val + KEY_DIM_STEP_PERCENT;
        if (next >= KEY_DIM_MAX_PERCENT) {
            led_handle.brightness_val = KEY_DIM_MAX_PERCENT;
            led_limit_flash(&led_handle);   /* 到100%: 闪烁提示 */
            return 0;                       /* 到边界, 本次长按停止 */
        }
    } else {
        next = led_handle.brightness_val - KEY_DIM_STEP_PERCENT;
        if (next <= KEY_DIM_MIN_PERCENT) {
            led_handle.brightness_val = KEY_DIM_MIN_PERCENT;
            led_limit_flash(&led_handle);   /* 到20%: 闪烁提示 */
            return 0;
        }
    }
    led_handle.brightness_val = next;
    led_apply_current_val(&led_handle);   /* 应用亮度(返回值无关) */
    return 1;                              /* 正常步进: 未到边界, 返回1继续调光
                                            * (0 专留给"到边界"让状态机停) */
}

bool led_key_dim_at_max(void)
{
    return led_handle.brightness_val >= KEY_DIM_MAX_PERCENT;
}

bool led_key_dim_at_min(void)
{
    return led_handle.brightness_val <= KEY_DIM_MIN_PERCENT;
}

bool led_key_dim_enabled(void)
{
    return power_on && led_handle.on;
}

/* 唤醒提示(替代红灯, 2026-08-14): 蓝绿(L1/L2)两路同时快闪,
 * 由 wake_ctrl 状态机控制节奏(亮-灭-亮-灭);
 * 直接驱动 PWM 通道强制亮灭——绕过 pwm_apply_color 的
 * "关灯状态不亮"保护(否则灯灭时唤醒闪的全程是灭, 看不到提示) */
void led_key_wake_hint(bool on)
{
    if (on) {
        /* 唤醒提示用固定满亮度100: 关机/灭灯状态 brightness_val 为0,
         * 用亮度值闪会全程看不见(电饭煲语音模块无开灯概念) */
        set_timer_pwm_duty(BLUE_CH, 100);
        set_timer_pwm_duty(GREEN_CH, 100);
    } else {
        set_timer_pwm_duty(BLUE_CH, 0);
        set_timer_pwm_duty(GREEN_CH, 0);
    }
}

void led_key_wake_restore(void)
{
    if (led_handle.on) {
        pwm_apply_color(&led_handle, led_handle.brightness_val);
    } else {
        pwm_apply_color(&led_handle, 0);
    }
}

void led_loop(void)
{
    static led_timer_status led_status_last = LED_TIMER_DEFAULT;

    /* 按键轮询必须放在提前返回之前: 关机/关灯档也要检测双击开机
     * (2026-08-14 触摸按键换 PA4/5: 蓝灯挪 PA10/11 后 PA4/5 空出) */
    key_ctrl_loop();

    cook_flow_process();    /* 烹饪流程倒计时轮询 (不受 LED on 门控) */

    if (!led_handle.on) {
        if (led_status != LED_TIMER_DEFAULT) {
            led_status = LED_TIMER_DEFAULT;
            led_status_last = led_status;
            led_tick = 0;
        }
        return;
    }

    if (led_status != led_status_last) {
        led_status_last = led_status;
        led_tick = 0;
    }

    if (led_status != LED_TIMER_DEFAULT) {
        if (LED_TIMER_10MIN == led_status) {
            if (led_tick >= 10 * 60 * 1000) {
                led_status = LED_TIMER_DEFAULT;
                time_set_flag = false;
                led_yw_off(&led_handle);
                led_tick = 0;
            }
        } else if (LED_TIMER_30MIN == led_status) {
            if (led_tick >= 30 * 60 * 1000) {
                led_status = LED_TIMER_DEFAULT;
                time_set_flag = false;
                led_yw_off(&led_handle);
                led_tick = 0;
            }
        } else if (LED_TIMER_60S == led_status) {
            /* 睡眠模式: 60s 到点自动关灯 */
            if (led_tick >= 60 * 1000) {
                led_status = LED_TIMER_DEFAULT;
                time_set_flag = false;
                led_yw_off(&led_handle);
                led_tick = 0;
            }
        }
    }

    /* 音乐律动：非阻塞步进状态机。
     * led_loop 由 asr_sdadc_process 以 SDADC 帧率(~24~48ms)周期调用，每次推进一步。
     * 注意：不能用 led_tick 做时间基准——它被 time_set_flag 门控，非定时状态下冻结，
     * 会导致相位永远停在 0（蓝灯常亮不动的 bug 根因）。
     * 64 步一个循环：0~11 蓝亮 → 12~23 绿亮 → 24~63 双闪（约 1.5~3 秒一轮） */
    if (led_handle.music_beat_mode && led_handle.on) {
        int v = led_handle.brightness_val;      /* 沿用当前亮度档位 */
        u16 phase = beat_step % 64;
        if (phase < 12) {                       /* 蓝亮 */
            set_timer_pwm_duty(BLUE_CH, v);
            set_timer_pwm_duty(GREEN_CH, 0);
        } else if (phase < 24) {                /* 绿亮 */
            set_timer_pwm_duty(BLUE_CH, 0);
            set_timer_pwm_duty(GREEN_CH, v);
        } else {                                /* 双闪：5步亮 / 5步灭 */
            u16 sub = (phase - 24) % 10;
            if (sub < 5) {
                set_timer_pwm_duty(BLUE_CH, v);
                set_timer_pwm_duty(GREEN_CH, v);
            } else {
                set_timer_pwm_duty(BLUE_CH, 0);
                set_timer_pwm_duty(GREEN_CH, 0);
            }
        }
        beat_step++;
    }
}

// TEMP_DISABLED_FOR_DEBUG: key control functions
// void key1_shortpress_ctrl(void) { }
// void key2_shortpress_ctrl(void) { }
