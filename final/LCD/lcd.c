/*
 * @file lcd.c
 * @brief general Hitachi HD44780 interface header file
 * @author Daniel Jennings
 */

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include "lcd.h"

/*
 * @brief sets up the interface to LCD, and enters initial commands
 */
void setupLCD()
{
   LCD_CRTL_DDR = 0x03;
   LCD_DAT_DDR = 0xFF;

   //set RW and RS and E to 0
   LCD_CTRL_PORT &= ~RS_MASK; 
   LCD_CTRL_PORT |= E_MASK;   

   //give it time before turning on
   _delay_ms(200);
   
   //set 8-bit mode
   functionSet(1,1,0);
   //delay for this command
   _delay_ms(40);

   //set the direction of cursor
   entryModeSet(1,0);
   //delay for this command
   _delay_ms(40); 

   //clear display
   clearDisplay();
   //delay for this command
   _delay_ms(2); 

   //set display on, cursor on, blinking on
   displayOnOff(1,1,1);
   //delay for this command
   _delay_ms(40);
}


/*
 * @brief commands the LCD to clear display
 */
void clearDisplay()
{
   LCD_CTRL_PORT &= ~RS_MASK; 

   //clear display
   writeRAM(0x01);
}

/*
 * @brief commands the LCD to move curser to home
 */
void cursorHome()
{
   LCD_CTRL_PORT &= ~RS_MASK; 

   //set cursor to first spot
   writeRAM(0x02);
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
   if(incCursor == 1) 
   {
      entryMode = (entryMode | 0x02);
   }
   //or with mask of 00000001 to set shift on
   if(shiftDisp == 1)
   {
      entryMode = (entryMode | 0x01);
   }
   
   writeRAM(entryMode);
   
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

   //display on or off
   if(dispOn == 1)
   {
      //set display to on
      dispOnOff = (dispOnOff | 0x04);
   }

   if(cursOn)
   {
      //set cursor to on
      dispOnOff = (dispOnOff | 0x02);
   } 

   if(cursBlink)
   {
      //set blink to on
      dispOnOff = (dispOnOff | 0x01);
   }     

   writeRAM(dispOnOff); 
   
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

   if(shiftDisp==1)
   {
      //shift display
      cursorDispShift = (cursorDispShift | 0x08);
   } 

   if(shiftDisp==1)
   {
      //shift right
      cursorDispShift = (cursorDispShift | 0x04);
   }  
   
   writeRAM(cursorDispShift);
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
   
   writeRAM(function);
      
}

/*
 * @brief set the address into the CGRAM memory section.
 *
 * @param address the address to ser CGRAM to
 */
void setCGRAMaddr(uint8_t address)
{
   LCD_CTRL_PORT |= RS_MASK;    

   writeRAM(0x40 | address);
}

/*
 * @brief set the address into the DDRAM memory section.
 *
 * @param address the address to set DDRAM to
 */
void setDDRAMaddr(uint8_t address)
{
   LCD_CTRL_PORT &= ~RS_MASK; 

   writeRAM(0x80 | address);
}

/*
 * @brief writes to ram, the last ram region set is the ram region that will be
 * written to.
 *
 * @param data data to write to ram
 */
void writeRAM(uint8_t data)
{   

   //write data to the data port
   LCD_DAT_PORT = data; 
   _delay_ms(1);

   //set RW
   LCD_CTRL_PORT &= ~E_MASK;
   _delay_ms(1);
   LCD_CTRL_PORT |= E_MASK;
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
/*
   char tmp[128]; //limits length of string
   va_list args;
   va_start(args, fmt);
   vsnprintf(tmp, 128, fmt, args);
   va_end(args);
   Serial.print(tmp);
*/
}
