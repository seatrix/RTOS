#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "lcd.h"

// Clockstate
static xSemaphoreHandle clockStateSem; // State semaphore
uint32_t sec;  // Value in seconds since epoch (Jan 1 1970)
uint8_t on;    // 1 == on, 0 == off

// Test variables
static unsigned char red = 255;
static unsigned char green = 0;
static unsigned char blue = 0;

void ColorTask(void *args)
{
   unsigned char VR = 1;
   unsigned char VG = -1;
   unsigned char VB = 1;

   setupLCD();

   lcdprintf(0, "Color test");

   while(1) {
      if (red == 255)
         VR = -1;
      if (red == 0)
         VR = 1;
      if (green == 255)
         VG = -3;
      if (green == 0)
         VG = 3;
      if (blue == 254)
         VB = -2;
      if (blue == 0)
         VB = 2;

      red += VR;
      green += VG;
      blue += VB;
      lcd_setColorRGB(red, green, blue);

      vTaskDelay(20 / portTICK_RATE_MS);
   }
}

void TextTask(void *args)
{
   while(1) {
      xSemaphoreTake(clockStateSem, portMAX_DELAY);
      lcdprintf(0, "%i", sec); // TODO problem with variadic arguments
      xSemaphoreGive(clockStateSem);

      lcdprintf(1, " R%03i G%03i B%03i", red, green, blue);

      vTaskDelay(200 / portTICK_RATE_MS);
   }
}

void ClockTask(void *args)
{
   portTickType lastWakeTime;
	lastWakeTime = xTaskGetTickCount();

   while(1) {
      xSemaphoreTake(clockStateSem, portMAX_DELAY);
      sec += 1;
      xSemaphoreGive(clockStateSem);

      vTaskDelayUntil(&lastWakeTime, 1000 / portTICK_RATE_MS);
   }
}

int main(void)
{
   sec = 2;
   msec = 1;
   on = 1;

   vSemaphoreCreateBinary(clockStateSem);

   xTaskCreate(ColorTask, (const signed char *) "color", 1000, NULL, 2, NULL);
   xTaskCreate(TextTask, (const signed char *) "text", 1000, NULL, 1, NULL);
   xTaskCreate(ClockTask, (const signed char *) "clock", 1000, NULL, 3, NULL);
   vTaskStartScheduler();

   return 0;
}
