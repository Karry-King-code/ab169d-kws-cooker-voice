#ifndef _API_PWR_H_
#define _API_PWR_H_

void vddio_voltage_user_cfg(u8 vddio_set);          //用户配置VDDIO的电压,0~31(共32级), VDDIO: 1.5V~3.825V

void pmu_init(u8 buck_en);
u16 pmu_get_vbg(void);
u16 pmu_get_vddio(void);
void set_buck_mode(u8 buck_en);
#endif // _API_PWR_H_

