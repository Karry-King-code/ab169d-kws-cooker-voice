#include "include.h"
#include "uart_cmd.h"

/* 命令帧: 0xA5 0xFA | ID | 校验 | 0xFB
 * 发送: HUART 硬件UART(PA5, pin5), 波特率HUART_BAUD(4800), 硬件准确
 * 替代bit-bang(其波特率无法校准) */
#define CMD_H1    0xA5
#define CMD_H2    0xFA
#define CMD_END   0xFB

void uart_cmd_send(int cmd_id)
{
    u8 frame[5];

    frame[0] = CMD_H1;
    frame[1] = CMD_H2;
    frame[2] = (u8)(cmd_id & 0xFF);
    frame[3] = (u8)((CMD_H1 + CMD_H2 + cmd_id) & 0xFF);   /* 校验 */
    frame[4] = CMD_END;

    bsp_huart_putchar_data(frame, sizeof(frame));   /* HUART硬件发送 */
}

/* DEBUG遥测帧(2026-08-31): 未命中候选帧发 [A5 FB top probL probH thrL thrH CK FB] (9字节)
 * 与命令帧(A5 FA..FB)共用HUART/PA10; 仅在 prob<thr(未命中)时调用, 与 uart_cmd_send 同帧互斥无冲突。
 * 逻辑分析仪 4562bps 解码, 供离线看候选词/得分/门槛调 set_kw_offset。
 * HUART_DEBUG_TELEMETRY=0 时本函数为空, 正式版零开销。 */
void uart_debug_frame(int top, int prob, int thr)
{
#if HUART_DEBUG_TELEMETRY
    u8 f[9];

    f[0] = CMD_H1;
    f[1] = 0xFB;                                   /* 与命令帧 H2=0xFA 区分 */
    f[2] = (u8)top;
    f[3] = (u8)(prob & 0xFF);
    f[4] = (u8)((prob >> 8) & 0xFF);
    f[5] = (u8)(thr & 0xFF);
    f[6] = (u8)((thr >> 8) & 0xFF);
    f[7] = (u8)(f[0]+f[1]+f[2]+f[3]+f[4]+f[5]+f[6]);
    f[8] = CMD_END;

    bsp_huart_putchar_data(f, sizeof(f));
#else
    (void)top; (void)prob; (void)thr;
#endif
}
