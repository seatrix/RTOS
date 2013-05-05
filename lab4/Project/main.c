/*
*Project: Lab3 Semaphores
*
*
*Authors: Matt Zimmerer, Daniel Jennings
*
*Version: Lab03 version 1.0
*/
#define F_CPU 8000000L

#include <stdint.h>
#include <avr/io.h>
//#include <avr/interrupt.h>
#include <util/delay.h>
//#include "FreeRTOS.h"
//#include "semphr.h"
//#include "task.h"
#include "spi_sseg.h"

//void Task(void *tArgs);
/*
static void init_isr()
{
   EICRA = 0x00; // INT0-6 not configured
   EICRB = 0x40; // INT7 falling edge triggered
   EIMSK = 0x80; // Enable INT7
}

static void init_timer()
{
   TCCR2A = 0xC0; // Set timer2 to CTC mode, sets OC2A on compare match
   TCCR2B = 0x0F; // Set prescaler to divide by 1024
   OCR2A = 162;   // Set output compare reg to 162
   TIMSK2 = 0x02; // Enables interrupt on compare match A
}
*/
int main(void)
{

   //init_isr()initializing ISR
   //init_timer();//initializing timer
   //sei();//enabling interrupts

   uint8_t brightness = 0;

   // Initialize SSEG display
   SPI_MasterInit();
   SSEG_Set_Brightness(0);
   SSEG_Reset();

   DDRF=0xFF;
   while (1) {
      SSEG_Reset();
      _delay_ms(1000);
      PORTF=~brightness;
      brightness++;
      if (brightness >= 110)
         brightness = 0;
      SSEG_Set_Brightness(brightness);
      SSEG_Reset();
      SSEG_Write_digit(1, 1); 
      SSEG_Write_digit(2, 2); 
      SSEG_Write_digit(3, 3); 
      SSEG_Write_digit(4, 4);
      _delay_ms(1000);
      SSEG_Write_digit(1, 5); 
      SSEG_Write_digit(2, 6); 
      SSEG_Write_digit(3, 7); 
      SSEG_Write_digit(4, 8);
      _delay_ms(1000);
      SSEG_Write_digit(1, 0); 
      SSEG_Write_digit(2, 0); 
      SSEG_Write_digit(3, 9); 
      SSEG_Write_digit(4, 9);
      _delay_ms(1000);

      SSEG_Write_left_digits(10); 
      _delay_ms(250);
      SSEG_Write_right_digits(20); 
      _delay_ms(250);

      int i;
      for (i=0;i<=6;i++) {
         SSEG_Write_Decimal_Point(i);
         _delay_ms(250);
      }

      for (i=0;i<=6;i++)
         SSEG_Clear_Decimal_Point(i);
   }

   //xTaskCreate(Task, (const signed char *) "TIMHDLR", 100, NULL, 3, NULL);

   // Kick off the scheduler
   //vTaskStartScheduler();

   return 0;
}
/*
void Task(void *tArgs)
{
   for (;;) {
   }
}

ISR(INT7_vect)
{
}

ISR(TIMER2_COMPA_vect)
{
}
*/
