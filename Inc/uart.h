#ifndef UART_H_
#define UART_H_
#include <stdio.h>
#define STM32F411xE

#include "stm32f4xx.h"

void uart2_write(int ch);

void uart2_rxtx_int_init(void);

char uart2_read();

void uart2_tx_init(void);

void dma1_stream6_init(uint32_t src, uint32_t dst, uint32_t len);

#endif /* UART_H_ */
