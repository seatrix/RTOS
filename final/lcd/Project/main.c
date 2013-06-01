#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "lcd.h"

static xSemaphoreHandle globalSem;

static uint16_t count = 0;
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
      lcdprintf(0, " R%03i G%03i B%03i", red, green, blue);
      lcdprintf(1, " C%03i M%03i Y%03i", 255-red, 255-green, 255-blue);
      vTaskDelay(200 / portTICK_RATE_MS);
   }
}

int main(void)
{
   xTaskCreate(ColorTask, (const signed char *) "color", 100, NULL, 2, NULL);
   xTaskCreate(TextTask, (const signed char *) "text", 100, NULL, 1, NULL);
   vTaskStartScheduler();

   return 0;
}
