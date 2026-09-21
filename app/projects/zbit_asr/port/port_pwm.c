#include "include.h"
#include "port_pwm.h"

#if PWM_TMR2_EN
#define PWM_1KHZ_TEST_EN    0
const u8 pwm_tmr2_map_sel[] = PWM_TMR2_MAP_SEL;

//#define pwm_cnt 0xf
//int cnt_0 = 0;
//
//u16 TMR2DUTY0_temp = 0x7FFF;

AT(.text.bsp.pwm)
void pwm_gpio_config(u8 io_num)
{
    if(io_num == 0){
        return;
    }

    gpio_t gpio;
    gpio_cfg_init(&gpio, io_num);
    gpio.sfr[GPIOxDE] |= BIT(gpio.num);
    gpio.sfr[GPIOxFEN] |= BIT(gpio.num);
    gpio.sfr[GPIOxDIR] &= ~BIT(gpio.num);
    my_printf("%s,%d\n",__func__,__LINE__);
}

AT(.text.bsp.pwm)
void bsp_pwm_clk_init(void)
{
    CLKGAT0 |= BIT(25);                   //tmr2 clken
}

AT(.text.bsp.pwm)
void bsp_pwm_io_map_init(u8 num0, u8 num1, u8 num2, u8 num3)
{
    if(num0){
        pwm_gpio_config(num0);
        FUNCOUTCON =(FUNCOUTCON & ~(0xf << 0)) | (0 << 0);                                        //PWM0 map
        FUNCOUTMCON = (FUNCOUTMCON & ~(0xf << 0)) | (num0 << 0);
    }

    if(num1){
        pwm_gpio_config(num1);
        FUNCOUTCON =(FUNCOUTCON & ~(0xf << 8)) | (1 << 8);                                        //PWM0 map
        FUNCOUTMCON = (FUNCOUTMCON & ~(0xf << 8)) | (num1 << 8);
    }

    if(num2){
        pwm_gpio_config(num2);
        FUNCOUTCON =(FUNCOUTCON & ~(0xf << 16)) | (2 << 16);                                        //PWM0 map
        FUNCOUTMCON = (FUNCOUTMCON & ~(0xf << 16)) | (num2 << 16);
    }

    if(num3){
        pwm_gpio_config(num3);
        FUNCOUTCON =(FUNCOUTCON & ~(0xf << 24)) | (3 << 24);                                        //PWM0 map
        FUNCOUTMCON = (FUNCOUTMCON & ~(0xf << 24)) | (num3 << 24);
    }
}

AT(.text.bsp.pwm)
void bsp_pwm_map_init(u8 num)
{
    if(num) {
        FUNCMCON1 = (FUNCMCON1 & ~(0xf << 4)) | (num << 4);
        FUNCMCON1 = (FUNCMCON1 & ~(0xf << 8)) | (num << 8);
        FUNCMCON1 = (FUNCMCON1 & ~(0xf << 20)) | (num << 20);
        FUNCMCON1 = (FUNCMCON1 & ~(0xf << 24)) | (num << 24);
        FUNCMCON1 = (FUNCMCON1 & ~(0xf << 28)) | (num << 28);
    }

    if (num == 1) {
        pwm_gpio_config(IO_PA1);                                        //对应G1 G2 G3的顺序配置IO，否则会导致其他错误，G1组的PA2是下载口
        pwm_gpio_config(IO_PA2);
        pwm_gpio_config(IO_PA3);
        pwm_gpio_config(IO_PA4);
        pwm_gpio_config(IO_PA5);
    } else if (num == 2) {
        pwm_gpio_config(IO_PA6);                                        //对应G1 G2 G3的顺序配置IO，否则会导致其他错误，G1组的PA2是下载口
        pwm_gpio_config(IO_PA7);
        pwm_gpio_config(IO_PA8);
        pwm_gpio_config(IO_PA9);
        pwm_gpio_config(IO_PA10);
    } else if (num == 3) {
        pwm_gpio_config(IO_PA11);                                        //对应G1 G2 G3的顺序配置IO，否则会导致其他错误，G1组的PA2是下载口
        pwm_gpio_config(IO_PA12);
        pwm_gpio_config(IO_PA13);
        pwm_gpio_config(IO_PA14);
        pwm_gpio_config(IO_PA15);
    }
}

AT(.text.bsp.pwm)
void bsp_pwm_en(u8 pwm0_en, u8 pwm1_en, u8 pwm2_en, u8 pwm3_en, u8 pwm4_en)
{
    if(pwm0_en) {
        TMR2CON |= BIT(16);
    } else {
        TMR2CON &= ~BIT(16);
    }

    if(pwm1_en) {
        TMR2CON |= BIT(18);
    } else {
        TMR2CON &= ~BIT(18);
    }

    if(pwm2_en) {
        TMR2CON |= BIT(20);
    } else {
        TMR2CON &= ~BIT(20);
    }

    if(pwm3_en) {
        TMR2CON |= BIT(22);
    } else {
        TMR2CON &= ~BIT(22);
    }

    if(pwm4_en){
        TMR2CON |= BIT(24);
    } else {
        TMR2CON &= ~BIT(24);
    }
}

AT(.text.bsp.pwm)
void bsp_pwm_inv_en(u8 pwm0_inv, u8 pwm1_inv, u8 pwm2_inv, u8 pwm3_inv, u8 pwm4_inv)
{
    if(pwm0_inv) {
        TMR2CON |= BIT(17);
    } else {
        TMR2CON &= ~BIT(17);
    }

    if(pwm1_inv) {
        TMR2CON |= BIT(19);
    } else {
        TMR2CON &= ~BIT(19);
    }

    if(pwm2_inv) {
        TMR2CON |= BIT(21);
    } else {
        TMR2CON &= ~BIT(21);
    }

    if(pwm3_inv) {
        TMR2CON |= BIT(23);
    } else {
        TMR2CON &= ~BIT(23);
    }

    if(pwm4_inv) {
        TMR2CON |= BIT(25);
    } else {
        TMR2CON &= ~BIT(25);
    }
}

AT(.text.bsp.pwm)
void bsp_pwm_pre_div(u8 pdivsel)
{
    if (pdivsel == CLOCK_DEVIDE_1) {
        //TMR2CON = (TMR2CON & ~ (BIT(1) * 0x7)) | BIT(1) * 0x0;            //Pre_clock source sel
        TMR2CON = (TMR2CON & ~ (0x7 << 4)) | (0 << 4);                      //Pre_div sel
    } else if (pdivsel == CLOCK_DEVIDE_4) {
        //TMR2CON = (TMR2CON & ~ (BIT(1) * 0x7)) | BIT(1) * 0x1;            //Pre_clock source sel
        TMR2CON = (TMR2CON & ~ (0x7 << 4)) | (2 << 4);                      //Pre_div sel
    } else if(pdivsel == CLOCK_DEVIDE_8) {
        //TMR2CON = (TMR2CON & ~ (BIT(1) * 0x7)) | BIT(1) * 0x1;            //Pre_clock source sel
        TMR2CON = (TMR2CON & ~ (0x7 << 4)) | (3 << 4);                      //Pre_div sel
    } else if(pdivsel == CLOCK_DEVIDE_16) {
        //TMR2CON = (TMR2CON & ~ (BIT(1) * 0x7)) | BIT(1) * 0x1;            //Pre_clock source sel
        TMR2CON = (TMR2CON & ~ (0x7 << 4)) | (4 << 4);                      //Pre_div sel
    } else if(pdivsel == CLOCK_DEVIDE_32) {
        //TMR2CON = (TMR2CON & ~ (BIT(1) * 0x7)) | BIT(1) * 0x1;            //Pre_clock source sel
        TMR2CON = (TMR2CON & ~ (0x7 << 4)) | (5 << 4);                      //Pre_div sel
    } else if(pdivsel == CLOCK_DEVIDE_64) {
        //TMR2CON = (TMR2CON & ~ (BIT(1) * 0x7)) | BIT(1) * 0x1;            //Pre_clock source sel
        TMR2CON = (TMR2CON & ~ (0x7 << 4)) | (6 << 4);                      //Pre_div sel
    }
}

AT(.text.bsp.pwm)
void bsp_pwm_mode_sel(PWM_TMR_MODE_TYPEDEF_SEL mode_sel)
{
    TMR2CON = (TMR2CON & ~(0x3 << 14)) | (mode_sel << 14);
}

AT(.text.bsp.pwm)
void bsp_pwm_clk_sel(PWM_PRE_SEL_SOURCE_TYPEDEF clk_sel)
{
    TMR2CON = (TMR2CON & ~(0x7 << 1)) | (clk_sel  << 1);
}

AT(.text.bsp.pwm)
void bsp_tmr2pwm_init(pwm_init_typedef *pwm_cfg)
{
    pwm_init_typedef *config = pwm_cfg;

    if(config) {
        bsp_pwm_clk_init();
        //按组映射
        bsp_pwm_map_init(config->group_num);
        //PWM使能
        bsp_pwm_en(config->io_state.pwm0_io_state, config->io_state.pwm1_io_state, config->io_state.pwm2_io_state, config->io_state.pwm3_io_state, config->io_state.pwm4_io_state);
        //是否反相
        bsp_pwm_inv_en(config->inv_state.pwm0_inv_state, config->inv_state.pwm1_inv_state, config->inv_state.pwm2_inv_state, config->inv_state.pwm3_inv_state, config->inv_state.pwm4_inv_state);
        //分频
        bsp_pwm_pre_div(config->pclk_div);
        //mode
        bsp_pwm_mode_sel(config->mode_sel);
        //时钟源选择
        bsp_pwm_clk_sel(config ->clk_sel);
        //周期
        TMR2PR = (config->tmrpr);
        //duty
        TMR2DUTY0 = (config->tmrduty.tmr0duty);
        TMR2DUTY1 = (config->tmrduty.tmr1duty);
        TMR2DUTY2 = (config->tmrduty.tmr2duty);
        TMR2DUTY3 = (config->tmrduty.tmr3duty);
        TMR2DUTY4 = (config->tmrduty.tmr4duty);

        TMR2CNT = -1;
        //TMR使能
        TMR2CON |= BIT(0);                                                //Timer enable
    }
    tmr2pwm_irq_init();
}

//初始化和中断
AT(.com_text.isr.pwm)
void tmr2pwm_isr(void)
{
    if(TMR2CON & BIT(9)){
        TMR2CPND |= BIT(9);
//test
//        my_printf(str_test1);
//        if(cnt_0 < TMR2DUTY0_temp / pwm_cnt){
//            TMR2DUTY0 = TMR2DUTY0 - pwm_cnt;
//            cnt_0++;
//            my_printf(str_test2,TMR2DUTY0);
//
//            }
//            if(cnt_0 >= TMR2DUTY0_temp / pwm_cnt){
//                TMR2DUTY0 = TMR2DUTY0 + pwm_cnt;
//                cnt_0++;
//                my_printf(str_test3,TMR2DUTY0);
//                if(cnt_0 >= 2 *(TMR2DUTY0_temp / pwm_cnt)){
//                    cnt_0 = 0;
//                }
//            }
    }
}

///TMR2PWM初始化函数
AT(.text.bsp.pwm)
void tmr2pwm_cfg_init(void)
{
    pwm_init_typedef pwm_cfg_init;
    memset(&pwm_cfg_init, 0, sizeof(pwm_cfg_init));

#if PWM_TMR2_IO_MAP_EN
    //单IO映射，最多支持映射四个IO
    bsp_pwm_io_map_init(pwm_tmr2_map_sel[0],pwm_tmr2_map_sel[1],pwm_tmr2_map_sel[2],pwm_tmr2_map_sel[3]);              //使用PA1产生PWM波，需将setting里的 MIC偏置电路配置改成"MIC省电阻电容"
#else
    #if (PWM_TMR2_GRP_MAP_SEL == PWM_MAPPING_G1)
        pwm_cfg_init.group_num = PWM_MAPPING_G1;                        //G1: PWM0~4 map to PA1  PA2  PA3  PA4  PA5
    #elif (PWM_TMR2_GRP_MAP_SEL == PWM_MAPPING_G2)
        pwm_cfg_init.group_num = PWM_MAPPING_G2;                        //G2: PWM0~4 map to PA6  PA7  PA8  PA9  PA10
    #elif (PWM_TMR2_GRP_MAP_SEL == PWM_MAPPING_G3)
        pwm_cfg_init.group_num = PWM_MAPPING_G3;                        //G3: PWM0~4 map to PA11 PA12 PA13 PA14 PA115
    #endif
#endif // PWM_TMR2_IO_MAP_EN
    //参数配置
    //PWM使能
    pwm_cfg_init.io_state.pwm0_io_state        = PWM_IO_EN;
    pwm_cfg_init.io_state.pwm1_io_state        = PWM_IO_EN;
    pwm_cfg_init.io_state.pwm2_io_state        = PWM_IO_EN;
    pwm_cfg_init.io_state.pwm3_io_state        = PWM_IO_EN;
    pwm_cfg_init.io_state.pwm4_io_state        = PWM_IO_EN;
    //是否反相invert
    pwm_cfg_init.inv_state.pwm0_inv_state      = PWM_INV_EN;
    pwm_cfg_init.inv_state.pwm1_inv_state      = PWM_INV_EN;
    pwm_cfg_init.inv_state.pwm2_inv_state      = PWM_INV_EN;
    pwm_cfg_init.inv_state.pwm3_inv_state      = PWM_INV_EN;
    pwm_cfg_init.inv_state.pwm4_inv_state      = PWM_INV_EN;
    //占空比duty (范围是 0~0xffff, duty)
    pwm_cfg_init.tmrduty.tmr0duty              = 0x3fff;
    pwm_cfg_init.tmrduty.tmr1duty              = 0x5fff;
    pwm_cfg_init.tmrduty.tmr2duty              = 0x7fff;
    pwm_cfg_init.tmrduty.tmr3duty              = 0x9fff;
    pwm_cfg_init.tmrduty.tmr4duty              = 0xafff;
    //分频选择pre_sel (注意系统时钟不能二分频。另外，如果需要分频，先把下面时钟源选成 SOURCE_SYSCLK_PRE_DIV)
    pwm_cfg_init.pclk_div                      = CLOCK_DEVIDE_1;
    //timer or PWM mode 默认PWM模式不需要修改
    pwm_cfg_init.mode_sel                      = TIMER_PWM_MODE;
    //PWM时钟源选择
    pwm_cfg_init.clk_sel                       = SOURCE_SYSCLK;                            //注意选择其他时钟时，注意减小TMRPR
    //周期PR 输出频率 = 选择时钟频率/PDIV/(TMRPR+1) (PWM周期范围时0~0xffff, PWM周期必须大于PWM的占空比)
    pwm_cfg_init.tmrpr                         = 0xffff;
#if PWM_1KHZ_TEST_EN
    //系统时钟192M，PWM频率 = 系统时钟(192M) / （PWM周期(tmrpr)+1） / PWM分频(clock_pre_sel)
    // 192000000 / (47999 + 1)/4 = 1000HZ
    pwm_cfg_init.tmrduty.tmr0duty              = 0x5dc0;  //周期的一半，占空比为50%
    pwm_cfg_init.tmrduty.tmr1duty              = 0x5dc0;
    pwm_cfg_init.tmrduty.tmr2duty              = 0x5dc0;
    pwm_cfg_init.tmrduty.tmr3duty              = 0x5dc0;
    pwm_cfg_init.tmrduty.tmr4duty              = 0x5dc0;
    pwm_cfg_init.pclk_div                      = CLOCK_DEVIDE_4;
    pwm_cfg_init.clk_sel                       = SOURCE_SYSCLK_PRE_DIV;
    pwm_cfg_init.tmrpr                         = 0xbb7f;
#endif // PWM_1KHZ_TEST_EN

    //TMR2中断使能
    TMR2CON |= BIT(7);

    bsp_tmr2pwm_init(&pwm_cfg_init);
}

#endif // PWM_TMR2_EN

void timer_pwm_init(u8 io1, u8 io2, u8 io3, u8 io4, u32 fre, u32 duty)
{
    u32 duty_num = 0;
    u32 fre_num  = 0;
    pwm_init_typedef pwm_cfg_init;
    memset(&pwm_cfg_init, 0, sizeof(pwm_cfg_init));

    bsp_pwm_io_map_init(io1, io2, io3, io4);
    //参数配置
    //PWM使能
    pwm_cfg_init.io_state.pwm0_io_state        = PWM_IO_EN;
    pwm_cfg_init.io_state.pwm1_io_state        = PWM_IO_EN;
    pwm_cfg_init.io_state.pwm2_io_state        = PWM_IO_EN;
    pwm_cfg_init.io_state.pwm3_io_state        = PWM_IO_EN;
    pwm_cfg_init.io_state.pwm4_io_state        = PWM_IO_EN;
    //是否反相invert
    pwm_cfg_init.inv_state.pwm0_inv_state      = PWM_INV_DIS;
    pwm_cfg_init.inv_state.pwm1_inv_state      = PWM_INV_DIS;
    pwm_cfg_init.inv_state.pwm2_inv_state      = PWM_INV_DIS;
    pwm_cfg_init.inv_state.pwm3_inv_state      = PWM_INV_DIS;
    pwm_cfg_init.inv_state.pwm4_inv_state      = PWM_INV_DIS;
    //分频选择pre_sel (注意系统时钟不能二分频。另外，如果需要分频，先把下面时钟源选成 SOURCE_SYSCLK_PRE_DIV)
    pwm_cfg_init.pclk_div                      = CLOCK_DEVIDE_4;
    //timer or PWM mode 默认PWM模式不需要修改
    pwm_cfg_init.mode_sel                      = TIMER_PWM_MODE;
    //PWM时钟源选择
    pwm_cfg_init.clk_sel                       = SOURCE_SYSCLK_PRE_DIV;                            //注意选择其他时钟时，注意减小TMRPR
    //周期PR 输出频率 = 选择时钟频率/PDIV/(TMRPR+1) (PWM周期范围时0~0xffff, PWM周期必须大于PWM的占空比)
    fre_num = get_sysclk_nhz()/(4*fre)-1;
    pwm_cfg_init.tmrpr                         = fre_num;
    //占空比duty (范围是 0~0xffff, duty)
    if(duty == 100)
    {
        duty_num = fre_num;
    }
    else
    {
        duty_num = (fre_num+1)*duty/100;
    }
    pwm_cfg_init.tmrduty.tmr0duty              = duty_num;
    pwm_cfg_init.tmrduty.tmr1duty              = duty_num;
    pwm_cfg_init.tmrduty.tmr2duty              = duty_num;
    pwm_cfg_init.tmrduty.tmr3duty              = duty_num;
    pwm_cfg_init.tmrduty.tmr4duty              = duty_num;
    //TMR2中断使能
    TMR2CON |= BIT(7);

    bsp_tmr2pwm_init(&pwm_cfg_init);
}

AT(.com_text.pwm)
void set_timer_pwm_duty(u8 num, u32 duty)
{
    int duty_num = 0;
    bool inv_flag = false;
    if(duty == 0)
    {
        duty_num = TMR2PR;
        inv_flag = true;
    }
    else
    {
        duty_num = (TMR2PR+1)*duty/100;
    }

    if(num == 0)
    {
        TMR2DUTY0 = duty_num;
        if(inv_flag) {
            TMR2CON |= BIT(17);
            inv_flag = false;
        } else {
            TMR2CON &= ~BIT(17);
        }
    }
    else if(num == 1)
    {
        TMR2DUTY1 = duty_num;
        if(inv_flag) {
            TMR2CON |= BIT(19);
            inv_flag = false;
        } else {
            TMR2CON &= ~BIT(19);
        }
    }
    else if(num == 2)
    {
        TMR2DUTY2 = duty_num;
        if(inv_flag) {
            TMR2CON |= BIT(21);
            inv_flag = false;
        } else {
            TMR2CON &= ~BIT(21);
        }
    }
    else if(num == 3)
    {
        TMR2DUTY3 = duty_num;
        if(inv_flag) {
            TMR2CON |= BIT(23);
            inv_flag = false;
        } else {
            TMR2CON &= ~BIT(23);
        }
    }
    else
    {
        TMR2DUTY4 = duty_num;
        if(inv_flag) {
            TMR2CON |= BIT(25);
            inv_flag = false;
        } else {
            TMR2CON &= ~BIT(25);
        }
    }
}


