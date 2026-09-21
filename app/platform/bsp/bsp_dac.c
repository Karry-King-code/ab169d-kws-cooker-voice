#include "include.h"
#include "bsp_dac.h"

uint8_t dac_fast_setup_en = DAC_FAST_SETUP_EN;
bool dac_mute_sta_sel = DAC_MUTE_STA_SEL;
#define BSP_DAC_DEBUG_EN 0

const u16 *dac_dvol_table;

AT(.rodata.dac)
const u16 dig_vol_tbl[61] = {
    DIG_N0DB,  DIG_N1DB,  DIG_N2DB,  DIG_N3DB,  DIG_N4DB,  DIG_N5DB,  DIG_N6DB,  DIG_N7DB,
    DIG_N8DB,  DIG_N9DB,  DIG_N10DB, DIG_N11DB, DIG_N12DB, DIG_N13DB, DIG_N14DB, DIG_N15DB,
    DIG_N16DB, DIG_N17DB, DIG_N18DB, DIG_N19DB, DIG_N20DB, DIG_N21DB, DIG_N22DB, DIG_N23DB,
    DIG_N24DB, DIG_N25DB, DIG_N26DB, DIG_N27DB, DIG_N28DB, DIG_N29DB, DIG_N30DB, DIG_N31DB,
    DIG_N32DB, DIG_N33DB, DIG_N34DB, DIG_N35DB, DIG_N36DB, DIG_N37DB, DIG_N38DB, DIG_N39DB,
    DIG_N40DB, DIG_N41DB, DIG_N42DB, DIG_N43DB, DIG_N44DB, DIG_N45DB, DIG_N46DB, DIG_N47DB,
    DIG_N48DB, DIG_N49DB, DIG_N50DB, DIG_N51DB, DIG_N52DB, DIG_N53DB, DIG_N54DB, DIG_N55DB,
    DIG_N56DB, DIG_N57DB, DIG_N58DB, DIG_N59DB, DIG_N60DB,
};

AT(.rodata.dac)
const u16 dig_vol_tbl_up[6] = {
    DIG_P1DB, DIG_P2DB, DIG_P3DB, DIG_P4DB, DIG_P5DB, DIG_P6DB,
};

AT(.rodata.dac)
const u16 dac_dvol_tbl_16[16 + 1] = {
    60,  43,  32,  26,  24,  22,  20,  18, 16,
    14,  12,  10,  8,   6,   4,   2,   0,
};

AT(.rodata.dac)
const u16 dac_dvol_tbl_32[32 + 1] = {
    60,  50,  43,  38,  35,  30,  28,  26,
    24,  23,  22,  21,  20,  19,  18,  17,
    16,  15,  14,  13,  12,  11,  10,  9,
    8,   7,   6,   5,   4,   3,   2,   1,   0,
};

AT(.rodata.dac)
const u16 dac_dvol_tbl_50[50 + 1] = {
    60,  56,  54,  52,  50,  48,  46,  44,
    42,  39,  38,  37,  36,  35,  34,  33,
    32,  31,  30,  29,  28,  27,  26,  25,
    24,  23,  22,  21,  20,  19,  18,  17,
    16,  15,  14,  13,  12,  11,  10,  9,
    8,   7,   6,   5,   4,   3,   4,   3,
    2,   1,   0,
};

AT(.text.bsp.dac)
void bsp_change_volume(u8 vol)
{
    u8 level = 0;

    if (vol <= VOL_MAX) {
        level = dac_dvol_table[vol];
        if (level > 60) {
            level = 60;
        }
        dac_set_dvol(dig_vol_tbl[level], 2, 0);  //三个参数分别是设置的音量，音量设置的步长，是否直接设置音量
    }
}

AT(.text.dac)
bool bsp_set_volume(u8 vol)
{
    bsp_change_volume(vol);
    if (vol == sys_cb.vol) {
        return false;
    }

    if (vol <= VOL_MAX) {
        sys_cb.vol = vol;
        param_sys_vol_write();
        sys_cb.cm_times = 0;
        sys_cb.cm_vol_change = 1;
    }
    printf("vol: %d\n", sys_cb.vol);
    return true;
}


AT(.text.dac.vtbl)
u8 bsp_volume_inc(u8 vol, u8 step)
{
    vol += step;
    if(vol > VOL_MAX)
        vol = VOL_MAX;
    return vol;
}

AT(.text.dac.vtbl)
u8 bsp_volume_dec(u8 vol, u8 step)
{
    if(vol >= step) {
        vol -= step;
    } else {
        vol = 0;
    }
    return vol;
}

#if BSP_DAC_DEBUG_EN
//1K
AT(.com_rodata.test)
s16 sina_48k_tbl[48] = {
    0x0000, 0x085F, 0x109A, 0x188D, 0x2013, 0x270E, 0x2D5C, 0x32E4,
    0x378F, 0x3B45, 0x3DF7, 0x3F99, 0x4026, 0x3F9B, 0x3DF7, 0x3B45,
    0x378E, 0x32E4, 0x2D5C, 0x270E, 0x2014, 0x188D, 0x109B, 0x085F,
    0x0000, 0xF7A0, 0xEF65, 0xE774, 0xDFED, 0xD8F2, 0xD2A4, 0xCD1C,
    0xC872, 0xC4BC, 0xC208, 0xC067, 0xBFD9, 0xC067, 0xC209, 0xC4BC,
    0xC872, 0xCD1B, 0xD2A4, 0xD8F3, 0xDFEC, 0xE773, 0xEF65, 0xF7A1,
};

AT(.com_rodata.test)
u8 sina_48k_0db_mono_tbl[] = {
  0x01, 0x00, 0xb5, 0x10, 0x22, 0x21, 0xfb, 0x30, 0x00, 0x40, 0xeb, 0x4d,
  0x82, 0x5a, 0x8b, 0x65, 0xda, 0x6e, 0x40, 0x76, 0xa3, 0x7b, 0xe7, 0x7e,
  0xfe, 0x7f, 0xe7, 0x7e, 0xa2, 0x7b, 0x42, 0x76, 0xd9, 0x6e, 0x8b, 0x65,
  0x81, 0x5a, 0xeb, 0x4d, 0x00, 0x40, 0xfc, 0x30, 0x21, 0x21, 0xb5, 0x10,
  0x00, 0x00, 0x4b, 0xef, 0xde, 0xde, 0x04, 0xcf, 0x00, 0xc0, 0x15, 0xb2,
  0x7f, 0xa5, 0x74, 0x9a, 0x27, 0x91, 0xbf, 0x89, 0x5e, 0x84, 0x1a, 0x81,
  0x01, 0x80, 0x19, 0x81, 0x5e, 0x84, 0xbf, 0x89, 0x26, 0x91, 0x75, 0x9a,
  0x7e, 0xa5, 0x15, 0xb2, 0x00, 0xc0, 0x04, 0xcf, 0xdf, 0xde, 0x4b, 0xef
};

AT(.com_rodata.test)
s16 sina_16k_tbl[16] = {
    0x0000, 0x188D, 0x2D5C, 0x3B44, 0x4026, 0x3B45, 0x2D5C, 0x188D,
    0x0000, 0xE773, 0xD2A3, 0xC4BC, 0xBFDA, 0xC4BC, 0xD2A4, 0xE774,
};

void dac_play_sin1k_test(void)
{
    printf("`BSP_DAC_DEBUG_EN` is true, set false to disable debuging:\n");
    printf("sin1k playing...\n");
    bsp_change_volume(sys_cb.vol);
#if 1
    dac_spr_set(SPR_48000);
    while (1) {
        if (dac_gpdma_is_ready()) {
            dac_gpdma_kick((s16 *)sina_48k_0db_mono_tbl, 48);
        }
        WDT_CLR();
        static u32 bsp_dac_ticks = 3000;
        if (tick_check_expire(bsp_dac_ticks,3000)) {
            bsp_dac_ticks = tick_get();
            printf("DACVOLCON: %x\n", DACVOLCON);
        }
    }
#else
    dac_spr_set(SPR_16000);
    while (1) {
        if (dac_gpdma_is_ready()) {
            dac_gpdma_kick((s16 *)sina_16k_tbl, 16);
        }
        WDT_CLR();
    }
#endif
}
#endif // BSP_DAC_DEBUG_EN

AT(.text.bsp.dac)
void dac_set_vol_table(u8 vol_max)
{
    if (vol_max == 16) {
        dac_dvol_table = dac_dvol_tbl_16;
    } else if (vol_max <= 32) {
        dac_dvol_table = dac_dvol_tbl_32;
    } else {
        dac_dvol_table = dac_dvol_tbl_50;
    }
}

AT(.text.bsp.dac)
void dac_init(void)
{
    dac_set_vol_table(sys_cb.vol_max);
    printf("[%s] vol_max:%d\n", __func__, sys_cb.vol_max);
#if DAC_MAP_EN
    dac_io_map(DAC_CLASSD_P, DAC_CLASSD_N);
#endif
    GPIOADE &= ~BIT(3);

#if DAC_FAST_SETUP_EN
    loudspk_mute_init(DAC_MUTE_IO_SEL,DAC_MUTE_STA_SEL);
#endif // DAC_FAST_SETUP_EN

    dac_power_on();

#if BSP_DAC_DEBUG_EN
    dac_play_sin1k_test();
#endif
}


