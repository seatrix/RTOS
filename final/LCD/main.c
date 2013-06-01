#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "lcd.h"

int main(void)
{
   setupLCD();

   lcdprintf(0, "blah %i", 123);
   lcdprintf(1, "mooo %x", 123);

   unsigned char red = 100;
   unsigned char green = 255;
   unsigned char blue = 0;
   unsigned char VR = 1;
   unsigned char VG = -1;
   unsigned char VB = 1;

   while(1) {
      _delay_ms(10);
      if (red == 255) {
         VR = -1;
      }
      if (red == 0) {
         VR = 1;
      }
      if (green == 255) {
         VG = -1;
      }
      if (green == 0) {
         VG = 1;
      }
      if (blue == 255) {
         VB = -1;
      }
      if (blue == 0) {
         VB = 1;
      }

      red += VR;
      green += VG;
      blue += VB;
      RGB[RED] = red;
      RGB[GREEN] = green;
      RGB[BLUE] = blue;
   }

   return 0;
}

ISR(TIMER0_COMPA_vect)
{
   static volatile uint8_t counter = 0;

   if (counter == 0) {
      if (RGB[RED] > 0)
         LCD_CTRL_PORT &= ~R_MASK;
      if (RGB[GREEN] > 0)
         LCD_CTRL_PORT &= ~G_MASK;
      if (RGB[BLUE] > 0)
         LCD_CTRL_PORT &= ~B_MASK;
   }

   if (RGB[RED] == counter)
      LCD_CTRL_PORT |= R_MASK;
   if (RGB[GREEN] == counter)
      LCD_CTRL_PORT |= G_MASK;
   if (RGB[BLUE] == counter)
      LCD_CTRL_PORT |= B_MASK;

   counter++;
}























