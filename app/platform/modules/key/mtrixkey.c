#include "include.h"
#include "mtrixkey.h"

#if MAXTRIX_KEY_EN

static u8 maxtrix_scan_flag = 1;

AT(.com_text.bsp.maxtrixkey)
void maxtrix_key_gpio_input_dir(void)
{
    g0->sfr[GPIOxDIR]  |=  g0->pin;
    g1->sfr[GPIOxDIR]  |=  g1->pin;
    g2->sfr[GPIOxDIR]  |=  g2->pin;
    g3->sfr[GPIOxDIR]  |=  g3->pin;
    g4->sfr[GPIOxDIR]  |=  g4->pin;
    g5->sfr[GPIOxDIR]  |=  g5->pin;
    g6->sfr[GPIOxDIR]  |=  g6->pin;
    g7->sfr[GPIOxDIR]  |=  g7->pin;
}

AT(.com_text.bsp.maxtrixkey)
void maxtrix_key_gpio_output_dir(void)
{
    g0->sfr[GPIOxDIR]  &=  ~g0->pin;
    g1->sfr[GPIOxDIR]  &=  ~g1->pin;
    g2->sfr[GPIOxDIR]  &=  ~g2->pin;
    g3->sfr[GPIOxDIR]  &=  ~g3->pin;
    g4->sfr[GPIOxDIR]  &=  ~g4->pin;
    g5->sfr[GPIOxDIR]  &=  ~g5->pin;
    g6->sfr[GPIOxDIR]  &=  ~g6->pin;
    g7->sfr[GPIOxDIR]  &=  ~g7->pin;
}

AT(.com_text.bsp.maxtrixkey)
void maxtrix_key_gpio_input_pushup(void)
{
    maxtrix_key_gpio_input_dir();

    g0->sfr[GPIOxPU]  |=  g0->pin;
    g1->sfr[GPIOxPU]  |=  g1->pin;
    g2->sfr[GPIOxPU]  |=  g2->pin;
    g3->sfr[GPIOxPU]  |=  g3->pin;
    g4->sfr[GPIOxPU]  |=  g4->pin;
    g5->sfr[GPIOxPU]  |=  g5->pin;
    g6->sfr[GPIOxPU]  |=  g6->pin;
    g7->sfr[GPIOxPU]  |=  g7->pin;
}

AT(.com_text.bsp.maxtrixkey)
void maxtrix_key_gpio_input_pushdown(void)
{
    maxtrix_key_gpio_input_dir();

    g0->sfr[GPIOxPD]  |=  g0->pin;
    g1->sfr[GPIOxPD]  |=  g1->pin;
    g2->sfr[GPIOxPD]  |=  g2->pin;
    g3->sfr[GPIOxPD]  |=  g3->pin;
    g4->sfr[GPIOxPD]  |=  g4->pin;
    g5->sfr[GPIOxPD]  |=  g5->pin;
    g6->sfr[GPIOxPD]  |=  g6->pin;
    g7->sfr[GPIOxPD]  |=  g7->pin;
}

AT(.com_text.bsp.maxtrixkey)
void maxtrix_key_gpio_cancel_pushup(void)
{
    maxtrix_key_gpio_output_dir();

    g0->sfr[GPIOxPU]  &=  ~g0->pin;
    g1->sfr[GPIOxPU]  &=  ~g1->pin;
    g2->sfr[GPIOxPU]  &=  ~g2->pin;
    g3->sfr[GPIOxPU]  &=  ~g3->pin;
    g4->sfr[GPIOxPU]  &=  ~g4->pin;
    g5->sfr[GPIOxPU]  &=  ~g5->pin;
    g6->sfr[GPIOxPU]  &=  ~g6->pin;
    g7->sfr[GPIOxPU]  &=  ~g7->pin;
}

AT(.com_text.bsp.maxtrixkey)
void maxtrix_key_gpio_cancel_pushdown(void)
{
    maxtrix_key_gpio_output_dir();

    g0->sfr[GPIOxPD]  &=  ~g0->pin;
    g1->sfr[GPIOxPD]  &=  ~g1->pin;
    g2->sfr[GPIOxPD]  &=  ~g2->pin;
    g3->sfr[GPIOxPD]  &=  ~g3->pin;
    g4->sfr[GPIOxPD]  &=  ~g4->pin;
    g5->sfr[GPIOxPD]  &=  ~g5->pin;
    g6->sfr[GPIOxPD]  &=  ~g6->pin;
    g7->sfr[GPIOxPD]  &=  ~g7->pin;
}

AT(.com_text.bsp.maxtrixkey)
int maxtrix_key_scan(void)
{
        u8 value= 0;

    //先扫描右边I组，J组
    maxtrix_key_gpio_input_pushup();

    delay_us(5);

    if(!(g0->sfr[GPIOx] & g0->pin)){                  //KI0

    }
    if(!(g1->sfr[GPIOx] & g1->pin)){                  //KI1

    }
    if(!(g2->sfr[GPIOx] & g2->pin)){                  //KI2

    }
    if(!(g3->sfr[GPIOx] & g3->pin)){                  //KI3

    }
    if(!(g4->sfr[GPIOx] & g4->pin)){                  //KI4

    }
    if(!(g5->sfr[GPIOx] & g5->pin)){                  //KI5

    }
    if(!(g6->sfr[GPIOx] & g6->pin)){                  //KI6

    }
    if(!(g7->sfr[GPIOx] & g7->pin)){                  //KI7

    }

    delay_us(5);
    //若按下右边I组的8个按键，则暂时屏蔽左边按键按下，松开即取消屏蔽，若不适用PA8~PA15需要自行更改判断条件
    if(((g0->sfr[GPIOx]>>8&0xff) == 0xfe) || ((g1->sfr[GPIOx]>>8&0xff) == 0xfd) || ((g2->sfr[GPIOx]>>8&0xff) == 0xfb) || ((g3->sfr[GPIOx]>>8&0xff) == 0xf7) || ((g4->sfr[GPIOx]>>8&0xff) == 0xef) || ((g5->sfr[GPIOx]>>8&0xff) == 0xdf) || ((g6->sfr[GPIOx]>>8&0xff) == 0xbf) || ((g7->sfr[GPIOx]>>8&0xff) == 0x7f)){
        maxtrix_scan_flag = 0;
    }else{
        maxtrix_scan_flag = 1;
    }

    maxtrix_key_gpio_cancel_pushup();

    delay_us(5);

    maxtrix_key_gpio_input_pushdown();

    delay_us(5);

    if(g0->sfr[GPIOx] & g0->pin){                        //KJ0

    }
    if(g1->sfr[GPIOx] & g1->pin){                        //KJ1

    }
    if(g2->sfr[GPIOx] & g2->pin){                        //KJ2

    }
    if(g3->sfr[GPIOx] & g3->pin){                        //KJ3

    }
    if(g4->sfr[GPIOx] & g4->pin){                        //KJ4

    }
    if(g5->sfr[GPIOx] & g5->pin){                        //KJ5

    }
    if(g6->sfr[GPIOx] & g6->pin){                        //KJ6

    }
    if(g7->sfr[GPIOx] & g7->pin){                        //KJ7

    }

    maxtrix_key_gpio_cancel_pushdown();

    delay_us(5);

    //再扫左边按键键盘
    if(maxtrix_scan_flag){
        //第一行输入上拉，第二行输出拉低
        maxtrix_key_gpio_input_pushup();

        g0->sfr[GPIOxDIR] &= ~g0->pin;
        g0->sfr[GPIOxCLR] = g0->pin;

        delay_us(5);

        if(!(g1->sfr[GPIOx] & g1->pin)){                  //KA1
            value = KEY_1;
        }
        if(!(g2->sfr[GPIOx] & g2->pin)){                  //KA2
            value = KEY_VOL_DOWN;
        }
        if(!(g3->sfr[GPIOx] & g3->pin)){                  //KA3
            value = KEY_VOL_UP;
        }
        if(!(g4->sfr[GPIOx] & g4->pin)){                  //KA4

        }
        if(!(g5->sfr[GPIOx] & g5->pin)){                  //KA5

        }
        if(!(g6->sfr[GPIOx] & g6->pin)){                  //KA6

        }
        if(!(g7->sfr[GPIOx] & g7->pin)){                  //KA7

        }

        //第二行输入上拉，第三行输出拉低
        maxtrix_key_gpio_input_pushup();

        g1->sfr[GPIOxDIR] &= ~g1->pin;
        g1->sfr[GPIOxCLR] = g1->pin;

        delay_us(5);

        if(!(g2->sfr[GPIOx] & g2->pin)){                  //KB2

        }
        if(!(g3->sfr[GPIOx] & g3->pin)){                  //KB3

        }
        if(!(g4->sfr[GPIOx] & g4->pin)){                  //KB4

        }
        if(!(g5->sfr[GPIOx] & g5->pin)){                  //KB5

        }
        if(!(g6->sfr[GPIOx] & g6->pin)){                  //KB6

        }
        if(!(g7->sfr[GPIOx] & g7->pin)){                  //KB7

        }

        //第三行输入上拉，第四行输出拉低

        maxtrix_key_gpio_input_pushup();

        g2->sfr[GPIOxDIR] &= ~g2->pin;
        g2->sfr[GPIOxCLR] = g2->pin;

        delay_us(5);

        if(!(g3->sfr[GPIOx] & g3->pin)){                  //KC3

        }
        if(!(g4->sfr[GPIOx] & g4->pin)){                  //KC4

        }
        if(!(g5->sfr[GPIOx] & g5->pin)){                  //KC5

        }
        if(!(g6->sfr[GPIOx] & g6->pin)){                  //KC6

        }
        if(!(g7->sfr[GPIOx] & g7->pin)){                  //KC7

        }

        //第四行输入上拉，第五行输出拉低
        maxtrix_key_gpio_input_pushup();

        g3->sfr[GPIOxDIR] &= ~g3->pin;
        g3->sfr[GPIOxCLR] = g3->pin;

        delay_us(5);

        if(!(g4->sfr[GPIOx] & g4->pin)){                  //KD4

        }
        if(!(g5->sfr[GPIOx] & g5->pin)){                  //KD5

        }
        if(!(g6->sfr[GPIOx] & g6->pin)){                  //KD6

        }
        if(!(g7->sfr[GPIOx] & g7->pin)){                  //KD7

        }

        //第五行输入上拉，第六行输出拉低
        maxtrix_key_gpio_input_pushup();

        g4->sfr[GPIOxDIR] &= ~g4->pin;
        g4->sfr[GPIOxCLR] = g4->pin;

        delay_us(5);

        if(!(g5->sfr[GPIOx] & g5->pin)){                  //KE5

        }
        if(!(g6->sfr[GPIOx] & g6->pin)){                  //KE6

        }
        if(!(g7->sfr[GPIOx] & g7->pin)){                  //KE7

        }

        //第六行输入上拉，第七行输出拉低
        maxtrix_key_gpio_input_pushup();

        g5->sfr[GPIOxDIR] &= ~g5->pin;
        g5->sfr[GPIOxCLR] = g5->pin;

        delay_us(5);

        if(!(g6->sfr[GPIOx] & g6->pin)){                  //KF6

        }
        if(!(g7->sfr[GPIOx] & g7->pin)){                  //KF7

        }

        //第七行输入上拉，第八行输出拉低
        maxtrix_key_gpio_input_pushup();

        g6->sfr[GPIOxDIR] &= ~g6->pin;
        g6->sfr[GPIOxCLR] = g6->pin;

        delay_us(5);

        if(!(g7->sfr[GPIOx] & g7->pin)){                  //KG7

        }

        delay_us(5);

    }

        return value;
}

AT(.com_text.bsp.maxtrixkey)
u8 bsp_maxtrix_get(void)
{
    u8 value = 0;

    value = maxtrix_key_scan();

    return value;
}

void maxtrix_key_init(void)
{

    gpio_t *g0 = &maxtrix_key.gpio0;
    gpio_cfg_init(g0, MAX_KEY_0);

    gpio_t *g1 = &maxtrix_key.gpio1;
    gpio_cfg_init(g1, MAX_KEY_1);

    gpio_t *g2 = &maxtrix_key.gpio2;
    gpio_cfg_init(g2, MAX_KEY_2);

    gpio_t *g3 = &maxtrix_key.gpio3;
    gpio_cfg_init(g3, MAX_KEY_3);

    gpio_t *g4 = &maxtrix_key.gpio4;
    gpio_cfg_init(g4, MAX_KEY_4);

    gpio_t *g5 = &maxtrix_key.gpio5;
    gpio_cfg_init(g5, MAX_KEY_5);

    gpio_t *g6 = &maxtrix_key.gpio6;
    gpio_cfg_init(g6, MAX_KEY_6);

    gpio_t *g7 = &maxtrix_key.gpio7;
    gpio_cfg_init(g7, MAX_KEY_7);

    if (g0->sfr) {
        g0->sfr[GPIOxFEN] &= ~g0->pin;
        g0->sfr[GPIOxDE]  |= g0->pin;
        g0->sfr[GPIOxDIR] &=~g0->pin;
    }

    if (g1->sfr) {
        g1->sfr[GPIOxFEN] &= ~g1->pin;
        g1->sfr[GPIOxDE]  |= g1->pin;
        g1->sfr[GPIOxDIR] &=~g1->pin;
    }

    if (g2->sfr) {
        g2->sfr[GPIOxFEN] &= ~g2->pin;
        g2->sfr[GPIOxDE]  |= g2->pin;
        g2->sfr[GPIOxDIR] &=~g2->pin;
    }

        if (g3->sfr) {
        g3->sfr[GPIOxFEN] &= ~g3->pin;
        g3->sfr[GPIOxDE]  |= g3->pin;
        g3->sfr[GPIOxDIR] &=~g3->pin;
    }

        if (g4->sfr) {
        g4->sfr[GPIOxFEN] &= ~g4->pin;
        g4->sfr[GPIOxDE]  |= g4->pin;
        g4->sfr[GPIOxDIR] &=~g4->pin;
    }

        if (g5->sfr) {
        g5->sfr[GPIOxFEN] &= ~g5->pin;
        g5->sfr[GPIOxDE]  |= g5->pin;
        g5->sfr[GPIOxDIR] &=~g5->pin;
    }

        if (g6->sfr) {
        g6->sfr[GPIOxFEN] &= ~g5->pin;
        g6->sfr[GPIOxDE]  |= g6->pin;
        g6->sfr[GPIOxDIR] &=~g6->pin;
    }

    if (g7->sfr) {
        g7->sfr[GPIOxFEN] &= ~g7->pin;
        g7->sfr[GPIOxDE]  |= g7->pin;
        g7->sfr[GPIOxDIR] &=~g7->pin;
    }

    my_printf("maxtrix_key_init\n");

}
#endif//MAXTRIX_KEY_EN
