#include "system/includes.h"
#include "app_timer.h"
#include "gpio.h"
//#include "clock.h"
//#include "common.h"
#include "ws2812.h"
#include "typedef.h"

#define LED_NUM_MAX         5
#define _COUNTOF(param)   ((int32_t)(sizeof(param)/(sizeof(*(param)))))


typedef uint8_t brightness_t;
typedef brightness_t rgb_t[3];

extern void led_spi_init(void);
extern void led_spi_rgb_to_24byte(u8 r, u8 g, u8 b, u8 *buf, int idx);
extern void led_spi_send_rgbbuf(u8 *rgb_buf, u16 led_num);
extern void led_spi_send_rgbbuf_isr(u8 *rgb_buf, u16 led_num);
extern void wdt_clear();


static void udelay(u32 usec)
{
    JL_TIMER0->CON = BIT(14);
    JL_TIMER0->CNT = 0;
#if(TCFG_CLOCK_SYS_SRC==SYS_CLOCK_INPUT_PLL_RCL)//省晶振，注意时钟源
    JL_TIMER0->PRD = clk_get("lsb") / 1000000L  * usec; //1us
    JL_TIMER0->CON = BIT(0); // clk
#else
    JL_TIMER0->PRD = clk_get("timer") / 1000000L  * usec; //1us
    JL_TIMER0->CON = BIT(0) | BIT(3); //sys clk
#endif
    while ((JL_TIMER0->CON & BIT(15)) == 0);
    JL_TIMER0->CON = BIT(14);
}

#define DEFAULT_BRIGHTNESS_LV      3
#define BRIGHTNESS_LV(x)  (10*((x)*(x)) + 0*(x) + 5)

static const brightness_t br_levels[] = {
    0,
    BRIGHTNESS_LV(1),
    BRIGHTNESS_LV(2),
    BRIGHTNESS_LV(3),
    BRIGHTNESS_LV(4),
    BRIGHTNESS_LV(5),
};

typedef struct _ws2812_handle
{
    bool on;
    uint8_t color_index;
    uint8_t br_level_index;
}ws2812_handle_t;
static ws2812_handle_t ws2812_handle[LED_NUM_MAX] = {0};
#define  GET_HANDLE_INDEX(handle, base)  ((handle)-(base))

enum _COLOR
{
    RED,
    GREEN,
    BLUE,
    YELLOW,
    PURPLE,
    CYAN,
    WHITE,
};

static const rgb_t rgb_table[] = {
    { 0xff, 0x00, 0x00 },   //Red
    { 0x00, 0xff, 0x00 },  	//Green
    { 0x00, 0x00, 0xff },  	//Blue
    { 0xff, 0xff, 0x00 },  	//Yellow
    { 0xff, 0x00, 0xff },  	//Purple
    { 0x00, 0xff, 0xff },  	//Cyan
    { 0xff, 0xff, 0xff },  	//White
};

static u8 spi_dat_buf[24 * (LED_NUM_MAX+1)] __attribute__((aligned(4)));
// void dump_buf(u8 *buf, int len)
// {
//     int i=0;
//     putchar('\n');
//     for (int i = 0; i < len; i++) {
//         u8 tmp = buf[i] / 16;
//         if (tmp < 10) {
//             putchar(tmp + '0');
//         } else {
//             putchar(tmp - 10 + 'A');
//         }
//         tmp = buf[i] % 16;
//         if (tmp < 10) {
//             putchar(tmp + '0');
//         } else {
//             putchar(tmp - 10 + 'A');
//         }
//         putchar(0x20);
//         if ((i+1)%24==0)
//         {
//             putchar('\n');
//         }
//     }
//     putchar('\n');
// }

//发送一个灯的数据
static void ws2812_send_rgb(ws2812_handle_t* handle, rgb_t rgb)
{
    uint8_t index = GET_HANDLE_INDEX(handle, ws2812_handle);
    if (index > LED_NUM_MAX)
    {
        printf("error led index:%d\n", index);
        return;
    }
    led_spi_rgb_to_24byte(rgb[0], rgb[1], rgb[2], spi_dat_buf, index);
   // dump_buf(spi_dat_buf, LED_NUM_MAX*24);
//    OS_ENTER_CRITICAL();
//    led_spi_send_rgbbuf(spi_dat_buf, LED_NUM_MAX);
    led_spi_send_rgbbuf_isr(spi_dat_buf, LED_NUM_MAX);
//    os_time_dly(2);
//    OS_EXIT_CRITICAL();
}

static inline brightness_t ws2812_get_brightness_val(ws2812_handle_t* handle)
{
    return br_levels[handle->br_level_index];
}

static void ws2812_set_brightness_val(ws2812_handle_t* handle, brightness_t brightness, rgb_t rgb)
{
    for(int i = 0; i < 3; i++)
    {
        if(rgb_table[handle->color_index][i] != 0)
        {
            rgb[i] = brightness;
        }
        else
            rgb[i] = 0;
    }

    return;
}


static int ws2812_on(ws2812_handle_t * handle)
{
    if(handle->on)
        return 1;

    rgb_t rgb ={0};
	brightness_t brightness = ws2812_get_brightness_val(handle);
    ws2812_set_brightness_val(handle, brightness, rgb);
    ws2812_send_rgb(handle, rgb);

    handle->on = true;

    return 0;
}

static int ws2812_off(ws2812_handle_t * handle)
{
    if(! handle->on)
        return 1;

    rgb_t rgb = {0, 0, 0};
    ws2812_send_rgb(handle, rgb);

    //memset(handle, 0, sizeof(ws2812_handle_t));

    //handle->br_level_index = DEFAULT_BRIGHTNESS_LV;
    handle->on = false;

    return 0;
}

static int ws2812_brightness_inc(ws2812_handle_t * handle)
{
    if(! handle->on)
        return 1;

    if(handle->br_level_index < _COUNTOF(br_levels)-1)
        handle->br_level_index++;
    else
        return 2;

    rgb_t rgb;
    brightness_t brightness = ws2812_get_brightness_val(handle);

    ws2812_set_brightness_val(handle, brightness, rgb);
    ws2812_send_rgb(handle, rgb);

    return 0;
}

static int ws2812_brightness_dec(ws2812_handle_t * handle)
{
    if(! handle->on)
        return 1;

    if(handle->br_level_index > 1)
        handle->br_level_index--;
    else
        return 2;

    rgb_t rgb;
    brightness_t brightness = ws2812_get_brightness_val(handle);

    ws2812_set_brightness_val(handle, brightness, rgb);
    ws2812_send_rgb(handle, rgb);

    return 0;
}

static int ws2812_change_color(ws2812_handle_t * handle, uint8_t color_index)
{
    if(! handle->on)
        return 1;
    if(handle->color_index == color_index)
        return 2;

    brightness_t brightness = ws2812_get_brightness_val(handle);

    handle->color_index = color_index;

    rgb_t rgb;
    ws2812_set_brightness_val(handle, brightness, rgb);
    ws2812_send_rgb(handle, rgb);

    return 0;
}

int ws2812_ctrl(uint8_t index, uint8_t cmd)
{
    int ret = 0;
    switch(cmd)
    {
        case 0:
            ws2812_off(&ws2812_handle[0]);
            ws2812_off(&ws2812_handle[1]);
            ws2812_off(&ws2812_handle[2]);
            ws2812_off(&ws2812_handle[3]);
            ws2812_off(&ws2812_handle[4]);
            ret = ws2812_on(&ws2812_handle[index]);
            break;
        case 1:
            ret = ws2812_off(&ws2812_handle[index]);
            break;
        case 2:
            ret = ws2812_brightness_inc(&ws2812_handle[index]);
            break;
        case 3:
            ret = ws2812_brightness_dec(&ws2812_handle[index]);
            break;
        case 4:
            ret = ws2812_change_color(&ws2812_handle[index], (ws2812_handle[index].color_index + 1)%_COUNTOF(rgb_table));
            break;
        case 5:
            ret = ws2812_on(&ws2812_handle[index]);
        break;
        default:
            ret = 1;
            break;
    }

    return ret;
}

#define WS2812_BLINK_2MS_MUL    100     //mul*2 ms    max:65535

struct _ws2812_it_ctrl{
    ws2812_handle_t * handle;
    /* interrupt */
    enum status{
        StaIdle,
        StaBlinkInited,
    }sta;
    uint8_t times;
    uint8_t count;
    uint8_t mul_2ms;
    uint16_t tick;
}ws2812_it_ctrl;

static void ws2812_2ms_fun(void)
{
    ws2812_it_ctrl.tick++;
    return;
}



/**
 * @brief 开始闪烁
 *
 * @param index 灯序号
 * @param times 亮+灭总次数
 * @param mul_2ms 亮灭的频率 n*2 ms
 */
void ws2812_blink_IT_start(uint8_t index, uint8_t times, uint8_t mul_2ms)
{
    ws2812_it_ctrl.handle = &ws2812_handle[index];
    ws2812_it_ctrl.sta = StaBlinkInited;
    ws2812_it_ctrl.count = 0;
    ws2812_it_ctrl.mul_2ms = mul_2ms;
    ws2812_it_ctrl.tick = 0;
    ws2812_it_ctrl.times = times;

   // ws2812_off(&ws2812_handle[0]);   //enable key
    ws2812_off(&ws2812_handle[1]);
    ws2812_off(&ws2812_handle[2]);
    ws2812_off(&ws2812_handle[3]);
    ws2812_off(&ws2812_handle[4]);

    regist_2ms_fun(WS2812_2MS_FUN_INDEX, ws2812_2ms_fun);
}

void ws2812_blink_IT_ctrl(void(*hook)(void))
{

    switch(ws2812_it_ctrl.sta)
    {
        case StaIdle:
            break;
        case StaBlinkInited:
            if(ws2812_it_ctrl.tick >= ws2812_it_ctrl.mul_2ms)
            {
                ws2812_it_ctrl.tick = 0;
                ws2812_it_ctrl.count++;
                if(ws2812_it_ctrl.handle->on)
                {
                    ws2812_off(ws2812_it_ctrl.handle);
                }
                else
                {
                    ws2812_on(ws2812_it_ctrl.handle);
                }

                if(ws2812_it_ctrl.times != 0xFF) //0xFF一直闪
                {
                    if(ws2812_it_ctrl.count >= ws2812_it_ctrl.times)
                    {
                        ws2812_it_ctrl.sta = StaIdle;
                        ws2812_it_ctrl.count = 0;
                        ws2812_it_ctrl.tick = 0;
                        unregist_2ms_fun(WS2812_2MS_FUN_INDEX);
                        if(hook) hook();
                    }
                }
            }
            break;
        default:
            break;

    }

    return;
}


void ws2812_init()
{

    //app_timer_init();

    led_spi_init();
    memset(spi_dat_buf, 0, sizeof(spi_dat_buf));

    for(int i = 0;i <LED_NUM_MAX;i++)
    {
        ws2812_handle[i].on = true;
        ws2812_handle[i].color_index = RED;
        ws2812_handle[i].br_level_index = DEFAULT_BRIGHTNESS_LV;

        ws2812_off(&ws2812_handle[i]);
    }


//    os_time_dly(2);
//#if 0
//        led_spi_send_rgbbuf(spi_dat_buf, LED_NUM_MAX);        //等待的方式，建议用在发的数据量小的场合
//#else
//        led_spi_send_rgbbuf_isr(spi_dat_buf, LED_NUM_MAX);    //中断的方式，建议用在发的数据量大的场合
//#endif
}


#if 0
void ws2812_blink_test(uint8_t index)
{
    ws2812_on(&ws2812_handle[index]);
    udelay(100000);
    ws2812_off(&ws2812_handle[index]);
    udelay(100000);
}
#endif

void kws_ws2812_ctrl(int top)
{
    switch (top)
    {
        case 1:
        case 6:
        case 8:
        case 12:
            ws2812_off(&ws2812_handle[0]);
            ws2812_off(&ws2812_handle[1]);
            ws2812_off(&ws2812_handle[2]);
            ws2812_off(&ws2812_handle[3]);
            ws2812_off(&ws2812_handle[4]);
        break;


        case 2:
        case 7:
        case 11:
            ws2812_on(&ws2812_handle[0]);
            ws2812_on(&ws2812_handle[1]);
            ws2812_on(&ws2812_handle[2]);
            ws2812_on(&ws2812_handle[3]);
            ws2812_on(&ws2812_handle[4]);
        break;

        case 3:
        case 4:
        case 5:
            ws2812_change_color(&ws2812_handle[0], (ws2812_handle[0].color_index + 1)%_COUNTOF(rgb_table));
            ws2812_change_color(&ws2812_handle[1], (ws2812_handle[1].color_index + 1)%_COUNTOF(rgb_table));
            ws2812_change_color(&ws2812_handle[2], (ws2812_handle[2].color_index + 1)%_COUNTOF(rgb_table));
            ws2812_change_color(&ws2812_handle[3], (ws2812_handle[3].color_index + 1)%_COUNTOF(rgb_table));
            ws2812_change_color(&ws2812_handle[4], (ws2812_handle[4].color_index + 1)%_COUNTOF(rgb_table));
        break;

        case 9:
            ws2812_brightness_dec(&ws2812_handle[0]);
            ws2812_brightness_dec(&ws2812_handle[1]);
            ws2812_brightness_dec(&ws2812_handle[2]);
            ws2812_brightness_dec(&ws2812_handle[3]);
            ws2812_brightness_dec(&ws2812_handle[4]);
        break;

        case 10:
            ws2812_brightness_inc(&ws2812_handle[0]);
            ws2812_brightness_inc(&ws2812_handle[1]);
            ws2812_brightness_inc(&ws2812_handle[2]);
            ws2812_brightness_inc(&ws2812_handle[3]);
            ws2812_brightness_inc(&ws2812_handle[4]);
        break;

        default:
            break;
    }
}
