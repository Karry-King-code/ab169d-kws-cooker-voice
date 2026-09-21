#include "include.h"
#include "sound_res.h"

extern void bsp_uart_init(void);   /* bsp_uart.c: 初始化UART0(波特率+发送使能) */
extern void uart0_mapping_sel(void); /* bsp_sys.c: 配置UART0 TX引脚(PRINTF_PA8=PA8) */
extern void bsp_huart_init(void);  /* bsp_huart.c: 初始化HUART(硬件UART, PA5, 发命令帧) */

//正常启动Main函数
int main(void)
{
    u32 rst_reason;
    rst_reason = sys_rst_init();
    printf("Hello Dragon asr project: %08x\n", rst_reason);
    printf("VERSIONID: %x\n", VERSIONID);
    printf("build-time: %s-%s\n", __DATE__, __TIME__);
    sys_rst_dump(rst_reason);
    sys_cb.wakeup_reason = lowpwr_get_wakeup_source();

    sys_ram_info_dump();
    bsp_sys_init();
    bsp_huart_init();      /* 初始化HUART硬件UART(PA5, 发命令帧给主控) */

#if SYS_CODE_ERASE
    erase_code();
#endif
    printf("DEVICEID: 0x%x\n", DEVICEID);

//    GPIOAFEN &= ~BIT(7); // PA0 作为gpio使用
//    GPIOADE |= BIT(7);   // pa0 作为数字IO
//    GPIOADIR &= ~BIT(7);
//
//    GPIOA |= BIT(7); // H
//    GPIOA &= ~BIT(7); // L

    /* 上电自动播报开机语"欢迎使用colorman智能产品" (res资源, 与参考工程 main.c:31 一致)
     * 在 func_run() 前播, 此时识别未启动、无采音, 无误唤醒风险 */
    sound_play_boot_welcome();

    func_run();
    return 0;
}




//升级完成
void update_complete(int mode)
{
    bsp_update_init();
    printf("update complete: %d\n", mode);
    if (mode == 0) {
        WDT_DIS();
        while (1);
    }
    WDT_RST();
}
