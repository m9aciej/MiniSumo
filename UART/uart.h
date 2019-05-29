
/*
 * uart.h    ATmega8    F_CPU = 8000000 Hz
 * 
 * Created on: 2017-04-30
 *     Author: admin
*/   

#ifndef UART_H_
#define UART_H_


#define UART_BAUD 9600	//def prednosci
#define __UBRR ((F_CPU+UART_BAUD*8UL) / (16UL*UART_BAUD)-1) // obliczamy UBRR


// deklaracje funkcji

void uart_Init(uint16_t baud);

char uart_getc(void);
void uart_putc(char data);
void uart_puts(char *str);
void uart_putint(int val);

#endif /* UART_H_ */
