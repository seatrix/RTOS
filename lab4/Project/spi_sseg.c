#include <avr/io.h>
#include "spi_sseg.h"

// Structure for single digit command LUT
typedef struct digitCommands {
   uint8_t digit;
   uint8_t command;
} DigitCommand;

// Single digit command LUT
DigitCommand digitCommands[] = {
   {1, 0x7B},
   {2, 0x7C},
   {3, 0x7D},
   {4, 0x7E},
   {0, 0}
};

// Structure for converting single digit values to SSEG commands
typedef struct valueCommand {
   uint8_t value;
   uint8_t command;
} ValueCommand;

// Single digit value LUT
ValueCommand valueCommands[] = {
   {0, 0x3F},
   {1, 0x06},
   {2, 0x5B},
   {3, 0x4F},
   {4, 0x66},
   {5, 0x6D},
   {6, 0x7D},
   {7, 0x07},
   {8, 0x7F},
   {9, 0x67},
   {0, 0}
};

// Structure for single decimal command LUT
typedef struct decimalCommands {
   uint8_t decimal;
   uint8_t command;
} DecimalCommand;

// Single decimal command LUT
DecimalCommand decimalCommands[] = {
   {0, SSEG_DP_0},
   {1, SSEG_DP_1},
   {2, SSEG_DP_2},
   {3, SSEG_DP_3},
   {4, SSEG_DP_4},
   {5, SSEG_DP_5},
   {0, 0}
};

static uint8_t decimalPoints;

void SPI_MasterInit(void)
{
   // Set all required pins as outputs, no inputs
   SPI_DDR = (1<<SPI_SS) | (1<<SPI_MOSI) | (1<<SPI_SCK);

   // Enable spi, set master mode, and sck = fosc/64
   SPCR = (1<<SPE) | (1<<MSTR) | (1<<SPR1);
} 

void SPI_MasterTransmit(uint8_t data)
{
   SPDR = data;
   while(!(SPSR & (1<<SPIF)));
}
 
void SSEG_Set_Brightness(uint8_t val)
{
   //Special command for brightness
   SPI_MasterTransmit(0x7A);

   //Brightness value between 0 and 254
   if (val > 254)
      val = 254;

   SPI_MasterTransmit(val);
}

void SSEG_Reset(void)
{
   //Special command for reset display
   SPI_MasterTransmit(0x76);
}

void SSEG_Write_digit(uint8_t digit, uint8_t val)
{
   DigitCommand *dc;
   ValueCommand *vc;
   uint8_t command = 0;
   uint8_t value = 0;   

   // Iterate through command LUT and find the correct command per digit
   for (dc = digitCommands; dc && (dc->digit || dc->command); dc++)
      if (dc->digit == digit)
         command = dc->command;

   // Iterate through value LUT and find the correct command per value
   for (vc = valueCommands; vc && (vc->value || vc->command); vc++)
      if (vc->value == val)
         value = vc->command;

   // If either is not found, forget about it!
   if (!command || !value)
      return;

   SPI_MasterTransmit(command);
   SPI_MasterTransmit(value);
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
   SSEG_Write_digit(1, val/10);
   SSEG_Write_digit(2, val%10);
}

void SSEG_Write_right_digits(uint8_t val)
{
   SSEG_Write_digit(3, val/10);
   SSEG_Write_digit(4, val%10);
}

void SSEG_Write_Decimal_Point(uint8_t val)
{
   DecimalCommand *dc;

   // Iterate through decimal LUT and find the correct command per decimal
   for (dc = decimalCommands; dc && (dc->decimal || dc->command); dc++)
      if (dc->decimal == val)
         decimalPoints |= dc->command;

   SPI_MasterTransmit(SSEG_DEC_PNT);
   SPI_MasterTransmit(decimalPoints);
}

void SSEG_Clear_Decimal_Point(uint8_t val)
{
   DecimalCommand *dc;

   // Iterate through decimal LUT and find the correct command per decimal
   for (dc = decimalCommands; dc && (dc->decimal || dc->command); dc++)
      if (dc->decimal == val)
         decimalPoints &= ~dc->command;

   SPI_MasterTransmit(SSEG_DEC_PNT);
   SPI_MasterTransmit(decimalPoints);
}
