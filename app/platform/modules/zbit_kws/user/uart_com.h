#ifndef __UART_COM_H__
#define __UART_COM_H__

#include "typedef.h"

//void uart_com_init(int baud);
int uart_com_write(uint8_t* data, int len);
int uart_com_read(uint8_t* data, int len);

#endif
