/*
* TODO
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
#include "semphr.h"
#include "task.h"

void vHdlrTask(void *tArgs);
void vLEDTask(void *tArgs);

xSemaphoreHandle xISRSemaphore;

static void init_isr()
{
   EICRA = 0x00; // INT0-6 not configured
   EICRB = 0x80; // INT7 falling edge triggered
   EIMSK = 0x80; // Enable INT7
   sei();
}

int main( void )
{
   DDRB = 0xFF;
   DDRE = 0x00;
   PORTB = 0x00;

   init_isr();

   vSemaphoreCreateBinary(xISRSemaphore);

   xTaskCreate(vHdlrTask, (const signed char *) "HDLR", 100, NULL, 2, NULL);
   xTaskCreate(vLEDTask, (const signed char *) "LED", 100, NULL, 1, NULL);

   // Kick off the scheduler
   vTaskStartScheduler();

   return 0;
}

void vHdlrTask(void *tArgs)
{
   for (;;) {
      xSemaphoreTake(xISRSemaphore, portMAX_DELAY);
      PORTB ^= 0xFF;
   }
}

void vLEDTask(void *tArgs)
{
   for (;;) {
   }
}

#define SW7 (1 << 7)

ISR(INT7_vect)
{
   static portBASE_TYPE xHPTW = pdFALSE;

   // Debouncing logic
   _delay_ms(50);
   if (PORTE & SW7)
      return;

   xSemaphoreGiveFromISR(xISRSemaphore, &xHPTW);
}
