#ifndef UART_CMD_H
#define UART_CMD_H

#include <stdint.h>

/* 串口命令协议(发给主控 AD248B):
 *   帧: 0xA5 0xFA | 命令ID(1B) | 校验(1B) | 0xFB
 *   校验: (0xA5 + 0xFA + 命令ID) & 0xFF
 *   命令ID: 0=唤醒词, 1~36=命令词(对应词表 top)
 *   波特率: 115200 8N1 (待客户确认) */
void uart_cmd_send(int cmd_id);

/* DEBUG遥测帧(未命中候选: top/prob/thr 发PA10给逻辑分析仪, HUART_DEBUG_TELEMETRY=1时生效)
 * 帧格式: 0xA5 0xFB | top | probL | probH | thrL | thrH | 校验 | 0xFB (9字节)
 * 仅 prob<thr 的识别帧调用, 与命令帧同帧互斥 */
void uart_debug_frame(int top, int prob, int thr);

#endif /* UART_CMD_H */
