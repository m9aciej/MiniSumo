/*
 * muart.c    ATmega8    F_CPU = 8000000 Hz
 * 
 * Created on: 2017-04-30
 *     Author: admin
*/   

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <util/delay.h>

#include "uart.h"


void uart_putc(char data ) {
	while ( !( UCSRA & ( 1 << UDRE ) ) );
	UDR = data;
}

void uart_puts(char *str)
{
	while(*str) uart_putc(*str++);
}

void uart_putint(int val)
{
	char buf[7];
	itoa(val,buf,10);
	uart_puts(buf);
}

void uart_Init( uint16_t baud ) {

	UBRRH = ( uint8_t )( baud >> 8 );
	UBRRL = ( uint8_t )baud;
	UCSRB = ( 1 << RXEN ) | ( 1 << TXEN );
	UCSRC = ( 1 << URSEL )  | ( 3 << UCSZ0 );

}

char uart_getc( void )
{
	while ( !(UCSRA & (1<<RXC)) );
	return UDR;
}       