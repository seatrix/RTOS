#include <avr/io.h>
#include <util/delay.h>
#include "lcd.h"

int main(void)
{
   setupLCD();

   lcdprintf(0, "blah %i", 123);
   lcdprintf(1, "mooo %x", 123);

   while(1);

   return 0;
}
