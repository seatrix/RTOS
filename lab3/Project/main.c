/*
* TODO
*
*Authors: Matt Zimmerer, Daniel Jennings
*
*Version: Lab03 version 1.0
*/
#define F_CPU 16000000L

#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

#define SW7 (1 << 7)

#define LED2 (1 << 2)
#define LED6 (1 << 6)
#define LED7 (1 << 7)

void vISRHdlrTask(void *tArgs);
void vTIMHdlrTask(void *tArgs);
void vLEDTask(void *tArgs);

xSemaphoreHandle xISRSemaphore;
xSemaphoreHandle xTIMSemaphore;

// Active high buttonState mirror
static volatile unsigned char buttonState = 1;

static void init_isr()
{
   EICRA = 0x00; // INT0-6 not configured
   EICRB = 0x40; // INT7 falling edge triggered
   EIMSK = 0x80; // Enable INT7
}

static void init_timer()
{
   // TODO fix comments
   TCCR2A = 0xC0; //set timer2 to CTC mode, sets OC2A on compare match
   TCCR2B = 0x0F; //set prescaler to divide by 256 (frequency at 31250Hz)
   OCR2A = 0x01; //set the bottom 8 bits of output compare reg to 4
   TIMSK2 = 0x02; //enables interrupt on compare match A
}

int main(void)
{
   DDRB |= LED2 | LED6 | LED7;
   DDRE &= ~SW7;
   PORTB |= (LED2 | LED6);
   PORTB &= ~LED7;

   init_isr();
   init_timer();
   sei();

   vSemaphoreCreateBinary(xISRSemaphore);
   vSemaphoreCreateBinary(xTIMSemaphore);

   xTaskCreate(vTIMHdlrTask, (const signed char *) "TIMHDLR", 100, NULL, 3, NULL);
   xTaskCreate(vISRHdlrTask, (const signed char *) "ISRHDLR", 100, NULL, 2, NULL);
   xTaskCreate(vLEDTask, (const signed char *) "LED", 100, NULL, 1, NULL);

   // Kick off the scheduler
   vTaskStartScheduler();

   return 0;
}

void vISRHdlrTask(void *tArgs)
{
   for (;;) {
      xSemaphoreTake(xISRSemaphore, portMAX_DELAY);

      PORTB ^= LED6;
   }
}

void vTIMHdlrTask(void *tArgs)
{
   static volatile char toggle = 0;

   for (;;) {
      xSemaphoreTake(xTIMSemaphore, portMAX_DELAY);

      // If button is pressed
      if (buttonState == 0) {
         
         // Only toggle LED every other timer tick
         if (toggle ^= 0xFF) {
            PORTB ^= LED7;
         }

      // If button is not pressed
      } else {
         PORTB ^= LED7;
      }
   }
}

void vLEDTask(void *tArgs)
{
   for (;;) {
      
   }
}

ISR(INT7_vect)
{
   static portBASE_TYPE xHPTW = pdFALSE;

   // Debouncing logic
//   _delay_ms(50);
//   if ((buttonState == 1 && (PORTE & SW7)) || (buttonState == 0 && !(PORTE & SW7)))
//      return;

   buttonState ^= 1;

   xSemaphoreGiveFromISR(xISRSemaphore, &xHPTW);
}

ISR(TIMER2_COMPA_vect)
{
   static portBASE_TYPE xHPTW = pdFALSE;

   xSemaphoreGiveFromISR(xTIMSemaphore, &xHPTW);
}
