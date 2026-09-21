#include "include.h"
#include "esbc_tran.h"

#if UART_AUDIO_EN

#define TRACE_EN                0
#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#define UART_RX_ERR_OVERTIME        20
#define VOL_CTRL_LEN    5

extern S_UART_BUF sUartbuf AT(.uart_buf);//TX/RX

SYS_VOL_VARS        sSysVolVars AT(.uart_buf.ucmd);
UART_DECODE_PACKET  sDecodePacket AT(.uart_buf.ucmd);
UART_VARS           suart2_vars AT(.uart_buf.ucmd);

cbuf_t cbuf_uart2tx AT(.uart_buf.utx);
cbuf_t cbuf_uart2rx AT(.uart_buf.urx);

const char str_tx_uinfo[] = "1M/S= %d, tx: %4dbyte/s, rx: %4dbyte/s\n";
const char str_rx_uinfo[] = "2M/S= %d, tx: %4dbyte/s, rx: %4dbyte/s\n";

#if MIC_ESBC_PROC_EN
static u8 tmp_send_buf[AUDIO_TX_FRAME] AT(.uart_buf.urx);
#endif

#if MIC_ESBC_PROC_EN
u8 each_frame_buf[UART_AUDIO_ESBC + UART_CMD_HEAD] AT(.uart_buf.urx);
#else
u8 each_frame_buf[MIC_SEND_PCM_SAMPLES + UART_CMD_HEAD] AT(.uart_buf.urx);
#endif

extern volatile bool flag_spk_data_kick;

//command03 系统总音量控制
AT(.rodata.spi1_cmd)
static const u8 sys_vol_ctrl_03[VOL_CTRL_LEN] = {
    0xEF,0xFA,  //head
    0x03,0x01,  //cmd ID/data len
    0x01,       //0~16级 或 0~30级 或 0~50级
};
//command04 MIC模拟增益控制
AT(.rodata.spi1_cmd)
static const u8 mic_alg_ctrl_04[VOL_CTRL_LEN] = {
    0xEF,0xFA,  //head
    0x04,0x01,  //cmd ID/data len
    0x01,       //0~13级
};
//command05 MIC数字增益控制
AT(.rodata.spi1_cmd)
static const u8 mic_dig_ctrl_05[VOL_CTRL_LEN] = {
    0xEF,0xFA,  //head
    0x05,0x01,  //cmd ID/data len
    0x01,       //0~63级
};
//command06 MIC后置数字增益控制
AT(.rodata.spi1_cmd)
static const u8 mic_dig_ctrl_06[VOL_CTRL_LEN] = {
    0xEF,0xFA,  //head
    0x06,0x01,  //cmd ID/data len
    0x01,       //0~14级
};

//从机发送有效数据
AT(.com_rodata.spi1_cmd)
static const u8 slave_tx_cmd_08[] = {
    //test
    0xEF,0xFA,  //head
    0x08,0x0B,  //cmd ID/data len
    0x00,       //mic number
    0x00,0x00,  //data---
    0x00,0x00,  //data---
    0x00,0x00,  //data---
    0x00,0x00,  //data---
    0x00,0x00,  //data---
};
//command09
//从机发送无效，缓存buf数据不足，主机照常接收空包数据
AT(.com_rodata.spi1_cmd)
static const u8 slave_fail_cmd_09[] = {
    0xF9,0xEF,  //head
    0x55,0xAA,
    0x00,0x00,  //00
    0x00,0x00,  //00
    0x00,0x00,  //00
    0x00,0x00,  //00
    0x00,0x00,  //00
};

AT(.com_text.cir_read)
bool far_mic1_w2cirbuf_process(void *buf, u16 length)
{
    if(circular_buf_is_write_able(&cbuf_uart2rx, length)){
        circular_buf_write(&cbuf_uart2rx, (u8 *)buf, length);  //写mic数据
        return true;
    }else{
        #if EF_PRINTF
        printf(buf_f);
        #endif
        return false;
    }
}

AT(.com_text.cir_read)
bool far_mic1_r_cirbuf_process(u8 *output, u16 length)
{
    if(cbuf_uart2rx.data_len >= length){
        circular_buf_read(&cbuf_uart2rx, output, length);  //读cbuf数据
        return true;
    }else{
        #if EF_PRINTF
        printf(buf_e);
        #endif
        return false;
    }
}

AT(.com_text.cbuf)
u32 circular_buf_read(cbuf_t *cbuf, void *out_buf, u32 read_len) //返回读出的字节数
{
    u32 r_len = read_len;
    u32 copy_len;

    if (!cbuf) {
        return 0;
    }

    if ((u32)cbuf->rptr >= (u32)cbuf->end) {            //读到cbuf的尾部，将指针重新定位到start
        cbuf->rptr = (u8 *)cbuf->start;
    }

    if (cbuf->data_len < read_len) {                    //有效数据不满足读取长度，停止读数据
        return 0;
    }

    copy_len = (u32)cbuf->end - (u32)cbuf->rptr;        //读指针距离cbuf尾部的字节数
    if (copy_len > read_len) {
        copy_len = read_len;
    }
    read_len -= copy_len;                               //read_len表示从读指针到cbuf尾端读不够的字节数

    memcpy(out_buf, cbuf->rptr, copy_len);              //从读指针到cbuf尾端要读的数据量写入output_buf
    // my_printf(spkr_str, copy_len, read_len);
    if (read_len == 0) {
        cbuf->rptr += copy_len;                         //更新读指针的位置
    } else {
        memcpy((u8 *)out_buf + copy_len, cbuf->start, read_len);  //读不够的字节数，从start开始读
        cbuf->rptr = cbuf->start + read_len;
    }

    GLOBAL_INT_DISABLE();
    cbuf->data_len -= r_len;                           //更新data数
    GLOBAL_INT_RESTORE();
    // my_printf(spkrr_str, cbuf->data_len);
    return r_len;
}

AT(.text.uart2_drv)
void uart_main_process(void)
{
    #if 1
#if MIC_ESBC_PROC_EN
    u8 i;
#endif

    if(flag_spk_data_kick ){
        flag_spk_data_kick = false;

        memcpy(each_frame_buf, slave_tx_cmd_08, UART_CMD_HEAD);
        #if MIC_ESBC_PROC_EN
        if(spk_read_by_cirbuf_process(tmp_send_buf, AUDIO_TX_FRAME)){
            for(i = 0; i < ESBC_CAL_TIME; i++){
                memcpy(&each_frame_buf[UART_CMD_HEAD], &tmp_send_buf[i*UART_AUDIO_ESBC], UART_AUDIO_ESBC);
                bsp_send_uart0_data(each_frame_buf, UART_KICK_NUM);
            }
            suart2_vars.send_cnt += AUDIO_TX_FRAME;//test speed
        }else{
            my_printf("E2 ");
            bsp_send_uart0_data((u8 *)slave_fail_cmd_09, UART_KICK_NUM);
        }
        #else
        each_frame_buf[3] = 0xff;//MIC_SEND_PCM_SAMPLES
        if(spk_read_by_cirbuf_process(&each_frame_buf[UART_CMD_HEAD], MIC_SEND_PCM_SAMPLES)){
            bsp_send_uart0_data(each_frame_buf, (MIC_SEND_PCM_SAMPLES + UART_CMD_HEAD));
            suart2_vars.send_cnt += MIC_SEND_PCM_SAMPLES;//test speed
        }else{
            my_printf("E2 ");
            bsp_send_uart0_data((u8 *)slave_fail_cmd_09, UART_KICK_NUM);
        }
        #endif
    }
    //printf test
    if(tick_check_expire(suart2_vars.main_proc_tick, 1000)){
        suart2_vars.main_proc_tick = tick_get();
        suart2_vars.sys_run_time++;
        my_printf(str_rx_uinfo, suart2_vars.sys_run_time, suart2_vars.send_cnt, suart2_vars.recv_cnt);
        suart2_vars.send_cnt = 0;
        suart2_vars.recv_cnt = 0;

        suart2_vars.rx_err_cnt = 0;//每秒检查一次
    }
#endif
}


AT(.text.bsp.uart)
void uart_rx_err_process(void)
{
    suart2_vars.rx_err_cnt++;
    if(suart2_vars.rx_err_cnt >= UART_RX_ERR_OVERTIME){
        // my_printf("uart_rx_err rst\n");
        suart2_vars.rx_err_cnt = 0;
        suart2_vars.bo_uart_rst = true;
        // WDT_RST();
    }
}

AT(.com_text.uart)
bool parse_uart_cmd_data_test(u8 uartData)
{
    u8 re_buf[5];
    if(suart2_vars.bo_uart_rst){
        // my_printf("%X ", uartData);
    }

    bool boDecodeSuccess = false;
    if((CMD_HEADER_ONE_UART0 == uartData) && (IDEL_STEP_UART0 == sDecodePacket.decodeStep)){//&& (SAVE_DATA != sDecodePacket.decodeStep)
        sDecodePacket.getHeader1 = true;
        sDecodePacket.decodeStep = FIND_HEADER;
    }else{
        switch(sDecodePacket.decodeStep)
        {
            case SAVE_DATA_UART0://存放有效数据
            {
                sDecodePacket.DataBuf[sDecodePacket.w_cnt] = uartData;
                sDecodePacket.w_cnt++;
                suart2_vars.recv_cnt++;
                if(sDecodePacket.w_cnt >= sDecodePacket.uLength){
                    sDecodePacket.decodeStep = CMD_SEND;

                }else{
                }
            }break;
            case IDEL_STEP_UART0:
                    if(CMD_HEADER_ONE_UART0 == uartData){
                        my_printf("\n !!!!! idl:0x%X \n", uartData);
                    }
                    break;
            case FIND_HEADER_UART0://识别header
            {
                // TRACE("FIND_HEADER\n");
                if(sDecodePacket.getHeader1){
                    if(CMD_HEADER_TWO_UART0 == uartData){
                        sDecodePacket.decodeStep = CONFIRM_ID;
                    }else{
                        sDecodePacket.getHeader1 = false;
                        TRACE("header error1: 0x%X\n", uartData);
                        uart_rx_err_process();
                        memset(&sDecodePacket, 0, sizeof(sDecodePacket));
                    }
                }else{
                    TRACE("header error2: 0x%X\n", uartData);
                    uart_rx_err_process();
                    memset(&sDecodePacket, 0, sizeof(sDecodePacket));
                }
            }break;
            case CONFIRM_ID_UART0:
            {
                if((uartData <= CMD_STX_09_UART0) || (uartData >= CMD_MTX_01_UART0)){
                    sDecodePacket.getIdentification = uartData;
                    sDecodePacket.decodeStep = GET_LENGTH;
                }else{
                    // TRACE("identification error:0x%X,0x%X\n", uartData, test_save_uart);
                    uart_rx_err_process();
                    memset(&sDecodePacket, 0, sizeof(sDecodePacket));
                }

            }break;
            case GET_LENGTH_UART0:
            {
                if(uartData <= UART_CMD_MAX_SIZE){
                    sDecodePacket.uLength = uartData;
                    if(uartData == 0xff){
                        sDecodePacket.uLength = UART_CMD_MAX_SIZE;//暂时先定义0xff作为收发1+256byte的，1指mic num
                    }
                    sDecodePacket.decodeStep = SAVE_DATA_UART0;
                }else{
                    TRACE("cmd length oversize: 0x%X\n", uartData);
                    uart_rx_err_process();
                    memset(&sDecodePacket, 0, sizeof(sDecodePacket));
                }
            }break;

            case CMD_SEND_UART0:
            {
                TRACE("cmd step error\n");
                memset(&sDecodePacket, 0, sizeof(sDecodePacket));
                uart_rx_err_process();
            }break;
            default:
                TRACE("step error: %d\n", sDecodePacket.decodeStep);
                memset(&sDecodePacket, 0, sizeof(sDecodePacket));
                uart_rx_err_process();
                break;
        }
    }

    if(CMD_SEND_UART0 == sDecodePacket.decodeStep){//主机接收从机一包数据
        switch(sDecodePacket.getIdentification){
            case CMD_MTX_01_UART0:
                uart_rx_err_process();
                boDecodeSuccess = true;
                break;
            case CMD_MTX_02_UART0:
                uart_rx_err_process();
                boDecodeSuccess = true;
                break;
            case CMD_SYS_VOL_UART0:
                memcpy(re_buf, sys_vol_ctrl_03, VOL_CTRL_LEN);
                if(sDecodePacket.DataBuf[0] <= VOL_MAX){//最大音量级数在setting上配置
                    if(sDecodePacket.DataBuf[0] != sSysVolVars.u_set_sys_vol){
                        sSysVolVars.u_set_sys_vol = sDecodePacket.DataBuf[0];
                        // my_printf("vol set: %d\n", sDecodePacket.DataBuf[0]);
                        bsp_set_volume(sSysVolVars.u_set_sys_vol);
                    }
                }
                re_buf[VOL_CTRL_LEN - 1] = sSysVolVars.u_set_sys_vol;
                bsp_send_uart0_data(re_buf, VOL_CTRL_LEN);
                boDecodeSuccess = true;
                break;
            case CMD_MIC_ALG_UART0:
                // my_printf("MIC_ANALOG_GAIN_CTRL, %d, %d\n", sDecodePacket.DataBuf[0], sSysVolVars.u_set_mic_anl);
                if((sDecodePacket.DataBuf[0] > 0) && (sDecodePacket.DataBuf[0] <= 13)){
                    if(sDecodePacket.DataBuf[0] != sSysVolVars.u_set_mic_anl){
                        sSysVolVars.u_set_mic_anl = sDecodePacket.DataBuf[0];
                    }
                }
                memcpy(re_buf, mic_alg_ctrl_04, VOL_CTRL_LEN);
                re_buf[VOL_CTRL_LEN - 1] = sSysVolVars.u_set_mic_anl;
                bsp_send_uart0_data(re_buf, VOL_CTRL_LEN);
                boDecodeSuccess = true;
                break;
            case CMD_MIC_DIG_UART0:
                if((sDecodePacket.DataBuf[0] >= 0) && (sDecodePacket.DataBuf[0] <= 63)){
                    if(sDecodePacket.DataBuf[0] != sSysVolVars.u_set_mic_dig){
                        // func_speaker_stop();
                        sSysVolVars.u_set_mic_dig = sDecodePacket.DataBuf[0];

                    }
                }
                memcpy(re_buf, mic_dig_ctrl_05, VOL_CTRL_LEN);
                re_buf[VOL_CTRL_LEN - 1] = sSysVolVars.u_set_mic_dig;
                bsp_send_uart0_data(re_buf, VOL_CTRL_LEN);
                boDecodeSuccess = true;
                break;
            case CMD_MIC_POST_GAIN_UART0:
                if((sDecodePacket.DataBuf[0] >= 0) && (sDecodePacket.DataBuf[0] <= 14)){
                    // my_printf("CMD_MIC_POST_GAIN\n");
                    if(sDecodePacket.DataBuf[0] != sSysVolVars.u_set_mic_post_gain){
                        sSysVolVars.u_set_mic_post_gain = sDecodePacket.DataBuf[0];
                        //spk_mic_post_gain_val           = sDecodePacket.DataBuf[0];  GZM
                    }
                }
                memcpy(re_buf, mic_dig_ctrl_06, VOL_CTRL_LEN);
                bsp_send_uart0_data(re_buf, VOL_CTRL_LEN);
                boDecodeSuccess = true;
                break;
            case CMD_STX_08_UART0:
            case CMD_STX_09_UART0:
                // my_print_r(&sDecodePacket.DataBuf[1], UART_CMD_ESBC_SIZE);
                if(far_mic1_w2cirbuf_process(&sDecodePacket.DataBuf[1], UART_CMD_ESBC_SIZE)){   //GZM
                    // TRACE("s1 ");
                }else{
                    // my_printf("we ");
                }
                boDecodeSuccess = true;
                break;
            default:
                uart_rx_err_process();
                //my_printf("7:%d", sDecodePacket.getIdentification);
                break;
        }

    }
    if(boDecodeSuccess)
    {
        memset(&sDecodePacket, 0, sizeof(sDecodePacket));
    }

    return boDecodeSuccess;
}

//将uart收到的数据进行解码
AT(.com_text.uart)
void uart_decode_process(void)
{
    u8 hidCharData;
    while(bsp_uart0_get_data(&hidCharData)){
        WDT_CLR();
        parse_uart_cmd_data_test(hidCharData);
    }
}
#endif
