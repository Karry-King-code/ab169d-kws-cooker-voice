#include "s_common.h"
#include "config.h"

.global _start
.section .reset, "ax"
_start:
    la      tp, _tp
    la      a0, __irq_stack_start           //Stack清成0x23
    li      a1, 0x23
    la      a2, __irq_stack_size
    la      x1, memset
    jalr    x1
    la      x1, __irq_stack
    mv      sp, x1
    la      x1, platform_main
    jr      x1
    la      x1, _exception_isr
    jr      x1

.section .vector, "ax"
    .org    0x04
    .long   _exception_isr

    //Hardware breakpoint
    .org    0x08
    .long   bp_isr

    //extend ebreak instruction
    .org    0x0c
    .long   bp_ret

    //Low priority interrupt
    .org    0x10
    .long   low_prio_irq

    //watch point interrupt
    .org    0x14
    .long   _watch_point

    //cache miss interrupt
    .org    0x20
    .long   isr_cache

    //software interrupt
    .org    0x28
    .long   nop_isr

    //timer0 interrupt
    .org    0x2c
    .long   nop_isr

    //timer1 interrupt
    .org    0x30
    .long   nop_isr

    //timer2 interrupt
    .org    0x34
    .long   nop_isr

    //irtx interupt
    .org    0x38
    .long   nop_isr

    //sd interrupt
    .org    0x40
    .long   nop_isr

    //adc dma interrupt
    .org    0x48
    .long   nop_isr

    //adc sample interrupt
    .org    0x54
    .long   nop_isr

    //uart0 interrupt
    .org    0x58
    .long   nop_isr

    //hsuart interrupt
    .org    0x5c
    .long   nop_isr

    //pwm interrupt
    .org    0x60
    .long   nop_isr

    //dac dmain interrupt
    .org    0x6c
    .long   nop_isr

    //spi0/1 interrupt
    .org    0x70
    .long   nop_isr

    //uart0 match interrupt
    .org    0x74
    .long   nop_isr

    //fre_det/touchkey interrupt
    .org    0x7c
    .long   nop_isr

    //port interrupt
    .org    0x88
    .long   nop_isr

    //saradc interrupt
    .org    0x90
    .long   nop_isr

    //RTC/alarm/LVD/WDT interrupt
    .org    0x94
    .long   lvd_isr

    //iic interrupt
    .org    0x98
    .long   nop_isr

    .org    0xa0
    .global nop_isr
nop_isr:
1:  j       1b
    nop
    mret
