#ifndef _BSP_GPIO_H
#define _BSP_GPIO_H

enum {
    GPIOxSET    =   0,
    GPIOxCLR,
    GPIOx,
    GPIOxDIR,
    GPIOxDE,
    GPIOxFEN,
    GPIOxDRV,
    GPIOxPU,
    GPIOxPD,
    GPIOxPU500K,
    GPIOxPD500K,
};

typedef struct {
    psfr_t sfr;             //GPIO SFR ADDR
    u8 num;
    u8 type;                //type = 1,高压IO,可以上拉10k或下拉60k, type = 0,普通IO,可上拉10k/500k或下拉60k/500k.
    u16 pin;
} gpio_t;

#define bsp_gpio_cfg_init(x, y)         gpio_cfg_init(x, y)

void gpio_cfg_init(gpio_t *g, u8 io_num);       //根据GPIO number初始化GPIO结构体
void wakeup_disable(void);
u8 get_adc_gpio_num(u8 adc_ch);
void wakeup_gpio_config(u8 io_num, u8 edge, u8 pupd_sel);    //任意GPIO的边沿唤醒配置, 参数edge 0下降沿, 1上升沿,  参数pupd  0:不开内部上下拉, 1:开内部上拉, 2:开内部下拉
void wakeup_wko_config(void);                   //配置WKO唤醒
void wakeup_udisk_config(u8 edge);              //配置U盘唤醒 0:上升沿唤醒, 1：下降沿唤醒
u32 wakeup_get_status(void);                    //获取唤醒状态，公共区函数
void adcch_io_pu10k_enable(u8 adc_ch);
u16 get_cur_vddio_vol(void);
void vddio_voltage_update(u16 vbat_val);
#endif // _BSP_GPIO_H
