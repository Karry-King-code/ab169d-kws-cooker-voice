#include "include.h"
#include "bsp_eq.h"
#if 0
#define EQ_CRC_SEED                             0xffff
#define EQ_BAND_NUM                             12
#define CAL_FIX(x)		                        ((int)(x * (1 << 27)))

uint calc_crc(void *buf, uint len, uint seed);
int eq_huart_putcs(uint8_t *packet, uint16_t len);
void huart_eq_init(u8 *rx_buf, uint rx_len);
void huart_rx_kick(void);

extern const eq_param music_eq_tbl[MUSIC_EQ_TBL_LEN];

AT(.text.music)
void music_set_eq_by_num(u8 num)
{
    // if (num > (MUSIC_EQ_TBL_LEN - 1)) {
    //     return;
    // }
    // music_set_eq_by_res(music_eq_tbl[num].addr, music_eq_tbl[num].len);
}

#if EQ_MODE_EN

AT(.text.music)
void sys_set_eq(void)
{
    sys_cb.eq_mode++;
    if (sys_cb.eq_mode > 5) {
        sys_cb.eq_mode = 0;
    }
    music_set_eq_by_num(sys_cb.eq_mode);
    gui_box_show_eq();
}
#endif // EQ_MODE_EN

u8 eq_rx_buf[EQ_BUFFER_LEN];
#if EQ_DBG_IN_UART
eq_dbg_cb_t eq_dbg_cb;
static u8 eq_tx_buf[12];
static gpio_t eq_gpio;

//格式:[E/D, Q/R, 0, *, (u8)size, (size >> 8), band_cnt, param size, param cnt, rsvd]
const char tbl_eq_version[10] = {'E', 'Q', '0', '*', 6, 0, 12, 4, 5, 0};
const char tbl_drc_version[10] = {'D', 'R', '0', '*', 6, 0, 2, 0, 0, 0};

static void tx_ack(uint8_t *packet, uint16_t len)
{
    delay_5ms(1);   //延时一段时间再ack
    if (eq_dbg_cb.rx_type) {
#if EQ_DBG_IN_UART
        eq_huart_putcs(packet, len);
#endif // EQ_DBG_IN_UART
    }
    eq_dbg_cb.rx_type = 0;
}

void eq_tx_ack(u8 bank_num, u8 ack)
{
    if (eq_dbg_cb.type) {
        eq_tx_buf[0] = 'D';
    } else {
        eq_tx_buf[0] = 'A';
    }
    eq_tx_buf[1] = bank_num;
    eq_tx_buf[2] = ack;
    eq_tx_buf[3] = -(eq_tx_buf[0] + eq_tx_buf[1] + eq_tx_buf[2]);
    tx_ack(eq_tx_buf, 4);
}

void eq_tx_version(void)
{
    if (eq_dbg_cb.type) {
        memcpy(eq_tx_buf, tbl_drc_version, 10);
    } else {
        memcpy(eq_tx_buf, tbl_eq_version, 10);
    }
    u16 crc = calc_crc(eq_tx_buf, 10, EQ_CRC_SEED);
    eq_tx_buf[10] = crc;
    eq_tx_buf[11] = crc >> 8;
    tx_ack(eq_tx_buf, 12);
}

void eq_parse_cmd(void)
{
    if (eq_rx_buf[0] == 'E' || eq_rx_buf[1] == 'Q') {
        eq_dbg_cb.type = 0;
    } else if (eq_rx_buf[0] == 'D' && eq_rx_buf[1] == 'R') {
        eq_dbg_cb.type = 1;
    } else {
        return;
    }
    if (eq_rx_buf[2] == '?' && eq_rx_buf[3] == '#') {
        eq_tx_version();
        return;
    }

//    print_r(eq_rx_buf, EQ_BUFFER_LEN);
    u8 band_num = eq_rx_buf[6];
    u32 size = little_endian_read_16(eq_rx_buf, 4);
    u32 crc = calc_crc(eq_rx_buf, size+4, EQ_CRC_SEED);
    if (crc == little_endian_read_16(eq_rx_buf, 4+size)) {
        eq_tx_ack(band_num, 0);
    } else {
        eq_tx_ack(band_num, 1);
        return;
    }

    if (crc == eq_dbg_cb.pre_crc) {
        return;
    }
    eq_dbg_cb.pre_crc = crc;

    if (eq_dbg_cb.type == 0) {
        u8 band_cnt = eq_rx_buf[7];
        printf("band_cnt = %d\n",band_cnt);
        music_set_eq(band_cnt, (u32 *)&eq_rx_buf[14]);

#if (UART0_PRINTF_SEL != PRINTF_NONE)
        printf("%08x\n", little_endian_read_32(eq_rx_buf, 14));
        u8 k;
        u32 *ptr = (u32 *)&eq_rx_buf[18];
        for (k = 0; k < band_cnt*5; k++) {
            printf("%08x", *ptr++);
            if (k % 5 == 4) {
                printf("\n");
            } else {
                printf(" ");
            }
        }
#endif
    } else {
        drc_set_param((u32 *)&eq_rx_buf[10]);
#if (UART0_PRINTF_SEL != PRINTF_NONE)
        u32 *ptr = (u32 *)&eq_rx_buf[10];
        for (int i = 0; i < 10; i++) {
            printf("%08x\n", ptr[i]);
        }
        printf("\n");
#endif
    }

    memset(eq_rx_buf, 0, EQ_BUFFER_LEN);
}

#if EQ_DBG_IN_UART

AT(.com_eq.text)
void huart_rx_done(void)
{
    msg_enqueue(EVT_ONLINE_SET_EQ);
    eq_dbg_cb.rx_type = 1;
}

AT(.com_eq.text)
void huart_rx_set_dir(void)
{
    if (eq_gpio.sfr != NULL) {
        eq_gpio.sfr[GPIOxDIR] |= BIT(eq_gpio.num);
    }
}

AT(.com_eq.text)
void huart_tx_set_dir(void)
{
    if (eq_gpio.sfr != NULL) {
        eq_gpio.sfr[GPIOxDIR] &= ~BIT(eq_gpio.num);
    }
}

void eq_dbg_huart_init(void)
{
    u8 io_num = IO_NONE;
    FUNCMCON0 = 0x0f << 20 | 0x0f << 16;
    if (xcfg_cb.eq_uart_sel == 0) {                 //HUART PA7
        if (UART0_PRINTF_SEL == PRINTF_PA7) {
            FUNCMCON0 = 0x0f << 8;
        }
        FUNCMCON0 = 0x04 << 20 | 0x04 << 16;
        io_num = IO_PA7;
    } else if (xcfg_cb.eq_uart_sel == 1) {          //HUART PB2
        if (UART0_PRINTF_SEL == PRINTF_PB2) {
            FUNCMCON0 = 0x0f << 8;
        }
        FUNCMCON0 = 0x06 << 20 | 0x06 << 16;
        io_num = IO_PB2;
    } else if (xcfg_cb.eq_uart_sel == 2) {          //HUART PB3
        if (UART0_PRINTF_SEL == PRINTF_PB3) {
            FUNCMCON0 = 0x0f << 8;
        }
        FUNCMCON0 = 0x07 << 20 | 0x07 << 16;
        io_num = IO_PB3;
    } else if (xcfg_cb.eq_uart_sel == 3) {          //HUART PA2
        if (UART0_PRINTF_SEL == PRINTF_PA2) {
            FUNCMCON0 = 0x0f << 8;
        }
        FUNCMCON0 = 0x02 << 20 | 0x02 << 16;
        io_num = IO_PA2;
    }
    bsp_gpio_cfg_init(&eq_gpio, io_num);
    if (eq_gpio.sfr != NULL) {
        eq_gpio.sfr[GPIOxDE] |= BIT(eq_gpio.num);
        eq_gpio.sfr[GPIOxFEN] |= BIT(eq_gpio.num);
        huart_eq_init(eq_rx_buf, EQ_BUFFER_LEN);
        huart_rx_set_dir();
        huart_rx_kick();
    }
}
#endif

void eq_dbg_init(void)
{
    printf("eq_dbg_init: %d, %d\n", xcfg_cb.eq_dgb_uart_en, xcfg_cb.eq_uart_sel);
    memset(eq_rx_buf, 0, EQ_BUFFER_LEN);
    memset(&eq_dbg_cb, 0, sizeof(eq_dbg_cb_t));
#if EQ_DBG_IN_UART
    if (xcfg_cb.eq_dgb_uart_en) {
        eq_dbg_huart_init();
    }
#endif // EQ_DBG_IN_UART

}
#endif

#if !EQ_DBG_IN_UART
void huart_rx_done(void) {}
void huart_rx_set_dir(void) {}
void huart_tx_set_dir(void) {}
#endif

struct eq_coef_t {
    u32 param0;                         //频点参数，由工具计算
    u32 param1;
    u32 coef[5];                        //频段参数，由bass_treble_coef_cal计算
};

struct eq_cfg_t {
    struct eq_coef_t *msc_coef[12];
    struct eq_coef_t *mic_coef[8];
};

//#if SYS_BASS_TREBLE_EN
//
//struct eq_coef_t msc_bass = {
//    .param0 = CAL_FIX(0.0284864965),    //BASS:300Hz
//    .param1 = CAL_FIX(0.9990866674),    //BASS:300Hz
//};
//
//struct eq_coef_t msc_treb = {
//    .param0 = CAL_FIX(0.2763541250),    //TREB:3000Hz
//    .param1 = CAL_FIX(0.9100351062),    //TREB:3000Hz
//};
//
//struct eq_coef_t mic_bass = {
//    .param0 = CAL_FIX(0.0284864965),    //BASS:300Hz
//    .param1 = CAL_FIX(0.9990866674),    //BASS:300Hz
//};
//
//struct eq_coef_t mic_treb = {
//    .param0 = CAL_FIX(0.2763541250),    //TREB:3000Hz
//    .param1 = CAL_FIX(0.9100351062),    //TREB:3000Hz
//};
//
//void bsp_bass_treble_init(void)
//{
//    bass_treble_coef_cal(&mic_bass, 0, 0);
//    bass_treble_coef_cal(&mic_treb, 0, 1);
//
//    bass_treble_coef_cal(&msc_bass, 0, 0);
//    bass_treble_coef_cal(&msc_treb, 0, 1);
//}
//
////设置MIC高低音（不会影响其他段EQ，可以边调EQ边调高低音）
//void mic_bass_treble_set(int mode, int gain)
//{
//    if (mode == 0) {            //bass
//        bass_treble_coef_cal(&mic_bass, gain, 0);
//    } else if (mode == 1) {     //treble
//        bass_treble_coef_cal(&mic_treb, gain, 1);
//    }
//#ifdef RES_BUF_EQ_MIC_NORMAL_EQ
//    mic_set_eq_by_res(&RES_BUF_EQ_MIC_NORMAL_EQ, &RES_LEN_EQ_MIC_NORMAL_EQ);    //更新参数
//#endif
//}
//
////设置MUSIC高低音（不会影响其他段EQ，可以边调EQ边调高低音）
//void music_bass_treble_set(int mode, int gain)
//{
//    if (mode == 0) {            //bass
//        bass_treble_coef_cal(&msc_bass, gain, 0);
//    } else if (mode == 1) {     //treble
//        bass_treble_coef_cal(&msc_treb, gain, 1);
//    }
//
//    music_set_eq_by_num(sys_cb.eq_mode);        //更新参数
//}
//#endif

static struct eq_cfg_t eq_cfg = {
//#if SYS_BASS_TREBLE_EN
//    .msc_coef[10] = &msc_bass,
//    .msc_coef[11] = &msc_treb,
//
//    .mic_coef[6] = &mic_bass,
//    .mic_coef[7] = &mic_treb,
//#endif
};

void bsp_eq_init(void)
{
    eq_var_init(&eq_cfg);
//#if SYS_BASS_TREBLE_EN
//    bsp_bass_treble_init();
//#endif
}

////根据index设置12条EQ示例，包含高低音
////Q:0.750000
//struct eq_coef_t tbl_coef[12] = {
//    //param0                    param1
//    { CAL_FIX(0.0029444916),    CAL_FIX(0.9999902462)},     //Band:0(31Hz)
//    { CAL_FIX(0.0059839058),    CAL_FIX(0.9999597162)},     //Band:1(63Hz)
//    { CAL_FIX(0.0151961090),    CAL_FIX(0.9997401793)},     //Band:2(160Hz)
//    { CAL_FIX(0.0379729998),    CAL_FIX(0.9983764898)},     //Band:3(400Hz)
//    { CAL_FIX(0.0946628813),    CAL_FIX(0.9898674722)},     //Band:4(1000Hz)
//    { CAL_FIX(0.3597005044),    CAL_FIX(0.8419530754)},     //Band:5(4000Hz)
//    { CAL_FIX(0.6057018917),    CAL_FIX(0.4177699622)},     //Band:6(8000Hz)
//    { CAL_FIX(0.5060881129),    CAL_FIX(-0.6509365173)},    //Band:7(16000Hz)
//    { CAL_FIX(0.5060881129),    CAL_FIX(-0.6509365173)},    //Band:8(16000Hz)
//    { CAL_FIX(0.5060881129),    CAL_FIX(-0.6509365173)},    //Band:9(16000Hz)
//    { CAL_FIX(0.0284864965),    CAL_FIX(0.9990866674)},     //BASS:300Hz
//    { CAL_FIX(0.2763541250),    CAL_FIX(0.9100351062)},     //TREB:3000Hz
//};
//
//void eq_index_init(void)
//{
//    int i;
//    for (i = 0; i < 12; i++) {
//        eq_cfg.msc_coef[i] = &tbl_coef[i];
//    }
//    for (i = 0; i < 12; i++) {
//        if (i == 10) {           //bass
//            bass_treble_coef_cal(eq_cfg.msc_coef[i], 0, 0);
//        } else if (i == 11) {    //treble
//            bass_treble_coef_cal(eq_cfg.msc_coef[i], 0, 1);
//        } else {                    //eq
//            eq_coef_cal(eq_cfg.msc_coef[i], 0);
//        }
//    }
//}
//
////根据EQ号来设置EQ[11:0]
//void music_set_eq_for_index(u8 index, int gain)
//{
//    if (index > 12 || gain < -12 || gain > 12) {
//        return;
//    }
//    if (index == 10) {           //bass
//        bass_treble_coef_cal(eq_cfg.msc_coef[index], gain, 0);
//    } else if (index == 11) {   //treble
//        bass_treble_coef_cal(eq_cfg.msc_coef[index], gain, 1);
//    } else {                    //eq
//        eq_coef_cal(eq_cfg.msc_coef[index], gain);
//    }
//
//    music_set_eq_by_num(sys_cb.eq_mode);        //更新参数
//}
#endif
