#ifndef __BSP_SDADC_H
#define __BSP_SDADC_H


//ADC remove dc filter select.
typedef enum{
    RMDC_FILTER_SEL_HPASS,  //highpass filter
    RMDC_FILTER_SEL_LPASS,  //lowpass filter
}SDADC_RMDC_FILTER_SEL_E;

//GETDC Bits Enumeration.
typedef enum{
    GETDC_SEL_13BITS,
    GETDC_SEL_12BITS,
    GETDC_SEL_11BITS,
    GETDC_SEL_10BITS,
}SDADC_GETDC_SEL_E;

//RemoveDC Bits Enumeration.
typedef enum{
    RMDC_SEL_10BITS,
    RMDC_SEL_9BITS,
    RMDC_SEL_8BITS,
    RMDC_SEL_7BITS,
}SDADC_RMDC_SEL_E;

#endif // __BSP_SDADC_H

