#include "include.h"

///语音识别处理例程，客户根据需要修改命令列表和处理流程

#define ASR_CMD_NUM         49
#define ASR_CMD_LEN         18

enum {
    ASR_CMD_1_CASE,
    ASR_CMD_2_CASE,
    ASR_CMD_3_CASE,
    ASR_CMD_4_CASE,
    ASR_CMD_5_CASE,
    ///...
};

static const char str_asr_cmd[ASR_CMD_NUM][ASR_CMD_LEN] = {
	"打开灯光",
	"小艾你好",
	"打开风扇",
	"关闭风扇",
	"打开上下摇头",
	"关闭上下摇头",
	"打开摇头",
	"关闭摇头",
	"打开左右摇头",
	"关闭左右摇头",
	"定时一小时",
	"定时二小时",
	"定时三小时",
	"定时四小时",
	"定时五小时",
	///...
};

///处理识别结果，注意耗时不要太长，耗时太长的操作可以放到main process中进行
AT(.asr_text.asr_deal)
void asr_result_deal(const char *result)
{
    int i;
    for (i = 0; i < ASR_CMD_NUM; i++) {
        if (strcmp(result, str_asr_cmd[i]) == 0) {
            break;
        }
    }
//    printf("asr deal:%x\n", i);
    switch (i) {
        case ASR_CMD_1_CASE:
            #if 0 //esbc+mp3播放参考
            esbc_res_play_do(RES_BUF_SBC_K003_SBC, RES_LEN_SBC_K003_SBC, 0);
            mp3_res_play(RES_BUF_EN_POWERON_MP3, RES_LEN_EN_POWERON_MP3);
            bsp_asr_restart();
            #endif
            break;

        case ASR_CMD_2_CASE:
            break;

        case ASR_CMD_3_CASE:
            // mp3_res_play(RES_BUF_EN_POWERON_MP3, RES_LEN_EN_POWERON_MP3);
            break;
        ///...

        default:
            break;
    }
}

