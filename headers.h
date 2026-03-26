/*
 * headers.h
 *
 * Created: 25/03/2026 5:43:37 p. m.
 *  Author: Institucional
 */ 


#ifndef HEADERS_H_
#define HEADERS_H_

#define F_CPU 16000000UL
#include <util/delay.h>
#include <stdio.h>
#define BAUD 57600
#define MYUBRR F_CPU/16/BAUD-1

void uart_init(unsigned int ubrr);
void ADC_init();
unsigned int ADC_read(unsigned char adc_input);
void uart_transmit(unsigned char data);
void uart_print(const char *str);
void send (uint8_t ch);

#endif /* HEADERS_H_ */