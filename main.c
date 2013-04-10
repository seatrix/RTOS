/************************************************************
 * Filename: main.c
 * Blinks LED0 and LED2 if SW0 is pressed
 * Blinks LED4 and LED6 if SW0 is not pressed
 * Authors: Daniel Jennings, Matt Zimmerer
 * Revision log:    created 4/2/13
 ***********************************************************/

#define __DELAY_BACKWARD_COMPATIBLE__
#define F_CPU 8000000UL

// Calculating delay count (HARDWARE SPECIFIC):
#define DESIRED_FREQ_HZ 2
//    Given a desired frequency, a naive count can be taken as follows:
#define NAIVE_COUNT F_CPU / DESIRED_FREQ_HZ
//    For our application, we want to double the frequency of our dumb delay.
//    This can be understood by observing that the delay is used to toggle an
//    LED, and two toggles will complete on full period.
#define NAIVE_HALF_COUNT NAIVE_COUNT / 2
//    A for loop involving one 32bit value, requires 23 atomic operations
//    per loop (observed from '-O3' optimised assembly). Thus, an acurate count
//    can be calculated as follows:
#define ACCU_COUNT NAIVE_HALF_COUNT / 23

#include <avr/io.h> 
#include <avr/interrupt.h>
#include <stdint.h>
#include "init.h"

// A state variable that reflects the state of button0
static volatile uint8_t state = 0;

int main(void)
{
   // A count variable. It must be declared volatile so that it survives any
   // optimisation used.
	static volatile uint32_t i;

	DDRA = 0x00; //Data direction register port A set to input 
	DDRB = 0xFF; //Data direction register port B set to output	

	initISR(); // Initialize registers for both timer interrupts
	PORTB = 0xFF; // Turn off all LEDs (active low)

	while(1){
	   if(PINA & (1 << PA0))
	   {
         // If PORTA pin 0 is pressed
		   state = 1; // Notify ISR handlers that button is pressed
         for (i = 0; i < ACCU_COUNT/4; i++); // Delay of 500ms
		   PORTB ^= 1<<PB4; // Toggle LED4
		   PORTB |=  1<<PB0; // Turn off LED0
	   }
	   else
	   {
         // If PORTA pin 0 is not pressed
		   state = 0; // Notify the ISR handlers that button is not pressed
         for (i = 0; i < ACCU_COUNT/2; i++); // Delay of 500ms
		   PORTB ^= 1<<PB0; // Toggle LED0
		   PORTB |= 1<<PB4; // Turn off LED4
	   }
	}

	return 0;
}

ISR(TIMER0_COMPA_vect)
{
	if (state == 0){ // If button is not pressed
		PORTB ^= 1<<PB2; // Toggle LED2
      PORTB |= 1<<PB6; // Turn of LED6
   }
}

ISR(TIMER1_COMPA_vect)
{
	if (state == 1){ // If button is pressed
		PORTB ^= 1<<PB6; // Toggle LED6
      PORTB |= 1<<PB2; // Turn off LED2
   }
}
