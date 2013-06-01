#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "lcd.h"

struct Time {
   uint16_t year;
   uint8_t month;
   uint8_t day;
   uint8_t hour;
   uint8_t min;
   uint8_t sec;
};

struct ClockState {
   struct Time time;
   uint8_t on;
};

static xSemaphoreHandle clockStateSem;
struct ClockState clockState;

// Test variables
static unsigned char red = 255;
static unsigned char green = 0;
static unsigned char blue = 0;

static void updateClock()
{
   while (clockState.time.sec > 59) {
      clockState.time.sec = 0;
      clockState.time.min++;
   }
   while (clockState.time.min > 59) {
      clockState.time.min = 0;
      clockState.time.hour++;
   }
   while (clockState.time.hour > 23) {
      clockState.time.hour = 0;
      clockState.time.day++;
   }
   while (clockState.time.day > 30) { // TODO day per month LUT
      clockState.time.day = 1;
      clockState.time.month++;
   }
   while (clockState.time.month > 12) {
      clockState.time.month = 1;
      clockState.time.year++;
   }
}

void ColorTask(void *args)
{
   unsigned char VR = 1;
   unsigned char VG = -1;
   unsigned char VB = 1;

   setupLCD();

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
   char writebuff[2][17];

   writebuff[0][10] = '\0';
   writebuff[1][8] = '\0';
   
   while(1) {

      xSemaphoreTake(clockStateSem, portMAX_DELAY);

      // Manually build large time string
      writebuff[0][0] = clockState.time.year/1000 + '0';
      writebuff[0][1] = (clockState.time.year/100)%10 + '0';
      writebuff[0][2] = (clockState.time.year/10)%10 + '0';
      writebuff[0][3] = (clockState.time.year)%10 + '0';
      writebuff[0][4] = '-';
      writebuff[0][5] = clockState.time.month/10 + '0';
      writebuff[0][6] = clockState.time.month%10 + '0';
      writebuff[0][7] = '-';
      writebuff[0][8] = clockState.time.day/10 + '0';
      writebuff[0][9] = clockState.time.day%10 + '0';

      // Manually build small time string
      writebuff[1][0] = clockState.time.hour/10 + '0';
      writebuff[1][1] = clockState.time.hour%10 + '0';
      writebuff[1][2] = ':';
      writebuff[1][3] = clockState.time.min/10 + '0';
      writebuff[1][4] = clockState.time.min%10 + '0';
      writebuff[1][5] = ':';
      writebuff[1][6] = clockState.time.sec/10 + '0';
      writebuff[1][7] = clockState.time.sec%10 + '0';

      xSemaphoreGive(clockStateSem);

      lcdprint(0, writebuff[0]);
      lcdprint(1, writebuff[1]);

      vTaskDelay(200 / portTICK_RATE_MS);
   }
}

void ClockTask(void *args)
{
   portTickType lastWakeTime;
	lastWakeTime = xTaskGetTickCount();

   while(1) {
      xSemaphoreTake(clockStateSem, portMAX_DELAY);
      clockState.time.sec += 1;
      updateClock();
      xSemaphoreGive(clockStateSem);

      vTaskDelayUntil(&lastWakeTime, 1000 / portTICK_RATE_MS);
   }
}

int main(void)
{
   // Initialize time to 1970 epoch
   clockState.time.year = 1970;
   clockState.time.month = 1;
   clockState.time.day = 1;
   clockState.time.hour = 0;
   clockState.time.min = 0;
   clockState.time.sec = 0;
   clockState.on = 1;

   vSemaphoreCreateBinary(clockStateSem);

   xTaskCreate(ColorTask, (const signed char *) "color", 1000, NULL, 2, NULL);
   xTaskCreate(TextTask, (const signed char *) "text", 1000, NULL, 1, NULL);
   xTaskCreate(ClockTask, (const signed char *) "clock", 1000, NULL, 3, NULL);
   vTaskStartScheduler();

   return 0;
}
