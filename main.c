/*
 * Demonstration on how to redirect stdio to UART. 
 *
 * http://appelsiini.net/2011/simple-usart-with-avr-libc
 *
 * To compile and upload run: make clean; make; make program;
 * Connect to serial with: screen /dev/tty.usbserial-*
 *
 * Copyright 2011 Mika Tuupola
 *
 * Licensed under the MIT license:
 *   http://www.opensource.org/licenses/mit-license.php
 *
 */
 
#include <stdio.h>

#include "main.h"
#include "uart.h"

#include <stdint.h>
#include <math.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define DELAY 100
#define UDELAY 3
#define SLEN 30
#define TM 20

volatile uint8_t ADCvalue, cosVal, sinVal;
void adc_prep(void);
int ssi_read(int b);

int main(void) {    
	
	adc_prep();
	double angle, sinValN, cosValN;
	DDRD = (0<<DDD6)|(1<<DDD5);
	int sangle;

	puts("Hello, angle!");
	while(1) {

		sangle = ssi_read(9);
		printf(" -> %f", (double)sangle/512*2*M_PI);
		
		printf(" |A: ");
		sinValN = (double) sinVal/128-1;
		cosValN = (double) cosVal/128-1;
		angle = atan2(sinValN, (double)cosValN);

		printf("(%f, %f) -> ", sinValN, cosValN);
		printf("%f\n", angle);
		_delay_ms(DELAY);
	}
	
	return 0;
}

int ssi_read(int b)
{
		int i, sangle=0, data[b];

		PORTD = (1<<DDD5);
		_delay_us(UDELAY);
		PORTD = (0<<DDD5);
		_delay_us(UDELAY); // begin read	

		for (i=0; i<9; i++)
		{
			PORTD = (1<<DDD5);
			_delay_us(UDELAY);

			data[i] = (PIND & (1<<DDD6)) == (1<<DDD6) ? 1 : 0;

			PORTD = (0<<DDD5);
			_delay_us(UDELAY);
		}

		_delay_us(TM); // end read
		
		printf("S: ");
		for (i=0; i<9; i++)
		{
			printf("%d", data[i]);
			sangle += data[i] << (8-i);
		}
		return sangle;
}
		
void adc_prep(void)
{	
// https://sites.google.com/site/qeewiki/books/avr-guide/analog-input
	uart_init();
	stdout = &uart_output;
	stdin  = &uart_input;
	
	//char input;
	ADMUX = 0;              // use ADC0
	ADMUX |= (1<<REFS0);    // use AVcc as the reference
	ADMUX |= (1<<ADLAR);    // Right adjust for 8 bit resolution

	ADCSRA |= (1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0); // 128 prescale 16Mhz
	ADCSRA |= (1<<ADATE);   // Set ADC Auto Trigger Enable
	ADCSRB = 0;             // 0 for free running mode
	ADCSRA |= (1<<ADEN);    // Enable the ADC
	ADCSRA |= (1<<ADIE);    // Enable Interrupts 

	ADCSRA |= (1<<ADSC);    // Start the ADC conversion
	
	sei(); // Because..?
}

ISR(ADC_vect)
{
	uint8_t tmp;   // temp register for storage of misc data

	tmp = ADMUX;   // read the value of ADMUX register
	tmp &= 0x0F;   // AND the first 4 bits (value of ADC pin being used)

	ADCvalue = ADCH;// read the sensor value

	if (tmp == 0)
	{
		// put ADCvalue into whatever register you use for ADC0 sensor
		cosVal = ADCvalue;
		ADMUX++;     // add 1 to ADMUX to go to the next sensor
	} else if (tmp == 1)
	{
		// put ADCvalue into whatever register you use for ADC1 sensor
		sinVal = ADCvalue;
		ADMUX &= 0xF8; // clear the last 4 bits to reset the mux to ADC0
	}
} 
