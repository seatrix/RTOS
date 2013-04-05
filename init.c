/************************************************************
 * Filename: init.c
 * Initializes the ISR for timers used in blinky LEDs
 * Authors: Daniel Jennings, Matt Zimmerer
 * Revision log:    created 4/2/13
 ***********************************************************/

#include<avr/io.h> 
#include "init.h"
#include <avr/interrupt.h>

void initISR()
{
   // Timer 0
	TCCR0A 	= 0xC0; //set timer0 to CTC mode, sets OC0A on compare match
	TCCR0B 	= 0x04; //set prescaler to divide by 256 (frequency at 31250Hz)
	OCR0A 	= 0x01; //output compare match register set frequency to 15625Hz
	TIMSK0	= 0x02; //enables interrupt on compare match A

   // Timer 1
	TCCR1A 	= 0xC0; //set timer0 to CTC mode, sets OC0A on compare match
	TCCR1B 	= 0x0B; //set prescaler to divide by 256 (frequency at 31250Hz)
	OCR1AH 	= 0x00; //set the top 8 bits of output compare reg to 0
	OCR1AL 	= 0x01; //set the bottom 8 bits of output compare reg to 4
	TIMSK1	= 0x02; //enables interrupt on compare match A
	
	sei(); //enable interrupts
}
