#include "include.h"

#define FS_CRC_SEED         0xffff

uint calc_crc(void *buf, uint len, uint seed);
bool mp3_dec_frame(void);
bool wav_dec_frame(void);
bool esbc_decode_frame(esbc_dec_cb_t *esbc_d_vars);
extern s16 gpdma_buf[576*2];

esbc_dec_cb_t esbc_dec_cb AT(.esbc_buf);        //0/1号输入
msc_cb_t msc_cb AT(.music_buf);

//音乐播放公共初始化
void music_res_init(void)
{
    u8 sys_clk = get_cur_sysclk();
    if(sys_clk < SYS_120M){
        set_sys_clk(SYS_120M);
    }
#if ASR_RECOG_EN
    bsp_asr_stop();
#endif
#if DAC_EN
    memset(gpdma_buf, 0, sizeof(gpdma_buf));
    dac_gpdma_reset();
#endif
}

void music_res_exit(void)
{
    /* 播报后不要无条件重配时钟(会干扰SDADC采样率, 识别失聪几秒)。
     * 也不重启ASR(asr_start会清空唤醒态wake_ctrl_init, 导致播报后命令失效)。
     * 引擎重置由 sound_res 的 bsp_asr_restart 负责(只kws_init, 不动唤醒态)。 */
    u8 sys_clk = get_cur_sysclk();
    if (sys_clk != SYS_CLK_SEL) {
        set_sys_clk(SYS_CLK_SEL);   /* 仅当时钟确实变化时才恢复 */
    }
#if 0
#if ASR_RECOG_EN
    bsp_asr_start();
#endif
#endif
}

#if WARNING_TONE_EN
void mp3_res_play_do(u32 addr, u32 len, bool sync)
{
#if WARNING_MP3_PLAY
    u16 msg;

    if (len == 0) {
        return;
    }

    if(get_music_dec_sta() != MUSIC_STOP) {
        music_control(MUSIC_STOP);
    }
    bsp_change_volume(WARNING_VOLUME);
    mp3_res_play_kick(addr, len);
    while (get_music_dec_sta() != MUSIC_STOP) {
        WDT_CLR();
        if (!music_decode()) {
            break;
        }
        music_pcm_gpdma_kick();
        msg = msg_dequeue();

#if (EX_SPIFLASH_SUPPORT & EXSPI_REC)
        if ((msg == KU_REC) || (msg == KL_REC)) {
            break;
        }
#endif

        if (msg != NO_MSG) {
            msg_enqueue(msg);       //还原未处理的消息
        }
    }
    music_control(MUSIC_STOP);
    bsp_change_volume(sys_cb.vol);
    mp3_res_play_exit();
#else
    printf("WARNING_MP3_PLAY disable\n");
#endif
}

void esbc_res_play_do(u32 addr, u32 len, bool sync)
{
#if WARNING_ESBC_PLAY
    u16 msg;
    u8 *ptr = (u8 *)addr;

    if (len == 0 || ptr[0] != 0x8B) {
        printf("esbc format err\n");
        return;
    }

    if(get_music_dec_sta() != MUSIC_STOP) {
        music_control(MUSIC_STOP);
    }
    bsp_change_volume(WARNING_VOLUME);
    esbc_res_play_kick(addr, len);
    while (get_music_dec_sta() != MUSIC_STOP) {
        WDT_CLR();
        if (!music_decode()) {
            break;
        }
        music_pcm_gpdma_kick();
        msg = msg_dequeue();
        if (msg != NO_MSG) {
            msg_enqueue(msg);       //还原未处理的消息
        }
    }
    music_control(MUSIC_STOP);
    bsp_change_volume(sys_cb.vol);
    esbc_res_play_exit();
#else
    printf("WARNING_ESBC_PLAY disable\n");
#endif
}

void esbc_link_play_do(const esbc_link_t *elink, const u8 e_num)
{
#if WARNING_ESBC_PLAY
    u16 msg;
    u8 i, *ptr;
    u32 *paddr, *plen;
    for(i = 0; i < e_num; i++){
        ptr  = (u8 *)(*elink[i].addr);
        plen = (u32 *)(*elink[i].len);
        if (plen == 0 || ptr[0] != 0x8B) {
            return;
        }
    }
    if(get_music_dec_sta() != MUSIC_STOP) {
        music_control(MUSIC_STOP);
    }
    bsp_change_volume(WARNING_VOLUME);
    esbc_link_play_init();
    for(i = 0; i < e_num; i++){
        paddr = (u32 *)(*elink[i].addr);
        plen  = (u32 *)(*elink[i].len);
        esbc_link_play_kick((u32)paddr, (u32)plen, elink[i].sta_skip_frame, elink[i].end_skip_frame);
        while (get_music_dec_sta() != MUSIC_STOP) {
            WDT_CLR();
            if (!music_decode()) {
                break;
            }
            music_pcm_gpdma_kick();
            msg = msg_dequeue();
            if (msg != NO_MSG) {
                msg_enqueue(msg);       //还原未处理的消息
            }
        }
    }
    music_control(MUSIC_STOP);
    bsp_change_volume(sys_cb.vol);
    esbc_res_play_exit();
#else
    printf("WARNING_ESBC_PLAY disable\n");
#endif
}
#else
void mp3_res_play_do(u32 addr, u32 len, bool sync){}
void esbc_res_play_do(u32 addr, u32 len, bool sync){}
#endif

#if WARNING_WAV_PLAY
void wav_res_play_do(u32 addr, u32 len, bool sync)
{
    u16 msg;
    printf("%s: addr: %x, len: %x\n", __func__, addr, len);
    if (len == 0) {
        return;
    }
    if(get_music_dec_sta() != MUSIC_STOP) {
        music_control(MUSIC_STOP);
    }
    bsp_change_volume(WARNING_VOLUME);
    wav_res_play_kick(addr, len);
#if WARNING_PAUSE_PLAY
    music_res_jump();
#endif
    while (get_music_dec_sta() != MUSIC_STOP) {
        WDT_CLR();
        if (!music_decode()) {
            break;
        }
        music_pcm_gpdma_kick();
        msg = msg_dequeue();

#if (EX_SPIFLASH_SUPPORT & EXSPI_REC)
        if ((msg == KU_REC) || (msg == KL_REC)) {
            break;
        }
#endif

#if WARNING_PAUSE_PLAY
        if (msg == KU_PLAY_POWER) {     //此处可自定义msg暂停提示音播放并回到主循环，调用mp3_res_continue恢复提示音播放
            music_res_pause(addr, len, wav_res_play);
            msg_enqueue(msg);
            break;
        }
#endif
        if (msg != NO_MSG) {
            msg_enqueue(msg);       //还原未处理的消息
        }
    }
    wav_res_play_exit();
}
#else
void wav_res_play_do(u32 addr, u32 len, bool sync){}
#endif

void mp3_res_play(u32 addr, u32 len)
{
    printf("%s: addr: %x, len: %x\n", __func__, addr, len);
    music_res_init();
    mp3_res_play_do(addr, len, 0);
    music_res_exit();
}

void esbc_res_play(u32 addr, u32 len)
{
    printf("%s: addr: %x, len: %x\n", __func__, addr, len);
    music_res_init();
    esbc_res_play_do(addr, len, 0);
    music_res_exit();
}

void music_mode_set(msc_mode_e mode)
{
    if(msc_cb.msc_mode != mode){
        msc_cb.msc_mode = mode;
    }
}

bool music_mode_check(msc_mode_e mode)
{
    bool bo_ret = false;
    if(msc_cb.msc_mode == mode){
        bo_ret = true;
    }
    return bo_ret;
}


//wav播放接口
void wav_res_play(u32 addr, u32 len)
{
    music_res_init();
    music_mode_set(MSC_WAV);
    wav_res_play_do(addr, len, 0);
    music_mode_set(MSC_NONE);
    music_res_exit();
}

AT(.text.music)
bool music_decode(void)
{
    if (music_cb.pcm_samples) {
        return true;
    }
    if (music_cb.type == WAV_TYPE) {
#if (WARNING_WAV_PLAY || EXSPI_WAV_EN)
        if(wav_res_is_play()){
            return wav_res_decode(0);
        }

#endif
    } else if (music_cb.type == MP3_TYPE) {
#if (WARNING_MP3_PLAY || EXSPI_MP3_EN)
        return mp3_dec_frame();
#endif
    } else if (music_cb.type == VSBC_TYPE) {

    } else if (music_cb.type == ESBC_TYPE) {
#if (WARNING_ESBC_PLAY || EXSPI_ESBC_EN)
        return esbc_decode_frame(&esbc_dec_cb);
#endif
    }
    music_cb.sta = MUSIC_STOP;
    return false;
}

#if 0
#define MSC_ESBC_LINK1_NUM  3
esbc_link_t msc_esbc_link1[MSC_ESBC_LINK1_NUM] = {
    {&RES_BUF_SBC_K003_SBC, &RES_LEN_SBC_K003_SBC, 5, 50},
    {&RES_BUF_SBC_K004_SBC, &RES_LEN_SBC_K004_SBC, 5, 50},
    {&RES_BUF_SBC_K005_SBC, &RES_LEN_SBC_K005_SBC, 5, 50},
};
void esbc_link_test(void)
{
    esbc_link_play_do(msc_esbc_link1, MSC_ESBC_LINK1_NUM);
}
#endif
