#ifndef __USART1_H
#define	__USART1_H

#include "stm32f10x.h"
#include <stdio.h>
#include <stdarg.h>

#define CMD_BUFFER_LEN 255

void UsartInit(void);

void usart2Printf(char *fmt, ...);
void GpuSend(char * buf1);
//int fputc(int ch, FILE *f);
//void USART1_printf(USART_TypeDef* USARTx, uint8_t *Data,...);

#endif /* __USART1_H */
