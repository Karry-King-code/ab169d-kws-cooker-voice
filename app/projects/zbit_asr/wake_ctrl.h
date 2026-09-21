#ifndef WAKE_CTRL_H
#define WAKE_CTRL_H

#include <stdint.h>
#include <stdbool.h>

/* ============================================================
 * 唤醒机制控制模块
 * - 唤醒词："你好小镜"(idx=0 完整词条) 或 "小镜小镜"(idx=1)
 * - 唤醒成功：主灯(L1/L2)同时快闪2下 (2026-08-14 替代红灯,
 *   PA10/11 让给触摸按键, 效果与红灯一致)
 * - 唤醒保持：10秒内任何词(唤醒词/命令词)刷新计时;超时退出
 * - 命令门控：仅唤醒状态下命令词(2-26)可执行
 * ============================================================ */

/* 帧周期约48ms(SDADC回调频率),以下参数以帧数计 */
#define WAKE_HOLD_FRAMES        625     /* 唤醒保持 30s (625 x 48ms) */
#define WAKE_FLASH_HALF_FRAMES  4       /* 闪烁半周期 ~200ms */

/* 初始化：红灯灭，状态复位 */
void wake_ctrl_init(void);

/* 每帧调用一次(仅在被识别词条命中时):
 * top=命中词条索引, 返回 true 表示该命令词可执行(唤醒状态下) */
bool wake_ctrl_process(int16_t top);

/* 每帧调用一次: 红灯闪烁/保持/超时状态机 */
void wake_ctrl_loop(void);

/* 查询是否处于唤醒保持态(WAKE_ON): 供错误指令检测等使用 */
bool wake_ctrl_is_on(void);

/* 双击关机时调用: 强制退出唤醒状态, 红灯灭 (电源门控) */
void wake_ctrl_shutdown(void);

#endif /* WAKE_CTRL_H */
