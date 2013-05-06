#include <avr/io.h>
#include "spi_sseg.h"

// Generic structure for LUT
typedef struct Command {
   uint8_t input;
   uint8_t command;
} Command;

// Single digit command LUT
Command digitCommands[] = {
   {1, SSEG_DIG1},
   {2, SSEG_DIG2},
   {3, SSEG_DIG3},
   {4, SSEG_DIG4},
   {0, 0}
};

// Single digit value LUT
Command valueCommands[] = {
   {0, SSEG_0},
   {1, SSEG_1},
   {2, SSEG_2},
   {3, SSEG_3},
   {4, SSEG_4},
   {5, SSEG_5},
   {6, SSEG_6},
   {7, SSEG_7},
   {8, SSEG_8},
   {9, SSEG_9},
   {10, SSEG_A},
   {11, SSEG_B},
   {12, SSEG_C},
   {13, SSEG_D},
   {14, SSEG_E},
   {15, SSEG_F},
   {0, 0}
};

// Single decimal command LUT
Command decimalCommands[] = {
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
   SPI_MasterTransmit(SSEG_BRIGHTNESS);

   //Brightness value between 0 and 254
   if (val > 254)
      val = 254;

   SPI_MasterTransmit(val);
}

void SSEG_Reset(void)
{
   //Special command for reset display
   SPI_MasterTransmit(SSEG_RESET);
}

void SSEG_Write_digit(uint8_t digit, uint8_t val)
{
   Command *dc;
   Command *vc;
   uint8_t command = 0;
   uint8_t value = SSEG_BLANK;   

   // Iterate through command LUT and find the correct command per digit
   for (dc = digitCommands; dc && (dc->input || dc->command); dc++)
      if (dc->input == digit)
         command = dc->command;

   // Iterate through value LUT and find the correct command per value
   for (vc = valueCommands; vc && (vc->input || vc->command); vc++)
      if (vc->input == val)
         value = vc->command;

   // If either is not found, forget about it!
   if (!command)
      return;

   SPI_MasterTransmit(command);
   SPI_MasterTransmit(value);
}
 
void SSEG_Write_4vals_array(uint8_t *vals)
{
   SSEG_Write_digit(DIGIT_1, vals[0]);
   SSEG_Write_digit(DIGIT_2, vals[1]);
   SSEG_Write_digit(DIGIT_3, vals[2]);
   SSEG_Write_digit(DIGIT_4, vals[3]);
}

void SSEG_Write_left_digits(uint8_t val)
{
   SSEG_Write_digit(DIGIT_1, val / 10);
   SSEG_Write_digit(DIGIT_2, val % 10);
}

void SSEG_Write_right_digits(uint8_t val)
{
   SSEG_Write_digit(DIGIT_3, val / 10);
   SSEG_Write_digit(DIGIT_4, val % 10);
}

void SSEG_Write_Decimal_Point(uint8_t val)
{
   Command *dc;

   // Iterate through decimal LUT and find the correct command per decimal
   for (dc = decimalCommands; dc && (dc->input || dc->command); dc++)
      if (dc->input == val)
         decimalPoints |= dc->command;

   SPI_MasterTransmit(SSEG_DEC_PNT);
   SPI_MasterTransmit(decimalPoints);
}

void SSEG_Clear_Decimal_Point(uint8_t val)
{
   Command *dc;

   // Iterate through decimal LUT and find the correct command per decimal
   for (dc = decimalCommands; dc && (dc->input || dc->command); dc++)
      if (dc->input == val)
         decimalPoints &= ~dc->command;

   SPI_MasterTransmit(SSEG_DEC_PNT);
   SPI_MasterTransmit(decimalPoints);
}
