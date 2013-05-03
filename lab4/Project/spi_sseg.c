#include <avr/io.h>
#include "spi_sseg.h"

#define DDR_SPI
#define DD_MOSI
#define DD_SCK

void SPI_MasterInit(void)
{      
   /* Set MOSI and SCK output, all others input */
   DDRB = (1<<SPI_SS) | (1<<SPI_MOSI) | (1<<SPI_SCK);
   /* Enable SPI, Master, set clock rate fck/16 */
   SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0);
} 

void SPI_MasterTransmit(uint8_t data)
{
   /* Start transmission */
   SPDR = data;
   /* Wait for transmission complete */
   while(!(SPSR & (1<<SPIF)));
}
 
void SSEG_Set_Brightness(uint8_t val)
{
   //Special command for brightness
   SPI_MasterTransmit(0x7E);
   //Brightness value between 0 and 256
   SPI_MasterTransmit(0x00);
}

void SSEG_Reset(void)
{
   //Special command for reset display
   SPI_MasterTransmit(0x76); 
}

void SSEG_Write_digit(uint8_t digit, uint8_t val)
{
   uint8_t dig;
   switch(digit) {

      case 1: 
         dig = 0x7B;
         break;

      case 2: 
         dig = 0x7C;
         break;

      case 3: 
         dig = 0x7D;
         break;

      case 4: 
         dig = 0x7E;
         break;

      default:
         break;
   }

   SPI_MasterTransmit(dig);
   SPI_MasterTransmit(val);      
} 
 
void SSEG_Write_4vals_array(uint8_t *vals)
{
   SSEG_Write_digit(1, vals[0]);
   SSEG_Write_digit(2, vals[1]);
   SSEG_Write_digit(3, vals[2]);
   SSEG_Write_digit(4, vals[3]);
}

void SSEG_Write_left_digits(uint8_t val)
{
   SSEG_Write_digit(1, vals[0]/10);
   SSEG_Write_digit(2, vals[1]%10);
}

void SSEG_Write_right_digits(uint8_t val)
{
   SSEG_Write_digit(3, vals[0]/10);
   SSEG_Write_digit(4, vals[1]%10);
}

void SSEG_Write_Decimal_Point(uint8_t val)
{

}
