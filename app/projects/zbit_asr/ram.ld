#include "config.h"

ENTRY(_start)

/* Define the flash max size */
__max_flash_size = FLASH_CODE_SIZE;

__base = 0x10000000;

/*  sram0+sram1空间分配图示,具体查看下方MEMORY项实际分配
|-------sram ------|0x0001 0000
    sram0+sram1
    |-----------|0x0001 0000
    |   boot
    |-----------|0x0001 0400
    |   stack
    |-----------|0x0001 0800
    |   data
    |-----------|0x0001 1900
    |   comm
    |-----------|0x0001 2700
    |   aram
    |           |-----------|0x0001 2700
    |           |   mic+ains3(pdn_comm)
    |           |-----------|0x0001 6800
    |           ||0x0001 6800|-------0x0001 6800-------|
    |           ||  rec/dnn                            |
    |           ||           |     mp3    |    esbc    |
    |           ||0x0001 E000|0x0001 A400 |0x0001 E000 |
    |-----------|0x0001 E000

    cache ram
    |-----------|0x0007 0000
    |   bram
    |-----------|0x0007 1000 / 0x0007 2000
|------- end ------|0x0001 E000
*/

/* Define the area */
__data_vma         = 0x10800;
__comm_vma         = 0x11900;
__aram_vma         = 0x12700;   /*max area addr 0x1E000*/
__bram_vma         = 0x70000;   /*只有使能SYS_BRAM_EN才能使用bram*/

__data_ram_size    = __comm_vma - __data_vma;
__comm_ram_size    = __aram_vma - __comm_vma;
__aram_size        = 0x1E000 - __aram_vma;
__bram_size        = 0x1000;    /*bram长度和SYS_BRAM_LEN匹配*/

__aram_mic_size    = 0x4100;
__aram_gpa_size    = 0x900;     /*dac buf大小,当不开启DAC时可设为0*/
__aram_mp3_size    = 0x2600;

__aram_fft_saddr   = (__aram_size - 0x1450);      /*FFT buf start*/
__aram_fft_eaddr   = (__aram_size - 0x14);        /*FFT work_ip */

/* 复用RAM临时当公共区*/
__pdn_comm_vma          = __aram_vma;                                                  //开关机代码区

__aram_mic_vma          = __pdn_comm_vma;                                                  //mic变量区起始地址

__aram_msc_vma          = (__pdn_comm_vma + __aram_mic_size);                              //music播放公共变量区起始地址
__aram_msc_size         = __aram_gpa_size;                                          //music变量区长度

__aram_mp3_comm_vma     = (__pdn_comm_vma + __aram_mic_size + __aram_gpa_size);            //mp3代码区起始地址(size:0xC00)
__aram_mp3_vma          = (__pdn_comm_vma + __aram_mic_size + __aram_gpa_size + 0xd00);    //mp3变量区起始地址
__aram_mp3_size         = 0x2600;                                                   //mp3变量区长度

__aram_esbc_comm_vma    = (__pdn_comm_vma + __aram_mic_size + __aram_gpa_size);            //esbc代码区起始地址(size:0x3000)
__aram_esbc_vma         = (__pdn_comm_vma + __aram_mic_size + __aram_gpa_size + 0x2A00);   //esbc变量区起始地址
__aram_esbc_size        = 0x4500;                                                   //esbc变量区长度

__aram_rec_comm_vma     = (__pdn_comm_vma + __aram_mic_size + __aram_gpa_size);            //record代码区起始地址(size:0x1400)
__aram_rec_vma          = (__pdn_comm_vma + __aram_mic_size + __aram_gpa_size + 0x1400);   //record变量区起始地址
__aram_rec_size         = 0x4f00;                                              //record变量区长度
/*
__aram_kws_comm_vma     = (__pdn_comm_vma + __aram_mic_size + __aram_gpa_size);      //kws代码区起始地址(size:0x2000)
__aram_kws_comm_size    = 0x2000;                                                   //kws代码区长度
__aram_kws_vma          = (__pdn_comm_vma + __aram_mic_size + __aram_gpa_size + __aram_kws_comm_size);   //kws变量区起始地址
__aram_kws_size         = 0x7E00;                                                   //kws变量区长度 -- 0x0001 E000
*/

__aram_dnn_comm_vma     = (__pdn_comm_vma + __aram_mic_size + __aram_gpa_size);            //dnn代码区起始地址(size:0x2000)
__aram_dnn_vma          = (__pdn_comm_vma + __aram_mic_size + __aram_gpa_size + 0x2000);   //dnn变量区起始地址
__aram_dnn_size         = 0x7E00;                                                   //dnn变量区长度 -- 0x0001 E000


MEMORY
{
    init                : org = __base,                 len = 512
    flash(rx)           : org = __base + 512,           len = __max_flash_size
    comm(rx)            : org = __comm_vma,             len = __comm_ram_size
    pdn_comm(rx)        : org = __pdn_comm_vma,         len = 0x1000

    stack               : org = 0x10400,                len = 1k
    data                : org = __data_vma,             len = __data_ram_size
    aram                : org = __aram_vma,             len = __aram_size
    bram                : org = __bram_vma,             len = __bram_size

    aram_mic            : org = __aram_mic_vma,         len = __aram_mic_size

    aram_msc            : org = __aram_msc_vma,         len = __aram_msc_size

    aram_mp3_comm(rx)   : org = __aram_mp3_comm_vma,    len = 0xd00
    aram_mp3            : org = __aram_mp3_vma,         len = __aram_mp3_size

    aram_esbc_comm(rx)  : org = __aram_esbc_comm_vma,   len = 0x2A00
    aram_esbc           : org = __aram_esbc_vma,        len = __aram_esbc_size

    aram_rec_comm(rx)   : org = __aram_rec_comm_vma,    len = 0x1400
    aram_rec            : org = __aram_rec_vma,         len = __aram_rec_size
/*
    aram_kws_comm(rx)   : org = __aram_kws_comm_vma,    len = __aram_kws_comm_size
    aram_kws            : org = __aram_kws_vma,         len = __aram_kws_size
*/
    aram_dnn_comm(rx)   : org = __aram_dnn_comm_vma,    len = 0x2000
    aram_dnn            : org = __aram_dnn_vma,         len = __aram_dnn_size

}

SECTIONS
{
    .init : {
        *(.reset)
    } > init

    .comm : {
        __comm_start = .;
        KEEP(*(.vector))
        *(.plt)
        *(.com_text*)
        *(.com_rodata*)
        *(.ains3_text*)
        *(.record*)
        *(.srodata*)
        *debug.o(.rodata*)
        *(.sleep_text*)
        *(.data*)
        *(.sdata*)
        . = ALIGN(512);
        __comm_end = .;
    } > comm AT > flash
    __comm_size = __comm_end - __comm_start;

    .pdn_comm : {
        *(.pdncom_text*)
        *(.pdncom_buf*)
        . = ALIGN(512);
    } > pdn_comm AT > flash

    .aram_mp3_comm : {
        *(.mp3_text*)
        . = ALIGN(512);
    } > aram_mp3_comm AT > flash

    .aram_esbc_comm : {
        *(.esbc.text*)
        . = ALIGN(512);
    } > aram_esbc_comm AT > flash
/*
    .aram_kws_comm : {
        *(.kws_text*)
        *(.kws_fft_text*)
        . = ALIGN(512);
    } > aram_kws_comm AT > flash
*/

    .aram_dnn_comm : {
        *(.dnn_text*)
        *(.dnn_fft_text*)
        . = ALIGN(512);
    } > aram_dnn_comm AT > flash


    .aram_rec_comm : {
        . = ALIGN(512);
    } > aram_rec_comm AT > flash

    .flash : {
        __code_start_update = .;
        *(.text.update*)
        . = ALIGN(512);
        __code_end_update = .;

        __code_start_mp3dec = .;
        *(.mp3dec*)
        . = ALIGN(512);
        __code_end_mp3dec = .;

        __code_start_ws = .;
        *(.ws_text*)
        . = ALIGN(512);
        __code_end_ws = .;

        __code_start_vad = .;
        *(.vad_text*)
        . = ALIGN(512);
        __code_end_vad = .;

        __code_start_fft = .;
        *(.rodata.fft*)
        . = ALIGN(512);
        __code_end_fft = .;

        __code_start_asr = .;
        /* *(.asr_text*) */
        . = ALIGN(512);
        __code_end_asr = .;

#if EXSPI_REC_AUTOTUNE_PLAY
        __code_start_autotune = .;
        *(.autotune_pro)
        . = ALIGN(512);
        __code_end_autotune = .;
#endif
/*
        __code_start_kws = .;
        *(.kws_voice*)
        *(.kws_rodata*)
        *(.kws_ftext*)
        . = ALIGN(512);
        __code_end_kws = .;
*/
#if SPK_DNN_EN
        __code_start_dnn = .;
        *(.dnn_voice*)
        *(.dnn_rodata*)
        *(.dnn_ftext*)
        . = ALIGN(512);
        __code_end_dnn = .;
#endif // SPK_DNN_EN

        *(.text*)
        *(.wavdec*)
        *(.rodata*)
        *(.rela*)
        LONG(0)
        . = ALIGN(512);
    } > flash

    .stack (NOLOAD) : {
        __irq_stack_start = .;
        . = 0x400;              /*中断的栈空间*//*运行时的栈空间*/
        __irq_stack = .;
    } > stack
    __irq_stack_size = __irq_stack - __irq_stack_start;

    .bss (NOLOAD) : {
        __bss_start = .;
        *(COMMON)
        *(.bss*)
        *(.sbss*)
        *(.buf*)
        *(.avio_buf)
        *(.rodata.l2enc)
        *(.huart_buf)
        *(.uart_buf*)
        *(.music_buf)
        __bss_end = .;
    } > data
    __bss_size = __bss_end - __bss_start;

    .aram_mic __aram_mic_vma (NOLOAD) : {
        *(.mic_buf)             /*mic采样buf*/
        *(.ws_asr.sum)          /*硬件tdnn*/
        #if ASR_AINS_EN
        *(.ains3_buf*)          /*ains3降噪*/
        #endif
/*
        *(.kws_data*)
        *(.kws_fft_data*)
*/
        #if SPK_DNN_EN
        *(.dnn_data*)
        *(.dnn_fft_data*)
        #endif // SPK_DNN_EN
    } > aram_mic

    .aram_rec __aram_rec_vma (NOLOAD) : {
        #if HUART_DEUMP_EN
        *(.huart_dump_buf)      /*huart打印音频数据缓存buf*/
        #endif

        #if (SPI1_AUDIO_EN && (!SPI_SLAVE_OR_MASTER_EN))
        *(.spi_recv_buf*)
        #endif

        *(.rec.obuf)
        *(.recenc.obuf)
        *(.l2enc_bit_buf)
        *(.l2enc_sb_buf)
        *(.l2enc.buf*)

        #if FUNC_REC_EN
        *(.mp2dec_buf)
        #endif

        #if EXSPI_REC_AUTOTUNE_PLAY
        *(.auto_mp3_inbuf)
        *(.autotune.scb)
        #endif

//        #if FFT_PROC_SAMPLE
//        *(.fft_buf.buf)
//        . = __aram_fft_eaddr;   /*控制FFT的实际数据起始地址*/
//        #endif
    } > aram_rec
/*
    .aram_kws __aram_kws_vma (NOLOAD) : {
        *(.kws.ram_buf*)
        *(.kws.ram_code*)
    } > aram_kws
*/

    .aram_dnn __aram_dnn_vma (NOLOAD) : {
        *(.kws.*)
    } > aram_dnn


    .aram_msc __aram_msc_vma (NOLOAD) : {
        *(.gpd_buf)             /*dac缓存buf */
    } > aram_msc

    .aram_mp3 __aram_mp3_vma (NOLOAD) : {
        *(.mp3_decram_buf*)
        *(.mp3buf.dec)

//        #if FFT_PROC_SAMPLE
//        . = __aram_fft_eaddr;
//        #endif
    } > aram_mp3

    .aram_esbc __aram_esbc_vma (NOLOAD) : {
        *(.esbc_buf*)

//        #if FFT_PROC_SAMPLE
//        . = __aram_fft_eaddr;
//        #endif
    } > aram_esbc

    .aram_fft __aram_vma (NOLOAD) : {
        *(.fft_buf.buf)
        . = __aram_fft_saddr;
        *(.fft_buf.m_st)
        . = __aram_size;
    } > aram

    .bram_asr __bram_vma (NOLOAD) : {
        . = 0x1000;
    } > bram

    /* 在.bss段之后，定义_end符号 */
    . = ALIGN(4);
    _end = .;
    PROVIDE(end = .);
}

/* Calc the lma */
__bank_size = SIZEOF(.flash);
__comm_lma = LOADADDR(.comm);
__comm_size = SIZEOF(.comm);
__pdn_comm_lma = LOADADDR(.pdn_comm);
__pdn_comm_size = SIZEOF(.pdn_comm);
__aram_mp3_comm_lma = LOADADDR(.aram_mp3_comm);
__aram_mp3_comm_size = SIZEOF(.aram_mp3_comm);
__aram_esbc_comm_lma = LOADADDR(.aram_esbc_comm);
__aram_esbc_comm_size = SIZEOF(.aram_esbc_comm);
/*
__aram_kws_comm_lma = LOADADDR(.aram_kws_comm);
__aram_kws_comm_size = SIZEOF(.aram_kws_comm);*/

__aram_dnn_comm_lma = LOADADDR(.aram_dnn_comm);
__aram_dnn_comm_size = SIZEOF(.aram_dnn_comm);

__aram_rec_comm_lma = LOADADDR(.aram_rec_comm);
__aram_rec_comm_size = SIZEOF(.aram_rec_comm);
