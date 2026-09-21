#ifndef _MULTI_LANG_H
#define _MULTI_LANG_H

enum {
    IDX_LANGUAGE_MP3,
    IDX_LOW_BATTERY_MP3,
    IDX_POWERON_MP3,
    IDX_POWEROFF_MP3,
    IDX_MAX_VOL_MP3,
    IDX_SDCARD_MODE_MP3,
    IDX_USB_MODE_MP3,
    IDX_AUX_MODE_MP3,
    IDX_CLOCK_MODE_MP3,
    IDX_FM_MODE_MP3,
    IDX_SPK_MODE_MP3,
    IDX_PC_MODE_MP3,
    IDX_MAX_MP3,
};

typedef struct {
    u32 *ptr;
    u32 *len;
} res_addr_t;

const res_addr_t *res_get_ring_num(u8 index);

#if (LANG_SELECT == LANG_EN_ZH)
void multi_lang_init(uint lang_id);

extern const res_addr_t mul_lang_tbl[2][IDX_MAX_MP3];
#define RES_BUF_LANGUAGE_MP3        *mul_lang_tbl[sys_cb.lang_id][IDX_LANGUAGE_MP3   ].ptr
#define RES_LEN_LANGUAGE_MP3        *mul_lang_tbl[sys_cb.lang_id][IDX_LANGUAGE_MP3   ].len
#define RES_BUF_LOW_BATTERY_MP3     *mul_lang_tbl[sys_cb.lang_id][IDX_LOW_BATTERY_MP3].ptr
#define RES_LEN_LOW_BATTERY_MP3     *mul_lang_tbl[sys_cb.lang_id][IDX_LOW_BATTERY_MP3].len
//#define RES_BUF_POWERON_MP3         *mul_lang_tbl[sys_cb.lang_id][IDX_POWERON_MP3    ].ptr
//#define RES_LEN_POWERON_MP3         *mul_lang_tbl[sys_cb.lang_id][IDX_POWERON_MP3    ].len
#define RES_BUF_POWEROFF_MP3        *mul_lang_tbl[sys_cb.lang_id][IDX_POWEROFF_MP3   ].ptr
#define RES_LEN_POWEROFF_MP3        *mul_lang_tbl[sys_cb.lang_id][IDX_POWEROFF_MP3   ].len
#define RES_BUF_SDCARD_MODE_MP3     *mul_lang_tbl[sys_cb.lang_id][IDX_SDCARD_MODE_MP3].ptr
#define RES_LEN_SDCARD_MODE_MP3     *mul_lang_tbl[sys_cb.lang_id][IDX_SDCARD_MODE_MP3].len
#define RES_BUF_USB_MODE_MP3        *mul_lang_tbl[sys_cb.lang_id][IDX_USB_MODE_MP3   ].ptr
#define RES_LEN_USB_MODE_MP3        *mul_lang_tbl[sys_cb.lang_id][IDX_USB_MODE_MP3   ].len
#define RES_BUF_AUX_MODE_MP3        *mul_lang_tbl[sys_cb.lang_id][IDX_AUX_MODE_MP3   ].ptr
#define RES_LEN_AUX_MODE_MP3        *mul_lang_tbl[sys_cb.lang_id][IDX_AUX_MODE_MP3   ].len
#define RES_BUF_CLOCK_MODE_MP3      *mul_lang_tbl[sys_cb.lang_id][IDX_CLOCK_MODE_MP3 ].ptr
#define RES_LEN_CLOCK_MODE_MP3      *mul_lang_tbl[sys_cb.lang_id][IDX_CLOCK_MODE_MP3 ].len
#define RES_BUF_FM_MODE_MP3         *mul_lang_tbl[sys_cb.lang_id][IDX_FM_MODE_MP3    ].ptr
#define RES_LEN_FM_MODE_MP3         *mul_lang_tbl[sys_cb.lang_id][IDX_FM_MODE_MP3    ].len
#define RES_BUF_SPK_MODE_MP3        *mul_lang_tbl[sys_cb.lang_id][IDX_SPK_MODE_MP3   ].ptr
#define RES_LEN_SPK_MODE_MP3        *mul_lang_tbl[sys_cb.lang_id][IDX_SPK_MODE_MP3   ].len
#define RES_BUF_PC_MODE_MP3         *mul_lang_tbl[sys_cb.lang_id][IDX_PC_MODE_MP3    ].ptr
#define RES_LEN_PC_MODE_MP3         *mul_lang_tbl[sys_cb.lang_id][IDX_PC_MODE_MP3    ].len
#define RES_BUF_MAX_VOL_MP3         *mul_lang_tbl[sys_cb.lang_id][IDX_MAX_VOL_MP3    ].ptr
#define RES_LEN_MAX_VOL_MP3         *mul_lang_tbl[sys_cb.lang_id][IDX_MAX_VOL_MP3    ].len

#elif (LANG_SELECT == LANG_ZH)
#define RES_BUF_LOW_BATTERY_MP3         RES_BUF_ZH_LOW_BATTERY_MP3
#define RES_LEN_LOW_BATTERY_MP3         RES_LEN_ZH_LOW_BATTERY_MP3
#define RES_BUF_SDCARD_MODE_MP3         RES_BUF_ZH_SDCARD_MODE_MP3
#define RES_LEN_SDCARD_MODE_MP3         RES_LEN_ZH_SDCARD_MODE_MP3
#define RES_BUF_POWEROFF_MP3            RES_BUF_ZH_POWEROFF_MP3
#define RES_LEN_POWEROFF_MP3            RES_LEN_ZH_POWEROFF_MP3
#define RES_BUF_USB_MODE_MP3            RES_BUF_ZH_USB_MODE_MP3
#define RES_LEN_USB_MODE_MP3            RES_LEN_ZH_USB_MODE_MP3
#define RES_BUF_AUX_MODE_MP3            RES_BUF_ZH_AUX_MODE_MP3
#define RES_LEN_AUX_MODE_MP3            RES_LEN_ZH_AUX_MODE_MP3
#define RES_BUF_CLOCK_MODE_MP3          RES_BUF_ZH_CLOCK_MODE_MP3
#define RES_LEN_CLOCK_MODE_MP3          RES_LEN_ZH_CLOCK_MODE_MP3
#define RES_BUF_FM_MODE_MP3             RES_BUF_ZH_FM_MODE_MP3
#define RES_LEN_FM_MODE_MP3             RES_LEN_ZH_FM_MODE_MP3
#define RES_BUF_SPK_MODE_MP3            RES_BUF_ZH_SPK_MODE_MP3
#define RES_LEN_SPK_MODE_MP3            RES_LEN_ZH_SPK_MODE_MP3
#define RES_BUF_PC_MODE_MP3             RES_BUF_ZH_PC_MODE_MP3
#define RES_LEN_PC_MODE_MP3             RES_LEN_ZH_PC_MODE_MP3
#define RES_BUF_MAX_VOL_MP3             RES_BUF_ZH_MAX_VOL_MP3
#define RES_LEN_MAX_VOL_MP3             RES_LEN_ZH_MAX_VOL_MP3
#define RES_BUF_MUSIC_MODE_MP3          RES_BUF_ZH_MUSIC_MODE_MP3
#define RES_LEN_MUSIC_MODE_MP3          RES_LEN_ZH_MUSIC_MODE_MP3
#define RES_BUF_REC_START_MP3           RES_BUF_ZH_REC_START_MP3
#define RES_LEN_REC_START_MP3           RES_LEN_ZH_REC_START_MP3

#elif (LANG_SELECT == LANG_EN)
#define RES_BUF_LOW_BATTERY_MP3         //RES_BUF_EN_LOW_BATTERY_MP3
#define RES_LEN_LOW_BATTERY_MP3         //RES_LEN_EN_LOW_BATTERY_MP3
#define RES_BUF_SDCARD_MODE_MP3         RES_BUF_EN_SDCARD_MODE_MP3
#define RES_LEN_SDCARD_MODE_MP3         RES_LEN_EN_SDCARD_MODE_MP3
#define RES_BUF_POWEROFF_MP3            RES_BUF_EN_POWEROFF_MP3
#define RES_LEN_POWEROFF_MP3            RES_LEN_EN_POWEROFF_MP3
#define RES_BUF_USB_MODE_MP3            RES_BUF_EN_USB_MODE_MP3
#define RES_LEN_USB_MODE_MP3            RES_LEN_EN_USB_MODE_MP3
#define RES_BUF_AUX_MODE_MP3            RES_BUF_EN_AUX_MODE_MP3
#define RES_LEN_AUX_MODE_MP3            RES_LEN_EN_AUX_MODE_MP3
#define RES_BUF_CLOCK_MODE_MP3          RES_BUF_EN_CLOCK_MODE_MP3
#define RES_LEN_CLOCK_MODE_MP3          RES_LEN_EN_CLOCK_MODE_MP3
#define RES_BUF_FM_MODE_MP3             RES_BUF_EN_FM_MODE_MP3
#define RES_LEN_FM_MODE_MP3             RES_LEN_EN_FM_MODE_MP3
#define RES_BUF_SPK_MODE_MP3            RES_BUF_EN_SPK_MODE_MP3
#define RES_LEN_SPK_MODE_MP3            RES_LEN_EN_SPK_MODE_MP3
#define RES_BUF_PC_MODE_MP3             RES_BUF_EN_PC_MODE_MP3
#define RES_LEN_PC_MODE_MP3             RES_LEN_EN_PC_MODE_MP3
#define RES_BUF_MAX_VOL_MP3             RES_BUF_EN_MAX_VOL_MP3
#define RES_LEN_MAX_VOL_MP3             RES_LEN_EN_MAX_VOL_MP3
#define RES_BUF_MUSIC_MODE_MP3          RES_BUF_EN_MUSIC_MODE_MP3
#define RES_LEN_MUSIC_MODE_MP3          RES_LEN_EN_MUSIC_MODE_MP3
#define RES_BUF_REC_START_MP3           RES_BUF_EN_REC_START_MP3
#define RES_LEN_REC_START_MP3           RES_LEN_EN_REC_START_MP3

#else
    #error "请选择提示音语言\n"
#endif

#endif
