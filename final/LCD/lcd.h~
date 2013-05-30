/*
 * @file lcd.h
 * @brief general Hitachi HD44780 interface header file
 * @author Daniel Jennings
 */


#ifndef _LCD_H
#define _LCD_H

#define RS_MASK   0x01;
#define RW_MASK   0x02;
#define E_MASK    0x04;
#define LCD_CTRL_PORT   PORTB;
#define LCD_DAT_PORT    PORTC;

#include <stdint.h>

/*
 * @brief sets up the interface to LCD, and enters initial commands
 */
void setupLCD();

/*
 * @brief commands the LCD to clear display
 */
void clearDisplay();

/*
 * @brief commands the LCD to move curser to home
 */
void cursorHome();

/*
 * @brief entry mode set command.
 *
 * @param incCursor 1 to increment cursor on write, 0 to de-increment
 * @param shiftDisp 1 to shift display on write, 0 to not shift
 */
void entryModeSet(uint8_t incCursor, uint8_t shiftDisp);

/*
 * @brief display on off command.
 *
 * @param dispOn 1 for display on, 0 for off
 * @param cursOn 1 for curso on, 0 for off
 * @param cursBlink 1 for cursor blinkage, 0 for off
 */
void displayOnOff(uint8_t dispOn, uint8_t cursOn, uint8_t cursBlink);

/*
 * @brief cursor display shift command. This command acts immediatly.
 *
 * @param shiftDisp 1 to shift display, 0 to shift cursor
 * @param shiftRight 1 to shift right, 0 to shift left
 */
void cursorDisplayShift(uint8_t shiftDisp, uint8_t shiftRight);

/*
 * @brief functionSet command.
 *
 * @param bytemode 1 for 8bit data transfers, 0 for nibble mode (4bit)
 * @param twolines 1 for twoline screen, 0 for 1 line screen
 * @param font 1 for 5x10 character set, 0 for 5x8 character set
 */
void functionSet(uint8_t bytemode, uint8_t twolines, uint8_t font);

/*
 * @brief set the address into the CGRAM memory section.
 *
 * @param address the address to ser CGRAM to
 */
void setCGRAMaddr(uint8_t address);

/*
 * @brief set the address into the DDRAM memory section.
 *
 * @param address the address to set DDRAM to
 */
void setDDRAMaddr(uint8_t address);

/*
 * @brief writes to ram, the last ram region set is the ram region that will be
 * written to.
 *
 * @param data data to write to ram
 */
void writeRAM(uint8_t data);

/*
 * @brief lcd printf function. Prints a variadic format string to LCD.
 *
 * @param line 0 to write on first line, 1 to write on second line
 * @param fmt a format string similar to printf's format string
 * @param ... variadic arguments containing values for format string
 */
void lcdprintf(uint8_t line, const char *fmt, ...);

#endif
