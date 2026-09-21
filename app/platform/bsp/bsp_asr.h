#ifndef __BSP_ASR_H
#define __BSP_ASR_H

extern u32 __aram_asr_comm_vma, __aram_asr_comm_lma, __aram_asr_comm_size;

enum {
    ASR_IDLE = 0,
    ASR_START,
};

enum {
    ASR_FLAG_ALG_PROC = BIT(0),
    ASR_FLAG_CLK_RESTORE = BIT(1),
    ASR_FLAG_CLK_INC = BIT(2),
};

enum {
    VAD_STATE_NO_VOICE,
    VAD_STATE_VOICE_START,
    VAD_STATE_VOICE_SPEAKING,
    VAD_STATE_VOICE_STOP,
};

void bsp_asr_init(void);
void bsp_asr_start(void);
void bsp_asr_stop(void);

#endif
