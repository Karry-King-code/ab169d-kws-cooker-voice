#ifndef _SOUND_RES_H_
#define _SOUND_RES_H_

#include <stdint.h>

/* 播上电开机语"欢迎使用colorman智能产品" (res 资源, 16kHz 8kbps) */
void sound_play_boot_welcome(void);

/* 播指定编号的回应语 (1~43 对应 res/mp3/resp01~43.mp3) */
void sound_play_resp(int resp_id);

/* 播唤醒回应语"我在，请问有什么需要" */
void sound_play_wake_response(void);

/* ============ 异步播报接口 (识别中断只请求, 主循环播放) ============ */
#define RESP_ID_WAKE    0   /* 唤醒回应: resp02(唤醒改走res统一) */
#define RESP_ID_TIMEOUT 43  /* 30秒无指令: resp43 我先退下了有事请叫我 */

/* 烹饪流程播报 (电饭煲: 煮饭/煮粥/煮面/蒸煮 → 10s → 保温 → 5s) */
#define RESP_ID_COOK_DONE   44  /* resp44: 已结束,进入保温状态 */
#define RESP_ID_WARM_DONE   45  /* resp45: 保温已结束,欢迎下次使用 */
/* 中断上下文调用: 只记录待播ID, 立即返回, 不阻塞识别 */
void sound_request(int resp_id);
/* 主循环调用: 有待播ID则播放(含暂停/恢复采音), 无则立即返回 */
void sound_poll(void);

#endif /* _SOUND_RES_H_ */
