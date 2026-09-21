#ifndef __BSP_SARADC_H
#define __BSP_SARADC_H

void e_saradc_init(u16 ch);
bool e_get_adc_val_test(void);
u16 e_get_saradc_data(u16 ch);
void e_saradc_proc(void);
#endif // __BSP_SARADC_H

