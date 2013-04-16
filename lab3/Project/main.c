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

#define LED0 0x01	//Definition of LED pins
#define LED2 0x04
#define LED4 0x10
#define LED6 0x40

struct task_args {
   uint16_t period_ms;	//Struct for period of blinking, and LED character
   uint8_t led;
};

void vTask(void *tArgs);//Prototype for vTask

int main( void )
{			//Struct initialization with periods and respective LEDs
   struct task_args tArgs[4] = {{1000,LED0}, {500,LED2}, {250,LED4}, {125,LED6}};

   DDRB = 0xFF; 	//Sets LED port to output, and off
   PORTB = 0xFF;
			//Task calls for all 4 LEDs
   xTaskCreate(vTask, (const char *) "1Hz", 100, (void *) &tArgs[0], 1, NULL);
   xTaskCreate(vTask, (const char *) "2Hz", 100, (void *) &tArgs[1], 1, NULL);
   xTaskCreate(vTask, (const char *) "4Hz", 100, (void *) &tArgs[2], 1, NULL);
   xTaskCreate(vTask, (const char *) "8Hz", 100, (void *) &tArgs[3], 1, NULL);

   vTaskStartScheduler();//Starts the Tasks running

   return 0;
}

void vTask(void *tArgs)	//Task Definition for vTask
{
			//Struct for using tArgs in vTask
   struct task_args *args = (struct task_args *) tArgs; 
			
   for (;;)  {
      PORTB ^= args->led;//Toggle LED, wait the given period
      vTaskDelay( (args->period_ms / 2) / portTICK_RATE_MS );
   }
}
