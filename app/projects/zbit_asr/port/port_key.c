#include "include.h"

//与配置工具下拉选项一一对应
AT(.rodata.key.table)
const u8 key_config_table[] = {
    NO_KEY,                     //None
    KEY_PLAY_POWER,             //PP/POWER
    KEY_PLAY,                   //P/P
    KEY_POWER,                  //POWER
    KEY_NEXT_VOL_UP,            //NEXT/VOL+
    KEY_VOL_UP_NEXT,            //VOL+/NEXT
    KEY_PREV_VOL_DOWN,          //PREV/VOL-
    KEY_VOL_DOWN_PREV,          //VOL-/PREV
    KEY_VOL_UP,                 //VOL+
    KEY_VOL_DOWN,               //VOL-
    KEY_NEXT,                   //NEXT
    KEY_PREV,                   //PREV
    KEY_MODE,                   //MODE
    KEY_PLAY_MODE,              //PP/MODE
    KEY_HSF,                    //HSF
    KEY_EQ,                     //EQ
    KEY_REPEAT,                 //REPEAT
    KEY_MUTE,                   //MUTE
    KEY_MODE_POWER,             //MODE/POWER
    KEY_REC,
    KEY_PLAY_HSF,               //PLAY/HSF
    KEY_AB_PLAY,                //A-B
};

#if USER_MULTI_PRESS_EN
AT(.com_text.key)
u8 check_key_double_configure(u16 key_val)
{
    int i;
    u8 *dblkey = dblkey_num;

    if (xcfg_cb.key_multi_config_en) {
        for (i = 0; i < 5; i++) {
            if (key_val == ((u16)dblkey[i] | KEY_SHORT_UP)) {
                return 2;
            }
        }
        return 1;
    }
    return 0;
}
#endif

#if (USER_ADKEY || USER_ADKEY_MUX_SDCLK)
#if ADKEY_PU10K_EN
/******************************************************************
*                   内部10K上拉的table表
* 1、不复用SDCLK时，最多支持7个按键，按键电阻0R, 2K, 5.1K, 10K, 22K, 56K, 150K
* 2、复用SDCLK时, 需要从2K电阻的按键开始，最多6个按键
* 3、150K电阻也可以用于插入检测
*******************************************************************/
AT(.com_rodata.adkey.table)
const adkey_tbl_t adkey_table[] = {
    {0x0A, KEY_PLAY},           //PLAY          0R
    {0x3F, KEY_PREV_VOL_DOWN},  //PREV/VOL-     2K
    {0x6C, KEY_NEXT_VOL_UP},    //NEXT/VOL+     5.1K,
    {0x97, KEY_MODE},           //MODE          10K,
    {0xC1, KEY_HSF},            //HSF           22K,
    {0xE7, KEY_REPEAT},         //REPEAT        56K,
    {0xF7, KEY_REC},            //REC           150K
    {0xFF, NO_KEY},             //              END
};
#else

#if USER_ADKEY_MUX_LED
/******************************************************************
*                   ADKEY复用LED的table表（外部100K上拉, 蓝灯）
* 1、最多支持7个按键
*******************************************************************/
AT(.com_rodata.adkey.table)
const adkey_tbl_t adkey_table[] = {
    {0x76, KEY_PLAY},           //PLAY          75K
    {0x8C, KEY_PREV},           //PREV          100K
    {0xA1, KEY_NEXT},           //NEXT          150K
    {0xB6, KEY_MODE},           //MODE          200K
    {0xCE, KEY_HSF},            //HSF           330K,
    {0xE0, KEY_VOL_DOWN},       //VOL-          560K,
    {0xF3, KEY_VOL_UP},         //VOL+          1M,
    {0xFF, NO_KEY},             //              END
};
#else
/******************************************************************
*                   外部10K上拉的table表
* 1、最多支持12个按键
* 2、复用SDCLK时, 需要从2K电阻的按键开始或ADKEY先串个10K电阻到IO
* 3、100K电阻也可以用于插入检测
*******************************************************************/
AT(.com_rodata.adkey.table)
const adkey_tbl_t adkey_table[] = {
    {0x0F, KEY_PLAY},           //PLAY          0R
    {0x24, KEY_PREV},           //PREV          1K
    {0x3E, KEY_NEXT},           //NEXT          2.2K
    {0x5A, KEY_MODE},           //MODE          4.7K
    {0x75, KEY_HSF},            //HSF           6.8K,
    {0x88, KEY_VOL_DOWN},       //VOL-          10K,
    {0xA4, KEY_VOL_UP},         //VOL+          15K,
    #if FUNC_REC_EN
    {0xC4, KEY_REC},            //REC           24K,
    #else
    {0xC4, KEY_EQ},             //EQ            24K,
    #endif
    {0xD0, KEY_REPEAT},         //REPEAT        33K,
    {0xDA, KEY_VOL_DOWN_PREV},  //MINUS         56K,
    {0xE6, KEY_VOL_UP_NEXT},    //PLUS          78K
    {0xF4, KEY_REC},            //REC           100K,
    {0xFF, NO_KEY},             //              END
};
#endif // USER_ADKEY_MUX_LED
#endif // ADKEY_PU10K_EN
#endif

#if USER_ADKEY2
///最多支持12个按键, 以0xff结束
AT(.com_text.adkey2.table)
const adkey_tbl_t adkey2_table[] = {
    {0x0F, KEY_NUM_0},
    {0x24, KEY_NUM_1},
    {0x3E, KEY_NUM_2},
    {0x5A, KEY_NUM_3},
    {0x75, KEY_NUM_4},
    {0x88, KEY_NUM_5},
    {0xA4, KEY_NUM_6},
    {0xC4, KEY_NUM_7},
    {0xD0, KEY_NUM_8},
    {0xDA, KEY_NUM_9},
    {0xE6, NO_KEY},
    {0xF4, NO_KEY},
    {0xFF, NO_KEY},
};
#endif // USER_ADKEY2



#if USER_IOKEY
//工具配置的IO初始化
gpio_t iokey_gpio[5] AT(.buf.key);
u8 iokey_num[6] AT(.buf.key);

//工具配置的IO初始化
AT(.text.key.init)
void iokey_cfg_port_init(gpio_t *g)
{
    if (g->sfr == NULL) {
        return;
    }
    g->sfr[GPIOxDE] |= BIT(g->num);
#if IOKEY_HIGH_TRIGGER
    g->sfr[GPIOxPD] |= BIT(g->num);
#else
    g->sfr[GPIOxPU] |= BIT(g->num);
#endif
    g->sfr[GPIOxDIR] |= BIT(g->num);
}

AT(.com_text.port.key)
bool iokey_cfg_port_pressed(gpio_t *g)
{
    if (g->sfr == NULL) {
        return false;
    }
#if IOKEY_HIGH_TRIGGER
    return ((g->sfr[GPIOx] & BIT(g->num)));
#else
    return (!(g->sfr[GPIOx] & BIT(g->num)));
#endif
}

AT(.com_text.port.key)
void iokey_cfg_port_out_low(gpio_t *g)
{
    if (g->sfr == NULL) {
        return;
    }
    g->sfr[GPIOxDE] |= BIT(g->num);
    g->sfr[GPIOxDIR] &= ~BIT(g->num);
    g->sfr[GPIOxCLR] = BIT(g->num);
}

AT(.com_text.port.key)
bool iokey_cfg_midkey_pressed(gpio_t *g0, gpio_t *g1)
{
    bool pressed_flag = false;

    iokey_cfg_port_out_low(g0);
    delay_us(6);
    if (iokey_cfg_port_pressed(g1)) {
        pressed_flag = true;
    }
    iokey_cfg_port_init(g0);
    return pressed_flag;
}

AT(.text.key.init)
void io_key_init(void)
{
    //配置工具是否使能了IOKEY？
    if (!xcfg_cb.user_iokey_en) {
        return;
    }

    bsp_gpio_cfg_init(&iokey_gpio[0], xcfg_cb.iokey_io0);
    bsp_gpio_cfg_init(&iokey_gpio[1], xcfg_cb.iokey_io1);
    bsp_gpio_cfg_init(&iokey_gpio[2], xcfg_cb.iokey_io2);
    bsp_gpio_cfg_init(&iokey_gpio[3], xcfg_cb.iokey_io3);
    bsp_gpio_cfg_init(&iokey_gpio[4], xcfg_cb.iokey_io4);
    for (int i = 0; i < 5; i++) {
        iokey_cfg_port_init(&iokey_gpio[i]);
    }

    iokey_num[0] = key_config_table[xcfg_cb.iokey_num0];
    iokey_num[1] = key_config_table[xcfg_cb.iokey_num1];
    iokey_num[2] = key_config_table[xcfg_cb.iokey_num2];
    iokey_num[3] = key_config_table[xcfg_cb.iokey_num3];
    iokey_num[4] = key_config_table[xcfg_cb.iokey_num4];
    iokey_num[5] = key_config_table[xcfg_cb.iokey_midkey_num];
}

AT(.com_text.port.key)
u8 get_iokey(void)
{
    u8 i;

    //配置工具是否使能了IOKEY？
    if (!xcfg_cb.user_iokey_en) {
        return NO_KEY;
    }

    for (i = 0; i < 5; i++) {
        if (iokey_cfg_port_pressed(&iokey_gpio[i])) {
            return iokey_num[i];
        }
    }
    if (iokey_num[5] && iokey_cfg_midkey_pressed(&iokey_gpio[0], &iokey_gpio[1])) {
        return iokey_num[5];
    }
    return NO_KEY;

}
#endif // USER_IOKEY

#if ((POWER_ON_FALL_NUM != 0) || (POWER_ON_RISE_NUM != 0))
void poweron_key_init(void)
{
#if (POWER_ON_FALL_NUM != 0)
    u8 fall_io_temp[POWER_ON_FALL_NUM] = POWER_ON_FALL_IO;
    for (int i = 0; i < POWER_ON_FALL_NUM; i++) {
        pwron_fall_set_do(fall_io_temp[i]);
    }
#endif
#if (POWER_ON_RISE_NUM != 0)
    u8 rise_io_temp[POWER_ON_RISE_NUM] = POWER_ON_RISE_IO;
    for (int i = 0; i < POWER_ON_RISE_NUM; i++) {
        pwron_rise_set_do(rise_io_temp[i]);
    }
#endif
}

void poweron_key_exit(void)
{
    set_poweron_flag(false);
}

u8 get_poweron_key(void)
{
    u32 io_sta = 0;
    io_sta = (GPIOB << 16);
    io_sta = (io_sta | (GPIOA & 0xffff));
#if (POWER_ON_FALL_NUM != 0)
    u8 fall_io_temp[POWER_ON_FALL_NUM] = POWER_ON_FALL_IO;
    for (int i = 0; i < POWER_ON_FALL_NUM; i++) {
        if((io_sta & BIT(fall_io_temp[i] - 1)) == 0){
            return fall_io_temp[i];
        }
    }
#endif
#if (POWER_ON_RISE_NUM != 0)
    u8 rise_io_temp[POWER_ON_RISE_NUM] = POWER_ON_RISE_IO;
    for (int i = 0; i < POWER_ON_RISE_NUM; i++) {
        if((io_sta & BIT(rise_io_temp[i] - 1)) == BIT(rise_io_temp[i] - 1)){
            return rise_io_temp[i];
        }
    }
#endif
    return 0;
}
#endif

#if MUL_PWRON_IO_EN
AT(.com_text.mulpwr.table1)
const u8 mulpwr_fall_group[] = MUL_PWRON_FALL_GROUP;
AT(.com_text.mulpwr.table2)
const u8 mulpwr_rise_group[] = MUL_PWRON_RISE_GROUP;

void mulpwr_fall_init_do(u8 io_num)
{
    // my_printf("mulpwr io_num: %d\n", io_num);
    if(io_num > IO_PA15){
        GPIOBFEN &= ~(BIT(io_num-1-IO_PA15));
        GPIOBDE  |=  (BIT(io_num-1-IO_PA15));
        GPIOBDIR |=  (BIT(io_num-1-IO_PA15));
        GPIOBPU  |=  (BIT(io_num-1-IO_PA15));
    }else{
        GPIOAFEN &= ~(BIT(io_num-1));
        GPIOADE  |=  (BIT(io_num-1));
        GPIOADIR |=  (BIT(io_num-1));
        GPIOAPU  |=  (BIT(io_num-1));
    }
}

void mulpwr_rise_init_do(u8 io_num)
{
    if(io_num > IO_PA15){
        GPIOBFEN &= ~(BIT(io_num-1-IO_PA15));
        GPIOBDE  |=  (BIT(io_num-1-IO_PA15));
        GPIOBDIR |=  (BIT(io_num-1-IO_PA15));
        GPIOBPD  |=  (BIT(io_num-1-IO_PA15));
    }else{
        GPIOAFEN &= ~(BIT(io_num-1));
        GPIOADE  |=  (BIT(io_num-1));
        GPIOADIR |=  (BIT(io_num-1));
        GPIOAPD  |=  (BIT(io_num-1));
    }
}

void mulpwr_key_init(void)
{
#if (MUL_PWRON_FALL_NUM != 0)
    for (int i = 0; i < MUL_PWRON_FALL_NUM; i++) {
        mulpwr_fall_init_do(mulpwr_fall_group[i]);
    }
#endif

#if (MUL_PWRON_RISE_NUM != 0)
    for (int i = 0; i < MUL_PWRON_RISE_NUM; i++) {
        mulpwr_rise_init_do(mulpwr_rise_group[i]);
    }
#endif

}

AT(.com_text.mulpwr.table)
const u8 mulpwr_table[] = {
    KEY_NUM_0,
    KEY_NUM_1,
    KEY_NUM_2,
    KEY_NUM_3,
    KEY_NUM_4,
    KEY_NUM_5,
    KEY_NUM_6,
    KEY_NUM_7,
    KEY_NUM_8,
    KEY_NUM_9,
    NO_KEY,
    NO_KEY,
    NO_KEY,
};

AT(.com_text.mulpwr_key)
u8 get_mulpwr_key(void)
{
    u32 io_sta = 0;
    io_sta = (GPIOB << 16);
    io_sta = (io_sta | (GPIOA & 0xffff));
#if (MUL_PWRON_FALL_NUM != 0)
    for (int i = 0; i < MUL_PWRON_FALL_NUM; i++) {
        if((io_sta & BIT(mulpwr_fall_group[i] - 1)) == 0){
            return mulpwr_table[i];
        }
    }
#endif
#if (MUL_PWRON_RISE_NUM != 0)
    for (int i = 0; i < MUL_PWRON_RISE_NUM; i++) {
        if((io_sta & BIT(mulpwr_rise_group[i] - 1)) == BIT(mulpwr_rise_group[i] - 1)){
            return mulpwr_table[i];
        }
    }
#endif
    return NO_KEY;
}
#endif
