#ifndef _WS2812_NEW_H_
#define _WS2812_NEW_H_


#define CMD_WS2812_ON   5
#define CMD_WS2812_OFF  1
void ws2812_init();

void ws2812_blink_IT_ctrl(void(*hook)(void));

void ws2812_blink_IT_start(uint8_t index, uint8_t times, uint8_t mul_2ms);

int ws2812_ctrl(uint8_t index, uint8_t cmd);

void ws2812_blink_test(uint8_t index);

void kws_ws2812_ctrl(int top);

#endif
