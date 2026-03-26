/*
 * comp_test_emb.c
 *
 * Created: 25/03/2026 5:42:41 p. m.
 * Author : Institucional
 */ 

#include <avr/io.h>
#include "headers.h"


int main(void)
{
    ADC_init();
	uart_init(MYUBRR);
	DDRB=0xFF;
	PORTE|=(1<<PE4);
    while (1) 
    {
		uart_print("X=");
		send(0);
		uart_print(" Y=");
		send(1);
		uart_print("\n\r");
		if((PINE&0x04)==0)
		{PORTB=(0<<PB7);}
		_delay_ms(250);
    }
}

