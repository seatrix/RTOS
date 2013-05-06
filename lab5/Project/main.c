/*
 * Project: Lab4 SPI seven segment display, with multiple tasks
 *
 * Authors: Matt Zimmerer, Daniel Jennings
 *
 * Version: Lab04 version 1.0
 */

#define F_CPU 8000000L
#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "spi_sseg.h"

// LED definitions
#define LEDDDR  DDRF
#define LEDPORT PORTF
#define LED0    (1<<PF0)
#define LED2    (1<<PF2)

typedef const signed char * cscharp;

typedef struct taskJob {
   uint16_t period;  // The period to blink the LED (square wave period)
   uint8_t LED;      // The LED to blink
   uint8_t dispSide; // 0 is left, 1 is right
   uint8_t counter;  // The state transition count
} Job;

void Blink(void *tArgs);

static xSemaphoreHandle rSSEG;

int main(void)
{
   Job job5Hz = {200, LED0, 0, 0};  // LED0 Blinks at 5Hz, displays on left
   Job job10Hz = {100, LED2, 1, 0}; // LED2 Blinks at 10Hz, displays on right

   // Set data direction register for LEDs
   LEDDDR = LED0 | LED2;

   // Initialize SSEG display
   _delay_ms(1000); // Give the SSEG a grace period
   SPI_MasterInit(); // Initialze ATMEGA spi interface
   SSEG_Reset(); // Reset the SSEG device
   SSEG_Set_Brightness(0); // Set brightness to max

   // Create SSEG resource semaphore
   vSemaphoreCreateBinary(rSSEG);

   // Creat the LED Blinky tasks
   xTaskCreate(Blink, (cscharp) "5HZ", 100, (void*) &job5Hz, 1, NULL);
   xTaskCreate(Blink, (cscharp) "10HZ", 100, (void*) &job10Hz, 1, NULL);

   // Kick off the scheduler
   vTaskStartScheduler();

   return 0;
}

// Tick rate is configured for 500Hz, so toggling the LED every 250th call
// will yield 1Hz
void vApplicationTickHook()
{
   // Higher priority task worken = false
   static portBASE_TYPE xHPTW = pdFALSE;
   static uint8_t colonState = 0;
   static uint16_t counter = 0;

   // Reduce the frequency of this code
   if (++counter == 250) {

      // Take the SSEG resource semaphore
      xSemaphoreTakeFromISR(rSSEG, &xHPTW);

      // Toggle colon state
      if (colonState ^= 1)
         SSEG_Write_Decimal_Point(4);
      else
         SSEG_Clear_Decimal_Point(4);

      // Release the SSEG resource semaphore
      xSemaphoreGiveFromISR(rSSEG, &xHPTW);

      counter = 0;
   }
}

void Blink(void *tArgs)
{
   Job *currJob = (Job*) tArgs;

   for (;;) {

      // Toggle target LED
      LEDPORT ^= currJob->LED;

      // Take the SSEG resources
      xSemaphoreTake(rSSEG, portMAX_DELAY);

      // Increment the state counter
      currJob->counter++;
      if (currJob->counter >= 100)
         currJob->counter = 0;

      // Display counter value on correct side of SSEG
      if (currJob->dispSide == 0)
         SSEG_Write_left_digits(currJob->counter);
      else
         SSEG_Write_right_digits(currJob->counter);
      
      // Release the SSEG resources
      xSemaphoreGive(rSSEG);

      // Delay a half period
      vTaskDelay((currJob->period/2)/portTICK_RATE_MS);
   }
}
