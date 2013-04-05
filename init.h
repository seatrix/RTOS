/************************************************************
 * Filename: init.h
 * Initializes the ISR for timers used in blinky LEDs
 * Authors: Daniel Jennings, Matt Zimmerer
 * Revision log:    created 4/2/13
 ***********************************************************/

#ifndef _INIT_H
#define _INIT_H

// Initialize interrupts for ISR(TIMER0_COMPA_vect) and ISR(TIMER1_COMPA_vect)
// Both timers use a prescaler of 256, and a count of 2. The final frequency
// is then 15625Hz.
void initISR();

#endif
