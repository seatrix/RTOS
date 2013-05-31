#include <avr/io.h>
#include <util/delay.h>
#include "lcd.h"

int main(void)
{
   setupLCD();
   clearDisplay();
   writeRAM('d');
   writeRAM('e');
   writeRAM('a');
   writeRAM('d');
   writeRAM('b');
   writeRAM('e');
   writeRAM('e');
   writeRAM('f');


   return 0;
}
