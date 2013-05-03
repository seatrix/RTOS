/*
*Project: Lab3 Semaphores
*
*
*Authors: Matt Zimmerer, Daniel Jennings
*
*Version: Lab03 version 1.0
*/
#define F_CPU 16000000L

#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "FreeRTOS.h"
//#include "semphr.h"
#include "task.h"
#include "spi_sseg.h"

void Task(void *tArgs);

static void init_isr()
{
   EICRA = 0x00; // INT0-6 not configured
   EICRB = 0x40; // INT7 falling edge triggered
   EIMSK = 0x80; // Enable INT7
}

static void init_timer()
{
   TCCR2A = 0xC0; // Set timer2 to CTC mode, sets OC2A on compare match
   TCCR2B = 0x0F; // Set prescaler to divide by 1024
   OCR2A = 162;   // Set output compare reg to 162
   TIMSK2 = 0x02; // Enables interrupt on compare match A
}

int main(void)
{

   //init_isr();//initializing ISR
   //init_timer();//initializing timer
   //sei();//enabling interrupts

   SPI_MasterInit(); 

   //xTaskCreate(Task, (const signed char *) "TIMHDLR", 100, NULL, 3, NULL);

   // Kick off the scheduler
   //vTaskStartScheduler();

   return 0;
}

void Task(void *tArgs)
{
   for (;;) {
   }
}

ISR(INT7_vect)
{
}

ISR(TIMER2_COMPA_vect)
{
}
