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
#include <string.h>
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "lcd.h"

#define RS_MASK   0x01
#define E_MASK    0x02
#define R_MASK    0x04
#define G_MASK    0x08
#define B_MASK    0x10

#define LCD_CRTL_DDR    DDRA
#define LCD_DAT_DDR     DDRC

#define LCD_CTRL_PORT   PORTA
#define LCD_DAT_PORT    PORTC

// RGB globals
static uint8_t _counter;
static uint8_t _red;
static uint8_t _green;
static uint8_t _blue;

// RGB color driver ISR
ISR(TIMER0_COMPA_vect)
{
   if (_counter == 0)
      LCD_CTRL_PORT &= ~(R_MASK | G_MASK | B_MASK);

   if (_red == _counter)
      LCD_CTRL_PORT |= R_MASK;

   if (_green == _counter)
      LCD_CTRL_PORT |= G_MASK;

   if (_blue == _counter)
      LCD_CTRL_PORT |= B_MASK;

   _counter++;
}

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
   vTaskDelay(1 / portTICK_RATE_MS);
   LCD_CTRL_PORT |= E_MASK;
}

// Write string to LCD, max of n characters
static void __writeString(char *string, uint8_t n)
{
   while (*string != '\0' && n-- > 0)
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
   vTaskDelay(200 / portTICK_RATE_MS);
   
   functionSet(1,1,0);
   entryModeSet(1,0);
   clearDisplay();
   displayOnOff(1,0,0);

   // Initialize screen color ISR
   _counter = 0;
   _red = 0;
   _green = 0;
   _blue = 0;
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

   vTaskDelay(2 / portTICK_RATE_MS);
}

/*
 * @brief commands the LCD to move curser to home
 */
void cursorHome()
{
   LCD_CTRL_PORT &= ~RS_MASK; 

   //set cursor to first spot
   __writeLCD(0x02);

   vTaskDelay(2 / portTICK_RATE_MS);
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

   vTaskDelay(1 / portTICK_RATE_MS);
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
   
   vTaskDelay(1 / portTICK_RATE_MS);
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

   vTaskDelay(1 / portTICK_RATE_MS);
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
      
   vTaskDelay(1 / portTICK_RATE_MS);
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

   vTaskDelay(1 / portTICK_RATE_MS);
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

   vTaskDelay(1 / portTICK_RATE_MS);
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

   vTaskDelay(1 / portTICK_RATE_MS);
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
   static char blank[17] = {' ',' ',' ',' ',' ',' ',' ',' ',' ',
                                ' ',' ',' ',' ',' ',' ',' ','\0'};
   char tmp[17];

   // Move cursor to beginning of selected line
   if (line == 0)
      setDDRAMaddr(0x00);
   else
      setDDRAMaddr(0x40);

   va_list args;
   va_start(args, fmt);
   vsnprintf(tmp, 17, fmt, args);
   va_end(args);
   tmp[16] = '\0';

   // Write the formated string
   __writeString(tmp, 17);

   // Clear the rest of the line
   __writeString(blank, 17 - strlen(tmp));
}

/*
 * @brief changes the color of the LCD to the 8-bit RGB value given.
 *
 * @param red the amount of red (8bit). 0 = none, 255 = all.
 * @param green the amount of green (8bit). 0 = none, 255 = all.
 * @param blue the amount of blue (8bit). 0 = none, 255 = all.
 */
void lcd_setColorRGB(uint8_t red, uint8_t green, uint8_t blue)
{
   _counter = 0;
   _red = red;
   _green = green;
   _blue = blue;
}

/*
 * @brief changes the color of the LCD to the 8-bit CMY value given.
 *
 * @param cyan the amount of cyan (8bit). 0 = none, 255 = all.
 * @param magenta the amount of magenta (8bit). 0 = none, 255 = all.
 * @param yellow the amount of yellow (8bit). 0 = none, 255 = all.
 */
void lcd_setColorCMY(uint8_t cyan, uint8_t magenta, uint8_t yellow)
{
   _counter = 0;
   _red = 255 - cyan;
   _green = 255 - magenta;
   _blue = 255 - yellow;
}
