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

void vHandlerTask(void *tArgs);
void vLEDTask(void *tArgs);

int main( void )
{
   DDRB = 0xFF;
   PORTB = 0xFF;

   xTaskCreate(vTask, (const char *) "1Hz", 100, (void *) &tArgs[0], 1, NULL);

   // Kick off the scheduler
   vTaskStartScheduler();

   return 0;
}

void vTask(void *tArgs)
{
   for (;;)  {
   }
}
