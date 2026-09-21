#include "include.h"

#if SYS_CODE_ERASE
void print_info(void);

u8 code_data[10];
void erase_code(void)
{
    os_spiflash_read(code_data,0x5000,10);
    my_print_r(code_data,10);

    for(u8 i = 0; i < 32; i++){                     //擦除 开启boot 跳转后，本工程code区从0x5000开始，0x5000~ 0x24fff的副程序代码区
        spiflash_erase(0x5000 + i * 0x1000);
    }
    os_spiflash_read(code_data,0x5000,10);
    my_print_r(code_data,10);

    while(1){
        WDT_CLR();
        print_info();
    }
}

#endif
