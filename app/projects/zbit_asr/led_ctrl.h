#include <stdint.h>
#include <stdbool.h>

#define LED_LOW_LEVEL_LIGHT     0   // default
#define LED_HIGH_LEVEL_LIGHT    1

#define LED_LIGHT_MODE      LED_HIGH_LEVEL_LIGHT

void led_ctrl_init(void);
int led_ctrl(uint16_t cmd);
void led_loop(void);

/* ============================================================
 * 触摸按键 LED 接口（key_ctrl 调用）
 *   双击: led_key_power_on / off / toggle
 *   短按: led_key_cycle_color
 *   长按: led_key_dim_step / dim_at_max / dim_at_min
 *   唤醒门控: led_power_is_on
 * ============================================================ */
void led_key_power_on(void);          /* 双击开机: 白光+100% */
void led_key_power_off(void);         /* 双击关机: 全灭+唤醒禁用 */
void led_key_power_toggle(void);      /* 双击: 开关切换 */
bool led_power_is_on(void);           /* 电源状态(唤醒词门控) */
void led_key_cycle_color(void);       /* 短按: 循环切色(继承亮度) */
int  led_key_dim_step(int dir);       /* 长按: 连续调光一步, 返回0=到边界 */
bool led_key_dim_at_max(void);        /* 当前是否已最亮 */
bool led_key_dim_at_min(void);        /* 当前是否已最暗 */
bool led_key_dim_enabled(void);       /* 是否允许调光(开机且灯亮) */
void led_key_wake_hint(bool on);      /* 唤醒提示: 蓝绿(L1/L2)两路亮/灭(快闪用) */
void led_key_wake_restore(void);      /* 快闪结束: 恢复灯光原状态 */

