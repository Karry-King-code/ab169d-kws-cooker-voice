#include "include.h"

#define TRACE_EN                0

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

///CM Init时，判断该Page参数是否有效，有效则加载
bool cm_loadparam(void *buff, uint page)
{
/*  //避免参数错误导致擦除整个page,导致同page的其他参数丢失
    u8 *buf = buff;
    TRACE("cm_loadparam: %d\n", page);
    if (page == SYS_CM_PAGE_NUM) {
        sys_cb.vol = buf[PARAM_SYS_VOL];

        if (xcfg_cb.lang_id >= LANG_EN_ZH) {
            sys_cb.lang_id = buf[PARAM_LANG_ID];
            if (sys_cb.lang_id >= LANG_EN_ZH) {
                TRACE("lang_id: %d error\n", sys_cb.lang_id);
                return false;
            }
        } else {
            sys_cb.lang_id = xcfg_cb.lang_id;
        }
#if MUSIC_BREAKPOINT_EN
        if (f_msc.cur_dev == DEV_SDCARD) {
            memcpy(&f_msc.brkpt, buf + PARAM_MSC_BRKPT_SD, 10);
        } else if (f_msc.cur_dev == DEV_SDCARD1) {
            memcpy(&f_msc.brkpt, buf + PARAM_MSC_BRKPT_SD1, 10);
        } else {
            memcpy(&f_msc.brkpt, buf + PARAM_MSC_BRKPT_USB, 10);
        }
#endif // MUSIC_BREAKPOINT_EN

#if FUNC_FMRX_EN
        fmrx_cb.ch_cur = buf[PARAM_FMRX_CHCUR];
        fmrx_cb.ch_cnt = buf[PARAM_FMRX_CHCNT];
        memcpy(fmrx_cb.buf, buf + PARAM_FMRX_CHBUF, 26);
#endif // FUNC_FMRX_EN

        if(sys_cb.vol > VOL_MAX) {
            TRACE("vol: %d error\n", sys_cb.vol);
            return false;                       //音量值无效
        }
    }
*/
    return true;
}

///CM Init时，如果找不到有效的参数，则进行出厂值设置
void cm_factory(void *buff, uint page)
{
    u8 *buf = buff;
    memset(buf, 0, 250);

    TRACE("cm_factory: %d\n", page);
    if (page == SYS_CM_PAGE_NUM) {
        //系统参数初始化
//        f_msc.file_num = 1;
        sys_cb.vol = SYS_INIT_VOLUME;
#if FUNC_FMRX_EN
        fmrx_cb.ch_cur = 1;
        fmrx_cb.ch_cnt = 1;
#endif // FUNC_FMRX_EN

#if MUSIC_BREAKPOINT_EN
        memset(&f_msc.brkpt, 0, 10);
#endif // MUSIC_BREAKPOINT_EN

        if (xcfg_cb.lang_id == 2) {
            sys_cb.lang_id = 0;             //出厂默认英文
        } else if (xcfg_cb.lang_id == 3) {
            sys_cb.lang_id = 1;             //出厂默认中文
        } else {
            sys_cb.lang_id = xcfg_cb.lang_id;
        }
//        PUT_LE16(buf + PARAM_MSC_NUM_SD, f_msc.file_num);
//        PUT_LE16(buf + PARAM_MSC_NUM_USB, f_msc.file_num);
        buf[PARAM_SYS_VOL] = sys_cb.vol;
        buf[PARAM_LANG_ID] = sys_cb.lang_id;
    }
}
