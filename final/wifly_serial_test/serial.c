/*
 * @file serial.c
 * @breif serial implementation for avr microcontroller. This file implements
 *        USART0, USART1, USART2, and USART3 with 8 bits, no parity, 1 stop bit
 *        configuration with a user supplied baudrate.
 * @author Matt Zimmerer
 */

#ifndef F_CPU
#error F_CPU must be defined to compile serial.c
#endif

#define __DELAY_BACKWARD_COMPATIBLE__ 
#include <avr/io.h>
#include <util/delay.h>
#include "serial.h"

#define NUMATTEMPTS 3000

static char __readByte(uint8_t usartn)
{
   switch (usartn) {
      case USART0: return UDR0;
      case USART1: return UDR1;
      case USART2: return UDR2;
      case USART3: return UDR3;
      default: return -1;
   }
}

static void __writeByte(char byte, uint8_t usartn)
{
   switch (usartn) {
      case USART0:
         UDR0 = byte;
         break;
      case USART1:
         UDR1 = byte;
         break;
      case USART2:
         UDR2 = byte;
         break;
      case USART3:
         UDR3 = byte;
         break;
      default:
         break;
   }
}

void setupSerial(uint32_t baudrate, uint8_t usartn)
{
   baudrate = (F_CPU / (16UL * baudrate)) - 1;

   switch (usartn) {
      case USART0:
         UBRR0H = (char) baudrate >> 8; // set baud rate
         UBRR0L = (char) baudrate;
         UCSR0A &= ~(U2X0 | MPCM0); // 1x speed, disable multi mcu mode
         UCSR0B = (1 << RXEN0) | (1 << TXEN0); // enable rx and tx
         UCSR0C = (1 << UCSZ00) // 8 data bits
                | (1 << UCSZ01) // 8 data bits
                | (0 << UPM00)  // no parity generation
                | (0 << USBS0); // 1 stop bit
         break;

      case USART1:
         UBRR1H = (char) baudrate >> 8; // set baud rate
         UBRR1L = (char) baudrate;
         UCSR1A &= ~(U2X1 | MPCM1); // 1x speed, disable multi mcu mode
         UCSR1B = (1 << RXEN1) | (1 << TXEN1); // enable rx and tx
         UCSR1C = (1 << UCSZ10) // 8 data bits
                | (1 << UCSZ11) // 8 data bits
                | (0 << UPM10)  // no parity generation
                | (0 << USBS1); // 1 stop bit
         break;

      case USART2:
         UBRR2H = (char) baudrate >> 8; // set baud rate
         UBRR2L = (char) baudrate;
         UCSR2A &= ~(U2X2 | MPCM2); // 1x speed, disable multi mcu mode
         UCSR2B = (1 << RXEN2) | (1 << TXEN2); // enable rx and tx
         UCSR2C = (1 << UCSZ20) // 8 data bits
                | (1 << UCSZ21) // 8 data bits
                | (0 << UPM20)  // no parity generation
                | (0 << USBS2); // 1 stop bit
         break;

      case USART3:
         UBRR3H = (char) baudrate >> 8; // set baud rate
         UBRR3L = (char) baudrate;
         UCSR3A &= ~(U2X3 | MPCM3); // 1x speed, disable multi mcu mode
         UCSR3B = (1 << RXEN3) | (1 << TXEN3); // enable rx and tx
         UCSR3C = (1 << UCSZ30) // 8 data bits
                | (1 << UCSZ31) // 8 data bits
                | (0 << UPM30)  // no parity generation
                | (0 << USBS3); // 1 stop bit
         break;

      default:
         break;
   }
}

int canRead(uint8_t usartn)
{
   switch (usartn) {
      case USART0: return UCSR0A & (1 << RXC0);
      case USART1: return UCSR1A & (1 << RXC1);
      case USART2: return UCSR2A & (1 << RXC2);
      case USART3: return UCSR3A & (1 << RXC3);
      default: return 0;
   }
}

int canWrite(uint8_t usartn)
{
   switch (usartn) {
      case USART0: return UCSR0A & (1 << UDRE0);
      case USART1: return UCSR1A & (1 << UDRE1);
      case USART2: return UCSR2A & (1 << UDRE2);
      case USART3: return UCSR3A & (1 << UDRE3);
      default: return 0;
   }
}

int readByte_nonblocking(char *dst, uint8_t usartn)
{
   if (canRead(usartn)) {
      *dst = __readByte(usartn);
      return 0;
   }

   return -1;
}

int writeByte_nonblocking(char byte, uint8_t usartn)
{
   if (canWrite(usartn)) {
      __writeByte(byte, usartn);
      return 0;
   }

   return -1;
}

void readByte_blocking(char *dst, uint8_t usartn)
{
   while (readByte_nonblocking(dst, usartn));
}

void writeByte_blocking(char byte, uint8_t usartn)
{
   while (writeByte_nonblocking(byte, usartn));
}

int readBytes(char *dst, uint16_t bytes, uint8_t usartn)
{
   uint16_t bytesRead = 0;
   uint16_t attempts = 0;

   while (bytes != 0 && ++attempts <= NUMATTEMPTS) {
      if (!readByte_nonblocking(&dst[bytesRead], usartn)) {
         attempts = 0;
         bytes--;
         bytesRead++;
      }
   }

   return bytesRead;
}

int writeBytes(char *src, uint16_t bytes, uint8_t usartn)
{
   uint16_t bytesWritten = 0;
   uint16_t attempts = 0;

   while (bytes != 0 && ++attempts <= NUMATTEMPTS) {
      if (!writeByte_nonblocking(src[bytesWritten], usartn)) {
         attempts = 0;
         bytes--;
         bytesWritten++;
      }
   }

   return bytesWritten;
}

void flushSerial(uint8_t usartn)
{
   volatile char null;

   while (canRead(usartn))
      null = __readByte(usartn);
}
