#ifndef KEY_CTRL_H
#define KEY_CTRL_H

#include <stdint.h>
#include <stdbool.h>

/* ============================================================
 * 触摸按键模块（PA1，测试板；高电平触发）
 * - 独立于 SDK IOKEY 框架，直接读 GPIO + 软件防抖
 * - 接线：一根线从 VBAT 引出模拟触摸高电平
 *   碰 VBAT = 按下(高)；悬空/碰GND = 未按(强下拉压死)
 * - 双击 = 电源开关；短按 = 循环切色；长按 = 连续调光
 * - key_ctrl_loop() 由 led_loop() 每帧调用（帧约48ms）
 * 产品板移植：只需改 key_ctrl.c 中的 KEY_IO / KEY_IO_MASK
 * ============================================================ */

void key_ctrl_init(void);
void key_ctrl_loop(void);

#endif /* KEY_CTRL_H */
