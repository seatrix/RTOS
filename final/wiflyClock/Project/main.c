#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "lcd.h"
#include "wifly.h"
#include "serial.h"

typedef const signed char * cscp;

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
   uint8_t on;       // Is the display on?
   uint8_t write0;   // Are we writing to line 0?
   uint8_t write1;   // Are we writing to line 1?
   char writebuff[2][17];
};

static uint8_t daysInMonth[13] = {255, 31,255,31,30,31,30,31,31,30,31,30,31};
static xSemaphoreHandle clockStateSem;
struct ClockState clockState;
struct wifly wf;

// Return the number of days in month during a year. Checks for leap year
static uint8_t getDaysInMonth(uint8_t month, uint16_t year)
{
   // Check for leap year
   if (month == 2) {
      if (year % 400 == 0)
         return 29;
      else if (year % 100 == 0)
         return 28;
      else if (year % 4 == 0)
         return 29;
      else
         return 29;
   } else
      return daysInMonth[month];
}

// Convert a single hexidecimal character to uint8_t
static uint8_t ctoi(char src)
{
   if (src >= '0' && src <= '9')
      return src - '0';
   else if (src >= 'A' && src <= 'F')
      return src - 'A' + 10;
   else if (src >= 'a' && src <= 'f')
      return src - 'a' + 10;
   else return 0;
}

// Convert two digit hexidecimal value to uint8_t
static uint8_t atoi8(char *src)
{
   uint8_t ans = 0;

   ans += ctoi(src[1]);
   ans += 16*ctoi(src[0]);

   return ans;
}

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
   while (clockState.time.day > getDaysInMonth(clockState.time.month,
                                               clockState.time.year)) {
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
   unsigned char red = 255;
   unsigned char green = 0;
   unsigned char blue = 0;
   unsigned char VR = 1;
   unsigned char VG = -1;
   unsigned char VB = 1;

   while(1) {
      if (clockState.on == 2) {
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
      }

      vTaskDelay(20 / portTICK_RATE_MS);
   }
}

void TextTask(void *args)
{
   setupLCD();
   
   while (1) {

      xSemaphoreTake(clockStateSem, portMAX_DELAY);

      if (clockState.on) {

         // Manually build large time string
         if (clockState.write0 == 0) {
            clockState.writebuff[0][0] = clockState.time.year/1000 + '0';
            clockState.writebuff[0][1] = (clockState.time.year/100)%10 + '0';
            clockState.writebuff[0][2] = (clockState.time.year/10)%10 + '0';
            clockState.writebuff[0][3] = (clockState.time.year)%10 + '0';
            clockState.writebuff[0][4] = '-';
            clockState.writebuff[0][5] = clockState.time.month/10 + '0';
            clockState.writebuff[0][6] = clockState.time.month%10 + '0';
            clockState.writebuff[0][7] = '-';
            clockState.writebuff[0][8] = clockState.time.day/10 + '0';
            clockState.writebuff[0][9] = clockState.time.day%10 + '0';
         }

         // Manually build small time string
         if (clockState.write1 == 0) {
            clockState.writebuff[1][0] = clockState.time.hour/10 + '0';
            clockState.writebuff[1][1] = clockState.time.hour%10 + '0';
            clockState.writebuff[1][2] = ':';
            clockState.writebuff[1][3] = clockState.time.min/10 + '0';
            clockState.writebuff[1][4] = clockState.time.min%10 + '0';
            clockState.writebuff[1][5] = ':';
            clockState.writebuff[1][6] = clockState.time.sec/10 + '0';
            clockState.writebuff[1][7] = clockState.time.sec%10 + '0';
            clockState.writebuff[1][8] = '\0';
         }

         clockState.writebuff[0][10] = '\0';
         clockState.writebuff[1][9] = '\0';

      } else {
         clockState.writebuff[0][0] = '\0';
         clockState.writebuff[1][0] = '\0';
      }

      xSemaphoreGive(clockStateSem);

      lcdprint(0, clockState.writebuff[0]);
      lcdprint(1, clockState.writebuff[1]);

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

void UARTTask(void *args) 
{
   while (1) {
      wifly_uart_rx_tx(&wf); // Poll the UART rx/tx buffer
      vTaskDelay(0); // Allow this task to yeild
   }
}

void WiflyTask(void *args)
{
   while (1) {
      wifly_check_state(&wf); // Increment the wifly state machine
      // Task delays are given within the wifly state check function
   }
}

void ReceiveTask(void *args)
{
   int cnt;
   char buff[128];

   while (1) {

      // Receive command type bytes
      if (3 == (cnt = wifly_receive(&wf, buff, 3))) {

         // Color change command
         if (!strncmp(buff, "COL", 3)) {
            if (6 == (cnt = wifly_receive(&wf, buff, 6))) {
               writeBytes("$COLOR=", 7, USART0);
               uint8_t newred = atoi8(&buff[0]);
               uint8_t newgreen = atoi8(&buff[2]);
               uint8_t newblue = atoi8(&buff[4]);

               // Easter egg! Awsome color change mode
               if (newred == 0x0A && newgreen == 0xCE && newblue == 0xD0) {
                  writeBytes("CoLoR ChAnGe mOdE!\r\n", 20, USART0);
                  xSemaphoreTake(clockStateSem, portMAX_DELAY);
                  clockState.on = 2;
                  xSemaphoreGive(clockStateSem);
               } else {
                  writeBytes(buff, cnt, USART0);
                  writeBytes("\r\n", 2, USART0);
                  xSemaphoreTake(clockStateSem, portMAX_DELAY);
                  clockState.on = 1;
                  xSemaphoreGive(clockStateSem);
                  lcd_setColorRGB(newred, newgreen, newblue);
               }
            }

         // Toggle display command
         } else if (!strncmp(buff, "TOG", 3)) {
            writeBytes("$TOGGLE\r\n", 9, USART0);
            if (clockState.on) {
               lcd_setColorRGB(0, 0, 0);
               lcdprint(0, "");
               lcdprint(1, "");
               xSemaphoreTake(clockStateSem, portMAX_DELAY);
               clockState.on = 0;
               xSemaphoreGive(clockStateSem);
            } else {
               lcd_setColorRGB(255, 255, 255);
               xSemaphoreTake(clockStateSem, portMAX_DELAY);
               clockState.on = 1;
               xSemaphoreGive(clockStateSem);
            }

         // Write display command
         } else if (!strncmp(buff, "WRT", 3)) {
            if (3 == (cnt = wifly_receive(&wf, buff, 3))) {
               writeBytes("$WRITE\r\n", 8, USART0);
               uint8_t line = ctoi(buff[0]);
               uint8_t bytes = atoi8(&buff[1]);
               if (bytes == (cnt = wifly_receive(&wf, buff, bytes))) {
                  writeBytes(buff, bytes, USART0);
                  buff[bytes] = '\0';
                  xSemaphoreTake(clockStateSem, portMAX_DELAY);
                  if (line == 0) {
                     strncpy(clockState.writebuff[line], buff, 17);
                     clockState.write0 = 1;
                  } else if (line == 1) {
                     strncpy(clockState.writebuff[line], buff, 17);
                     clockState.write1 = 1;
                  }
                  xSemaphoreGive(clockStateSem);
               }
            }

         // Clear display command
         } else if (!strncmp(buff, "CLR", 3)) {
            writeBytes("$CLEAR\r\n", 8, USART0);
            xSemaphoreTake(clockStateSem, portMAX_DELAY);
            clockState.write0 = 0;
            clockState.write1 = 0;
            xSemaphoreGive(clockStateSem);
         }
      }

      wifly_flush(&wf);
   }
}

int main(void)
{
   // Setup wifly and debug interface
   setupSerial(9600, USART0);
   setupSerial(9600, USART1);
   wifly_setup(&wf);

   // Initialize time to 1970 epoch
   clockState.time.year = 1970;
   clockState.time.month = 1;
   clockState.time.day = 1;
   clockState.time.hour = 0;
   clockState.time.min = 0;
   clockState.time.sec = 0;
   clockState.on = 1;
   clockState.write0 = 0;
   clockState.write1 = 0;

   vSemaphoreCreateBinary(clockStateSem);

   xTaskCreate(ColorTask, (cscp) "color", 1000, NULL, 4, NULL);
   xTaskCreate(TextTask, (cscp) "text", 1000, NULL, 5, NULL);
   xTaskCreate(ClockTask, (cscp) "clock", 1000, NULL, 6, NULL);

   xTaskCreate(UARTTask, (cscp) "uart", 100, NULL, 1, NULL);
   xTaskCreate(WiflyTask, (cscp) "wifly", 100, NULL, 2, NULL);
   xTaskCreate(ReceiveTask, (cscp) "receive", 300, NULL, 3, NULL);

   vTaskStartScheduler();

   return 0;
}
