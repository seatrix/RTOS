#include <avr/io.h>
#include <util/delay.h>
#include "serial.h"

int main(void)
{
   char usart0IN;

   setupSerial(9600, USART0); // Debug serial port
   setupSerial(9600, USART1); // Wifly serial port

   while (1) {

      // Forward debug port data into wifly
      if (!readByte_nonblocking(&usart0IN, USART0)) {
         writeByte_blocking(usart0IN, USART1);
         writeByte_blocking(usart0IN, USART0);
      }
      if (!readByte_nonblocking(&usart0IN, USART1)) {
         writeByte_blocking(usart0IN, USART0);
      }
   }

   return 0;
}
