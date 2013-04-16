/*
*Lab02 Blink LED0 @ 1Hz, LED2 @ 2Hz, LED4 @ 4Hz, and LED6 @ 8Hz
*
*Authors: Matt Zimmerer, Daniel Jennings
*
*Version: Lab02 version 1.0
*/
#define F_CPU 8000000

#include <stdint.h>
#include <avr/io.h>

#include "FreeRTOS.h"
#include "task.h"

void vHdlrTask(void *tArgs);
void vLEDTask(void *tArgs);

int main( void )
{
   DDRB = 0xFF;
   PORTB = 0xFF;

   xTaskCreate(vHdlrTask, (const char *) "HDLR", 100, NULL, 2, NULL);
   xTaskCreate(vLEDTask, (const char *) "LED", 100, NULL, 1, NULL);

   // Kick off the scheduler
   vTaskStartScheduler();

   return 0;
}

void vHdlrTask(void *tArgs)
{
   for (;;) {
   }
}

void vLEDTask(void *tArgs)
{
   for (;;) {
   }
}
