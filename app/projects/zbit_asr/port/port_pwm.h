#ifndef _PORT_PWM_H
#define _PORT_PWM_H

#define   PWM_MAPPING_G1 1
#define   PWM_MAPPING_G2 2
#define   PWM_MAPPING_G3 3

typedef enum{
    PWM_IO_DIS      =0x0,
    PWM_IO_EN       =0x1,
}PWM_GPIO_STATE_TYPEDEF;

typedef enum{
    PWM_INV_DIS      =0x0,
    PWM_INV_EN       =0x1,
}PWM_INV_STATE_TYPEDEF;

typedef enum{
    CLOCK_DEVIDE_1 = 0x0,
    CLOCK_DEVIDE_4 = 0x2,
    CLOCK_DEVIDE_8,
    CLOCK_DEVIDE_16,
    CLOCK_DEVIDE_32,
    CLOCK_DEVIDE_48,
    CLOCK_DEVIDE_64,
}PWM_PRE_SEL_DIVIDE_TYPEDEF;

typedef enum{
    SOURCE_SYSCLK = 0x0,
    SOURCE_SYSCLK_PRE_DIV = 0x1,
    SOURCE_XOSC24M = 0x2,
    SOURCE_RC2M   = 0x4,
}PWM_PRE_SEL_SOURCE_TYPEDEF;

typedef enum{
    TIMER_PWM_MODE = 0x0,
    CAPTURE_RISING_MODE,
    CAPTURE_FALLING_MODE,
    CAPTURE__RISING_OR_FALLING_MODE,
}PWM_TMR_MODE_TYPEDEF_SEL;

//PWM_DUTY
typedef struct{
    u16 tmr0duty;
    u16 tmr1duty;
    u16 tmr2duty;
    u16 tmr3duty;
    u16 tmr4duty;
}PWM_TMRDUTY_TYPEDEF;

//PWM_IO_SEL
typedef struct{
    PWM_GPIO_STATE_TYPEDEF pwm0_io_state;
    PWM_GPIO_STATE_TYPEDEF pwm1_io_state;
    PWM_GPIO_STATE_TYPEDEF pwm2_io_state;
    PWM_GPIO_STATE_TYPEDEF pwm3_io_state;
    PWM_GPIO_STATE_TYPEDEF pwm4_io_state;
}PWM_IO_TYPEDEF;

//PWM_INV_SEL
typedef struct{
    PWM_INV_STATE_TYPEDEF pwm0_inv_state;
    PWM_INV_STATE_TYPEDEF pwm1_inv_state;
    PWM_INV_STATE_TYPEDEF pwm2_inv_state;
    PWM_INV_STATE_TYPEDEF pwm3_inv_state;
    PWM_INV_STATE_TYPEDEF pwm4_inv_state;
}PWM_INV_TYPEDEF;

typedef struct{
    PWM_IO_TYPEDEF io_state;
    u8 group_num;
    PWM_INV_TYPEDEF inv_state;
    PWM_TMRDUTY_TYPEDEF tmrduty;
    PWM_PRE_SEL_DIVIDE_TYPEDEF pclk_div;
    PWM_TMR_MODE_TYPEDEF_SEL mode_sel;
    PWM_PRE_SEL_SOURCE_TYPEDEF clk_sel;
    u32 tmrpr;
}pwm_init_typedef;
extern pwm_init_typedef pwm_cfg_init;

void pwm_gpio_config(u8 io_num);
void bsp_tmr2pwm_init(pwm_init_typedef *pwm_cfg);
void bsp_pwm_io_map_init(u8 num0, u8 num1, u8 num2, u8 num3);
void gpio_cfg_init(gpio_t *g, u8 io_num);
void tmr2pwm_irq_init(void);
#endif
