/* ----------------------------------------------------------------
 * Filename: spi_sseg.h
 * 
 * This file provides support for the associated ".c" file. 
 * The function headers should be the same for both files. While not 
 * optimal, that seemed like the best thing to do at the moment. 
 *
 * -bryan (mealy) 2011
 *-----------------------------------------------------------------
 */ 

#include <stdint.h>

//----------------------------------------------
//- This driver uses the following:  
//-    PB4 as the chip select (arbitrary) 
//-    PB5 as the MOSI output
//-    PB7 as the clock output
//----------------------------------------------
#define SPI_SS     4
#define SPI_MOSI   5
#define SPI_SCK    7

// various escape code stuff for the 7-seg display
#define SSEG_BRIGHTNESS  0x7A 
#define SSEG_RESET       0x76 
#define SSEG_SPACE       0x78 
#define SSEG_DEC_PNT     0x77


//- date used to turn on individual bits for 0-F 
//- plus everything off (final value) 
#define SSEG_0     0x3F
#define SSEG_1     0x06
#define SSEG_2     0x5B
#define SSEG_3     0x4F
#define SSEG_4     0x66
#define SSEG_5     0x6D
#define SSEG_6     0x7D
#define SSEG_7     0x07
#define SSEG_8     0x7F
#define SSEG_9     0x67
#define SSEG_A     0x77
#define SSEG_B     0x7C
#define SSEG_C     0x79
#define SSEG_D     0x5E
#define SSEG_E     0x79
#define SSEG_F     0x71
#define SSEG_BLANK 0x00


//- decimal point stuff 
#define SSEG_DP_0   (0x01 << 0)   //- digit1 DP
#define SSEG_DP_1   (0x01 << 1)   //- digit2 DP
#define SSEG_DP_2   (0x01 << 2)   //- digit3 DP
#define SSEG_DP_3   (0x01 << 3)   //- digit4 DP
#define SSEG_DP_4   (0x01 << 4)   //- colon between digits two and three
#define SSEG_DP_5   (0x01 << 5)   //- digit3 degree


//- DIGIT_1 is the left-most display
#define DIGIT_1   0x01
#define DIGIT_2   0x02
#define DIGIT_3   0x03
#define DIGIT_4   0x04


//- escape code for individual 7-segment digits
//- DIG1 is the left-most digit
#define SSEG_DIG1 0x7B
#define SSEG_DIG2 0x7C
#define SSEG_DIG3 0x7D
#define SSEG_DIG4 0x7E



//--------------------------------------------------------------
//- Function: SPI_MasterInit()
//- 
//- return value: none
//- parameters: none
//-
//- Description: Initializes the AVR device as a master and 
//- and turns on the device and sets the clock frequency. 
//- For convenience, this function also configures the SPI
//- port GPIOs
//---------------------------------------------------------------
void SPI_MasterInit(void); 



//--------------------------------------------------------------
//- Function: SPI_MasterTransmit()
//- 
//- return value: none
//- parameters: data to be transmitted
//-
//- Description: This function selects the device, transmits
//- the data, waits for a the transmission to complete with 
//- a poll, then deselects the device. 
//---------------------------------------------------------------
void SPI_MasterTransmit(uint8_t data); 



//--------------------------------------------------------------
//- Function: SSEG_Set_Brightness()
//- 
//- return value: none
//- parameters: val, the value to set the brightness of display
//-
//- Description: This function set the brightness of the 
//- display where 255 is off and 0 is on as bright as possible. 
//---------------------------------------------------------------
void SSEG_Set_Brightness(uint8_t val); 



//--------------------------------------------------------------
//- Function: SSEG_Reset()
//- 
//- return value: none
//- parameters: none
//-
//- Description: This function turns off all display elements. 
//---------------------------------------------------------------
void SSEG_Reset(void); 



//--------------------------------------------------------------
//- Function: SSEG_Write_4vals_array()
//- 
//- return value: none
//- parameters: pointer to an array
//-
//- Description: This function takes a pointer that in theory is 
//- pointing to an array of four byte values, one for 
//- each of the 7-segment display locations. The four values
//- are then written to the 7-segment display.  
//---------------------------------------------------------------
void SSEG_Write_4vals_array(uint8_t* vals); 



//--------------------------------------------------------------
//- Function: SSEG_Write_digit()
//- 
//- return value: none
//- parameters: digit location and digit value
//-
//- Description: This is a wrapper function for writing 
//- individual digits to the display. In this case, you need 
//- to first send the initial escape character before you 
//- send the actual data. 
//---------------------------------------------------------------
void SSEG_Write_digit(uint8_t digit, uint8_t val); 



//--------------------------------------------------------------
//- Function: SSEG_Write_left_digits()
//- 
//- return value: none
//- parameters: binary value for display on two left-most digits
//-
//- Description: This function decomposes the sent value into
//- a tens and ones digit the then sends them off to the two 
//- left-most digits of the display. This function also handles
//- lead zero blanking. 
//---------------------------------------------------------------
void SSEG_Write_left_digits(uint8_t val); 



//--------------------------------------------------------------
//- Function: SSEG_Write_right_digits()
//- 
//- return value: none
//- parameters: binary value for display on two right-most digits
//-
//- Description: This function decomposes the sent value into
//- a tens and ones digit the then sends them off to the two 
//- right-most digits of the display. This function also handles
//- lead zero blanking. 
//---------------------------------------------------------------
void SSEG_Write_right_digits(uint8_t val); 


//--------------------------------------------------------------
//- Function: SSEG_Write_Decimal_Point()
//- 
//- return value: none
//- parameters: the individual values for the various "dots" on 
//-             the 7-segment display. 
//-
//- Description: Writes decimal point values to the display. 
//- There are six different values that can be written. Check 
//- out the 7-segment device spec for details on decimal points.
//---------------------------------------------------------------
void SSEG_Write_Decimal_Point(uint8_t val); 
