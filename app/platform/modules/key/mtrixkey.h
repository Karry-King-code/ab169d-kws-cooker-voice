#ifndef _MTRIXKEY_H
#define _MTRIXKEY_H

typedef struct{
    gpio_t gpio0;
    gpio_t gpio1;
    gpio_t gpio2;
    gpio_t gpio3;
    gpio_t gpio4;
    gpio_t gpio5;
    gpio_t gpio6;
    gpio_t gpio7;
}maxtrix_key_t;

maxtrix_key_t maxtrix_key;

gpio_t *g0 = &maxtrix_key.gpio0;
gpio_t *g1 = &maxtrix_key.gpio1;
gpio_t *g2 = &maxtrix_key.gpio2;
gpio_t *g3 = &maxtrix_key.gpio3;
gpio_t *g4 = &maxtrix_key.gpio4;
gpio_t *g5 = &maxtrix_key.gpio5;
gpio_t *g6 = &maxtrix_key.gpio6;
gpio_t *g7 = &maxtrix_key.gpio7;

#endif
