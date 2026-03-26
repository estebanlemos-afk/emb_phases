/*
 * implmt.c
 *
 * Created: 25/03/2026 5:43:48 p. m.
 *  Author: Institucional
 */ 
#include <avr/io.h>
#include "headers.h"

void uart_init(unsigned int ubrr) {
	UBRR0H =(unsigned char)(ubrr >>8);
	UBRR0L =(unsigned char)ubrr;
	UCSR0B =(1 <<RXEN0) |(1 << TXEN0);
	UCSR0C =(1 <<UCSZ01)| (1<< UCSZ00);
}
//88888888888888888888888888888888888888888888888888888888
void ADC_init() {
	DIDR0=0x00;
	DIDR2=0x00;
	ADMUX=0x40;
	ADCSRA=0x84;
	ADCSRB=0x00;
}
//88888888888888888888888888888888888888888888888888888888
unsigned int ADC_read(unsigned char adc_input)
{
	ADMUX=(adc_input & 0x1F) | 0X40;
	if(adc_input& 0x20) ADCSRB|=(1<<MUX5);
	else ADCSRB&=~(1<<MUX5);
	_delay_us(1);
	ADCSRA|=(1<<ADSC);
	while ((ADCSRA & (1<<ADIF))==0);
	ADCSRA|=(1<<ADIF);
	return ADCW;
}
//88888888888888888888888888888888888888888888888888888888888888888888
void uart_transmit(unsigned char data) {
	while (!(UCSR0A & (1 << UDRE0)));
	UDR0 = data;
}
//88888888888888888888888888888888888888888888888888888888888888888888
void uart_print(const char *str) {
	while (*str) {
		uart_transmit(*str++);
	}
}
//88888888888888888888888888888888888888888888888888888888888888888888
void send (uint8_t ch)
{
	uint8_t buffer[16];
	uint16_t val=0;
	val=ADC_read(ch);
	sprintf(buffer,"%d",val);
	uart_print(buffer);
}