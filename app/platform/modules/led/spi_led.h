#ifndef _BSP_SPI_LED_H
#define _BSP_SPI_LED_H

#define SPI_LED_BAUD                3000000 //SPI1 BAUD

#define LED_SPI2BIT_LEN             1       //使用 1byte SPI数据表示1812的 1bit
#define LED_EACH_RGB_LEN            8       //LED RGB控制分别需要8bit(RGB=3*8=24bit)
#define LED_SPI_ENCODE_LEN          LED_SPI2BIT_LEN * LED_EACH_RGB_LEN//控制1个LED的SPI数据量byte

#define LED_SPI_ENCODE_RESET_LEN    30
#define LED_MAP_BUF_SIZE            LED_TOTAL_NUM
#define LED_DATA_BUF_SIZE           LED_TOTAL_NUM * 3 + LED_SPI_ENCODE_RESET_LEN

typedef enum{
    LED_G_01,         //G
    LED_R_01,         //R
    LED_B_01,         //B

    LED_TOTAL_NUM,
    ALL_LED = 0xFF,
}led_id_t;

// typedef struct
// {
//     u8 rgb_id;
//     u8 rgb_sta;
// }led_cmd_t;

typedef struct 
{
    u8 led_map[LED_MAP_BUF_SIZE];
    u8 led_data_buf[LED_DATA_BUF_SIZE];

    bool led_chg_flag;
}led_data_t;

void spi1_led_init(void);
void spi1_led_ctrl(led_id_t rgb_id, u8 rgb_level);
u32 level_2_encode(u8 rgb_level);

void set_led_chg(bool boset);
bool get_led_chg(void);
void spi1_test_set(void);
void spi1_led_data_send_proc(void);

#endif  //_BSP_SPI_LED_H
