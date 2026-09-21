#include "include.h"

//Set ADC remote dc filter select.
void sdadc_rmdc_filter_sel_set(SDADC_RMDC_FILTER_SEL_E rmdc_ft_sel)
{
    u32 sdadccon = SDADCCON;
    sdadccon = (sdadccon & ~(0x01 << 12)) | ((rmdc_ft_sel & 0x01) << 12);
    SDADCCON = sdadccon;
}
//Get ADC remote dc filter select.
u32 sdadc_rmdc_filter_sel_get(void)
{
    return (SDADCCON & (0x1 << 12));
}

//Set getDC bits_len function.
void sdadc_getdc_sel_set(SDADC_GETDC_SEL_E getdc_sel)
{
    u32 sdadccon = SDADCCON;
    sdadccon = (sdadccon & ~(0x03 << 10)) | ((getdc_sel & 0x03) << 10);
    SDADCCON = sdadccon;
}

//Get getDC bits_len function.
u32 sdadc_getdc_sel_get(void)
{
    return (SDADCCON & (0x3 << 10));
}

//Set removeDC bits_len function.
void sdadc_rmdc_sel_set(SDADC_RMDC_SEL_E rmdc_sel)
{
    u32 sdadccon = SDADCCON;
    sdadccon = (sdadccon & ~(0x03 << 8)) | ((rmdc_sel & 0x03) << 8);
    SDADCCON = sdadccon;
}

//Get removeDC bits_len function.
u32 sdadc_rmdc_sel_get(void)
{
    return (SDADCCON & (0x3 << 8));
}