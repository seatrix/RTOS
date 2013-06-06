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

// Definitions for event system
#define EVENT_CLEAR    0
#define EVENT_TOGGLE   1
#define EVENT_WRITE    2
#define EVENT_COLOR    3
#define COLOR_REDNDX   0
#define COLOR_GREENNDX 1
#define COLOR_BLUENDX  2
#define DATSIZE        16

typedef const signed char * cscp;

struct Time {
   uint16_t year;
   uint8_t month;
   uint8_t day;
   uint8_t hour;
   uint8_t min;
   uint8_t sec;
};

struct ScheduledEvent {
   struct Time time;       // Time to execute event at
   uint8_t eventType;      // Type of event
   uint8_t line;           // [write events] line to write to
   uint8_t bytes;          // [write events] number of bytes to write
   char eventdat[DATSIZE]; // [write] bytes to write, [color] colors
   struct ScheduledEvent *next; // Pointer to next node
};

struct ClockState {
   struct Time time;
   uint8_t on;       // Is the display on?
   uint8_t write0;   // Are we writing to line 0?
   uint8_t write1;   // Are we writing to line 1?
   char writebuff[2][17];
   struct ScheduledEvent *evtList;
};

static uint8_t daysInMonth[13] = {255,31,255,31,30,31,30,31,31,30,31,30,31};
static xSemaphoreHandle clockStateSem;
struct ClockState clockState;
struct wifly wf;

// Change the LCD's color, protected by semaphore
static void changeColor(uint8_t red, uint8_t green, uint8_t blue)
{
   // Easter egg! Awsome color change mode
   if (red == 0x0A && green == 0xCE && blue == 0xD0) {
      xSemaphoreTake(clockStateSem, portMAX_DELAY);
      writeBytes("AMERICA\r\n", 9, USART0);
      clockState.on = 2;
      xSemaphoreGive(clockStateSem);
   } else {
      xSemaphoreTake(clockStateSem, portMAX_DELAY);
      clockState.on = 1;
      xSemaphoreGive(clockStateSem);
      lcd_setColorRGB(red, green, blue);
   }
}

// Toggles the LCD's state between ON/OFF, protected by semaphore
static void toggleONOFF()
{
   xSemaphoreTake(clockStateSem, portMAX_DELAY);
   if (clockState.on) {
      lcd_setColorRGB(0, 0, 0);
      lcdprint(0, "");
      lcdprint(1, "");
      clockState.on = 0;
   } else {
      lcd_setColorRGB(255, 255, 255);
      clockState.on = 1;
   }
   xSemaphoreGive(clockStateSem);
}

// Manual write to LCD line, protected by semaphore. Alters contents of src.
static void writeDisplay(uint8_t line, char *src, uint8_t bytes)
{
   src[bytes] = '\0';
   xSemaphoreTake(clockStateSem, portMAX_DELAY);
   if (line == 0) {
      strncpy(clockState.writebuff[line], src, 17);
      clockState.write0 = 1;
   } else if (line == 1) {
      strncpy(clockState.writebuff[line], src, 17);
      clockState.write1 = 1;
   }
   xSemaphoreGive(clockStateSem);
}

// Clears any text manually written to display, protected by semaphore
static void clearWrites()
{
   xSemaphoreTake(clockStateSem, portMAX_DELAY);
   clockState.write0 = 0;
   clockState.write1 = 0;
   xSemaphoreGive(clockStateSem);
}

// Sets the system clock,  protectected by semaphore
static void setTime(uint16_t year, uint8_t month, uint8_t day,
   uint8_t hour, uint8_t min, uint8_t second) {
   xSemaphoreTake(clockStateSem, portMAX_DELAY);
   clockState.time.sec = second;
   clockState.time.min = min;
   clockState.time.hour = hour;
   clockState.time.day = day;
   clockState.time.month = month;
   clockState.time.year = year;
   xSemaphoreGive(clockStateSem);
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
static uint8_t atoiX8(char *src)
{
   uint8_t ans = 0;

   ans += ctoi(src[1]);
   ans += 16*ctoi(src[0]);

   return ans;
}

// Convert two digit decimal value to uint8_t
static uint8_t atoiD8(char *src)
{
   uint8_t ans = 0;

   ans += ctoi(src[1]);
   ans += 10*ctoi(src[0]);

   return ans;
}

// Convert two digit decimal value to uint8_t
static uint16_t atoiD16(char *src)
{
   uint16_t ans = 0;



   ans += ctoi(src[3]);
   ans += 10*ctoi(src[2]);
   ans += 10*10*ctoi(src[1]);
   ans += 10*10*10*ctoi(src[0]);

   return ans;
}

// Time argument is in seconds since epoch
static void addEvent(struct Time *time, uint8_t type, char *eventdat,
                        uint8_t line, uint8_t bytes)
{
   struct ScheduledEvent *newEvent;

   newEvent = (struct ScheduledEvent *) pvPortMalloc(
                                                 sizeof(struct ScheduledEvent));

   newEvent->time.sec = time->sec;
   newEvent->time.min = time->min;
   newEvent->time.hour = time->hour;
   newEvent->time.day = time->day;
   newEvent->time.month = time->month;
   newEvent->time.year = time->year;
   newEvent->eventType = type;
   newEvent->line = line;
   newEvent->bytes = bytes;
   memcpy(newEvent->eventdat, eventdat, bytes);

   // Insert into list, no sorting needed
   xSemaphoreTake(clockStateSem, portMAX_DELAY);
   newEvent->next = clockState.evtList;
   clockState.evtList = newEvent;
   xSemaphoreGive(clockStateSem);
}

// Check a specific event to see if it should occur, if so, return 1, else 0
static int checkEvent(struct ScheduledEvent *evt)
{
   if (evt->time.year <= clockState.time.year
         && evt->time.month <= clockState.time.month
         && evt->time.day <= clockState.time.day
         && evt->time.hour <= clockState.time.hour
         && evt->time.min <= clockState.time.min
         && evt->time.sec <= clockState.time.sec)
      return 1;

   return 0;
}

// Check for any events, if there is event, handle it
static void checkEvents()
{
   struct ScheduledEvent *iter, *prev;

   prev = NULL;
   for (iter = clockState.evtList; iter; iter = iter->next) {
      if (checkEvent(iter)) {
         writeBytes("scheduled event!\r\n", 18, USART0);

         xSemaphoreGive(clockStateSem);
         switch (iter->eventType) {
            case EVENT_CLEAR:
               writeBytes("SCLR\r\n", 6, USART0);
               clearWrites();
               break;

            case EVENT_TOGGLE:
               writeBytes("STOG\r\n", 6, USART0);
               toggleONOFF();
               break;

            case EVENT_WRITE:
               writeBytes("SWRT\r\n", 6, USART0);
               writeDisplay(iter->line, iter->eventdat, iter->bytes);
               break;

            case EVENT_COLOR:
               writeBytes("SCOL\r\n", 6, USART0);
               changeColor(atoiX8(&iter->eventdat[0]),
                              atoiX8(&iter->eventdat[2]),
                              atoiX8(&iter->eventdat[4]));
               break;

            default:
               break;
         }
         xSemaphoreTake(clockStateSem, portMAX_DELAY);

         // Special case where event is head of list
         if (prev == NULL)
            clockState.evtList = clockState.evtList->next;
         else
            prev->next = iter->next;

         vPortFree(iter);
      }

      prev = iter;
   }
}

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

// Update the clock state by cascading rollovers on all time fields
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

// Task support for 'Awsome Color Mode Easter Egg'
void ColorTask(void *args)
{
   uint8_t red = 255;
   uint8_t green = 0;
   uint8_t blue = 0;
   uint8_t VR = 1;
   uint8_t VG = -1;
   uint8_t VB = 1;

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

// LCD hardware interface task
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

// Seconds clock tick task
void ClockTask(void *args)
{
   portTickType lastWakeTime;
	lastWakeTime = xTaskGetTickCount();

   while(1) {
      xSemaphoreTake(clockStateSem, portMAX_DELAY);
      clockState.time.sec += 1;
      updateClock();
      checkEvents();
      xSemaphoreGive(clockStateSem);
      vTaskDelayUntil(&lastWakeTime, 1000 / portTICK_RATE_MS);
   }
}

// UART rx/tx poll task
void UARTTask(void *args) 
{
   while (1) {
      wifly_uart_rx_tx(&wf); // Poll the UART rx/tx buffer
      vTaskDelay(0); // Allow this task to yeild
   }
}

// WIFLY configuration task. Prepares WIFLY for UDP communication
void WiflyTask(void *args)
{
   while (1) {
      wifly_check_state(&wf); // Increment the wifly state machine
      // Task delays are given within the wifly state check function
   }
}

// Packet receive/handler task
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
               writeBytes("$COL", 4, USART0);
               writeBytes(buff, cnt, USART0);
               writeBytes("\r\n", 2, USART0);
               changeColor(atoiX8(&buff[0]), atoiX8(&buff[2]), atoiX8(&buff[4]));
            }

         // Toggle display command
         } else if (!strncmp(buff, "TOG", 3)) {
            writeBytes("$TOG\r\n", 9, USART0);
            toggleONOFF();

         // Write display command
         } else if (!strncmp(buff, "WRT", 3)) {
            if (3 == (cnt = wifly_receive(&wf, buff, 3))) {
               writeBytes("$WRT", 4, USART0);
               writeBytes(buff, 3, USART0);
               writeBytes("\r\n", 2, USART0);

               uint8_t line = ctoi(buff[0]);
               uint8_t bytes = atoiX8(&buff[1]);


               if (bytes == (cnt = wifly_receive(&wf, buff, bytes))) {
                  writeBytes(buff, bytes, USART0);
                  writeDisplay(line, buff, bytes);
               }
            }

         // Clear display command
         } else if (!strncmp(buff, "CLR", 3)) {
            writeBytes("$CLR\r\n", 6, USART0);
            clearWrites();

         // Change time command
         } else if (!strncmp(buff, "TIM", 3)) {
            if (14 == (cnt = wifly_receive(&wf, buff, 14))) {
               writeBytes("$TIM", 4, USART0);
               writeBytes(buff, 13, USART0);
               writeBytes("\r\n", 2, USART0);

               uint16_t year = atoiD16(&buff[0]);
               uint8_t month = atoiX8(&buff[4]);
               uint8_t day = atoiX8(&buff[6]);
               uint8_t hour = atoiX8(&buff[8]);
               uint8_t min = atoiX8(&buff[10]);
               uint8_t second = atoiX8(&buff[12]);

               setTime(year, month, day, hour, min, second);
            }

         } else if (!strncmp(buff, "SCH", 3)) {
            if (17 == (cnt = wifly_receive(&wf, buff, 17))) {
               writeBytes("$SCH", 4, USART0);
               writeBytes(buff, 17, USART0);
               writeBytes("\r\n", 2, USART0);

               struct Time time;
               time.year = atoiD16(&buff[0]);
               time.month = atoiD8(&buff[4]);
               time.day = atoiD8(&buff[6]);
               time.hour = atoiD8(&buff[8]);
               time.min = atoiD8(&buff[10]);
               time.sec = atoiD8(&buff[12]);
               uint8_t type = ctoi(buff[14]);
               uint8_t line = ctoi(buff[15]);
               uint8_t bytes = ctoi(buff[16]);

               if (bytes == (cnt = wifly_receive(&wf, buff, bytes)))
                  addEvent(&time, type, buff, line, bytes);
            }

         // XXX TEST
         } else if (!strncmp(buff, "TS1", 3)) {
            writeBytes("$TESTY\r\n", 8, USART0);
            struct Time t1;
            t1.sec = 23;
            t1.min = 0;
            t1.hour = 0;
            t1.day = 1;
            t1.month = 1;
            t1.year = 1970;
            char color[6] = "FF0000";
            addEvent(&t1, EVENT_COLOR, color, 0, 6);
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

   xTaskCreate(ClockTask, (cscp) "clock", 1000, NULL, 6, NULL);
   xTaskCreate(TextTask, (cscp) "text", 1000, NULL, 5, NULL);
   //xTaskCreate(ColorTask, (cscp) "color", 1000, NULL, 4, NULL);
   xTaskCreate(ReceiveTask, (cscp) "receive", 400, NULL, 3, NULL);
   xTaskCreate(WiflyTask, (cscp) "wifly", 100, NULL, 2, NULL);
   xTaskCreate(UARTTask, (cscp) "uart", 100, NULL, 1, NULL);

   vTaskStartScheduler();

   return 0;
}
