#define F_CPU 8000000

#include <stdint.h>
#include <avr/io.h>

#include "FreeRTOS.h"
#include "task.h"

#define LED0 0x01
#define LED2 0x04
#define LED4 0x10
#define LED6 0x40

struct task_args {
   uint16_t period_ms;
   uint8_t led;
};

void vTask(void *tArgs); 

int main( void )
{
   struct task_args tArgs[4] = {{1000,LED0}, {500,LED2}, {250,LED4}, {125,LED6}};

   DDRB = 0xFF; 
   PORTB = 0xFF;

   xTaskCreate(vTask, (const char *) "1Hz", 100, (void *) &tArgs[0], 1, NULL);
   xTaskCreate(vTask, (const char *) "2Hz", 100, (void *) &tArgs[1], 1, NULL);
   xTaskCreate(vTask, (const char *) "4Hz", 100, (void *) &tArgs[2], 1, NULL);
   xTaskCreate(vTask, (const char *) "8Hz", 100, (void *) &tArgs[3], 1, NULL);

   vTaskStartScheduler();

   return 0;
}

void vTask(void *tArgs)
{
   struct task_args *args = (struct task_args *) tArgs; 

   for (;;)  {
      PORTB ^= args->led;
      vTaskDelay( (args->period_ms / 2) / portTICK_RATE_MS );
   }
}
