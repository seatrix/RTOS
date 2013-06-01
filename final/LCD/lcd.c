/*
 * @file lcd.c
 * @brief general Hitachi HD44780 interface header file
 * @author Daniel Jennings
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include "lcd.h"

static void __setupTimer0()
{
   TCCR0A = 0xC2; //set timer0 to CTC mode, sets OC0A on compare match
   TCCR0B = 0x04; //set prescaler to divide by 256 (frequency at 31250Hz)
   OCR0A  = 0x1; //output compare match register set frequency to 15625Hz
   TIMSK0 = 0x02; //enables interrupt on compare match A
}

void __writeLCD(uint8_t data)
{
   //write data to the data port
   LCD_DAT_PORT = data; 

   //set RW
   LCD_CTRL_PORT &= ~E_MASK;
   _delay_ms(1);
   LCD_CTRL_PORT |= E_MASK;
}

static void __writeString(char *string)
{
   while (*string != '\0')
      writeRAM(*string++);
}

/*
 * @brief sets up the interface to LCD, and enters initial commands
 */
void setupLCD()
{
   LCD_CRTL_DDR |= 0x03;
   LCD_DAT_DDR |= 0xFF;
   LCD_CRTL_DDR |= R_MASK | G_MASK | B_MASK;
   LCD_CTRL_PORT |= R_MASK | G_MASK | B_MASK;

   //set RW and RS and E to 0
   LCD_CTRL_PORT &= ~RS_MASK; 
   LCD_CTRL_PORT |= E_MASK;   

   //give it time before turning on
   _delay_ms(200);
   
   functionSet(1,1,0);
   entryModeSet(1,0);
   clearDisplay();
   displayOnOff(1,1,1);

   // Initialize screen color ISR
   RGB[RED] = 100;
   RGB[GREEN] = 0;
   RGB[BLUE] = 255;
   __setupTimer0();
   sei();
}

/*
 * @brief commands the LCD to clear display
 */
void clearDisplay()
{
   LCD_CTRL_PORT &= ~RS_MASK; 

   //clear display
   __writeLCD(0x01);

   _delay_ms(2); 
}

/*
 * @brief commands the LCD to move curser to home
 */
void cursorHome()
{
   LCD_CTRL_PORT &= ~RS_MASK; 

   //set cursor to first spot
   __writeLCD(0x02);

   _delay_ms(2); 
}

/*
 * @brief entry mode set command.
 *
 * @param incCursor 1 to increment cursor on write, 0 to de-increment
 * @param shiftDisp 1 to shift display on write, 0 to not shift
 */
void entryModeSet(uint8_t incCursor, uint8_t shiftDisp)
{
   LCD_CTRL_PORT &= ~RS_MASK; 

   //automatically just sends command for entrymode set
   uint8_t entryMode = 0x04;
   
   //or with a mask of 00000010 to set cursor increment on
   if(incCursor) 
      entryMode = (entryMode | 0x02);

   //or with mask of 00000001 to set shift on
   if(shiftDisp)
      entryMode = (entryMode | 0x01);
   
   __writeLCD(entryMode);

   _delay_ms(1);    
}

/*
 * @brief display on off command.
 *
 * @param dispOn 1 for display on, 0 for off
 * @param cursOn 1 for curso on, 0 for off
 * @param cursBlink 1 for cursor blinkage, 0 for off
 */
void displayOnOff(uint8_t dispOn, uint8_t cursOn, uint8_t cursBlink)
{

   uint8_t dispOnOff = 0x08;

   LCD_CTRL_PORT &= ~RS_MASK; 

   if(dispOn)
      dispOnOff = (dispOnOff | 0x04);

   if(cursOn)
      dispOnOff = (dispOnOff | 0x02);

   if(cursBlink)
      dispOnOff = (dispOnOff | 0x01);

   __writeLCD(dispOnOff); 
   
   _delay_ms(1); 
}

/*
 * @brief cursor display shift command. This command acts immediatly.
 *
 * @param shiftDisp 1 to shift display, 0 to shift cursor
 * @param shiftRight 1 to shift right, 0 to shift left
 */
void cursorDisplayShift(uint8_t shiftDisp, uint8_t shiftRight)
{
   LCD_CTRL_PORT &= ~RS_MASK; 

   uint8_t cursorDispShift = 0x10;

   if(shiftDisp)
      cursorDispShift = (cursorDispShift | 0x08);

   if(shiftRight)
      cursorDispShift = (cursorDispShift | 0x04);
   
   __writeLCD(cursorDispShift);

   _delay_ms(1); 
}

/*
 * @brief functionSet command.
 *
 * @param bytemode 1 for 8bit data transfers, 0 for nibble mode (4bit)
 * @param twolines 1 for twoline screen, 0 for 1 line screen
 * @param font 1 for 5x10 character set, 0 for 5x8 character set
 */
void functionSet(uint8_t bytemode, uint8_t twolines, uint8_t font)
{
   LCD_CTRL_PORT &= ~RS_MASK; 
   
   uint8_t function = 0x20;

   if (bytemode)
      function |= 0x10;

   if (twolines)
      function |= 0x08;

   if (font)
      function |= 0x04;
   
   __writeLCD(function);
      
   _delay_ms(1); 
}

/*
 * @brief set the address into the CGRAM memory section.
 *
 * @param address the address to ser CGRAM to
 */
void setCGRAMaddr(uint8_t address)
{
   LCD_CTRL_PORT &= ~RS_MASK;

   __writeLCD(0x40 | address);

   _delay_ms(1); 
}

/*
 * @brief set the address into the DDRAM memory section.
 *
 * @param address the address to set DDRAM to
 */
void setDDRAMaddr(uint8_t address)
{
   LCD_CTRL_PORT &= ~RS_MASK; 

   __writeLCD(0x80 | address);

   _delay_ms(1); 
}

/*
 * @brief writes to ram, the last ram region set is the ram region that will be
 * written to.
 *
 * @param data data to write to ram
 */
void writeRAM(uint8_t data)
{
   LCD_CTRL_PORT |= RS_MASK; 

   __writeLCD(data);

   _delay_ms(1); 
}

/*
 * @brief lcd printf function. Prints a variadic format string to LCD.
 *
 * @param line 0 to write on first line, 1 to write on second line
 * @param fmt a format string similar to printf's format string
 * @param ... variadic arguments containing values for format string
 */
void lcdprintf(uint8_t line, const char *fmt, ...)
{
   char tmp[32]; //limits length of string

   // Move cursor to beginning of selected line
   if (line == 0)
      setDDRAMaddr(0x00);
   else
      setDDRAMaddr(0x40);

   va_list args;
   va_start(args, fmt);
   vsnprintf(tmp, 32, fmt, args);
   va_end(args);

   __writeString(tmp);
}

/*
 * @brief changes the color of the LCD to the 8-bit RGB value given.
 *
 * @param red the amount of red (8bit). 0 = none, 255 = all.
 * @param green the amount of green (8bit). 0 = none, 255 = all.
 * @param blue the amount of blue (8bit). 0 = none, 255 = all.
 */
void lcd_setColor(uint8_t red, uint8_t green, uint8_t blue)
{
   // TODO
}
