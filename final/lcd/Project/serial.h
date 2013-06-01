/*
 * @file serial.h
 * @brief serial interface header
 * @author Matt Zimmerer
 */

#ifndef _SERIAL_H
#define _SERIAL_H
#include <stdint.h>

#define USART0 0
#define USART1 1
#define USART2 2
#define USART3 3

/*
 * @brief Sets up the serial interface for communication.
 *
 * @param baudrate - The baudrate the serial device will use for read/write.
 * @param usartn   - The enumerated usart port
 */
void setupSerial(uint32_t baudrate, uint8_t usartn);

/*
 * @brief Checks if data can be read from the serial port.
 *
 * @param usartn   - The enumerated usart port
 * @return 1 if data can be read, 0 if not.
 */
int canRead(uint8_t usartn);

/*
 * @brief Checks if data can be written to the serial port.
 *
 * @param usartn   - The enumerated usart port
 * @return 1 if data can be written, 0 if not.
 */
int canWrite(uint8_t usartn);

/*
 * @brief Reads a byte into the 8 bit field pointed to by dst. This call may
 * fail if the serial port is unavailable.
 *
 * @param dst - Pointer to destination of read byte.
 * @param usartn   - The enumerated usart port
 * @return 0 on success, -1 if port is busy or unavailable.
 */
int readByte_nonblocking(char *dst, uint8_t usartn);

/*
 * @brief Writes a byte from the 8 bit parameter to the serial line. This call
 * may fail if the serial port is unavailable.
 *
 * @param dst - Byte to write.
 * @param usartn   - The enumerated usart port
 * @return 0 on success, -1 if port is busy or unavailable.
 */
int writeByte_nonblocking(char byte, uint8_t usartn);

/*
 * @brief Reads a byte into the 8 bit field pointed to by dst. This is a
 * blocking version of readBye_nonblocking. This function cannot fail, but may
 * block indefinitely.
 *
 * @param dst - Pointer to destination of read byte.
 * @param usartn   - The enumerated usart port
 */
void readByte_blocking(char *dst, uint8_t usartn);

/*
 * @brief Writes a byte from the 8 bit parameter to the serial line. This is a
 * blocking version of writeByte_nonblocking. This function cannot fail, but may
 * block indefinitely.
 *
 * @param dst - Byte to write.
 * @param usartn   - The enumerated usart port
 */
void writeByte_blocking(char byte, uint8_t usartn);

/*
 * @breif Reads an array of bytes from the serial line. This uses non blocking
 * read calls, and will fail if the resource becomes unavailable.
 *
 * @param dst - Pointer to an destination array
 * @param bytes - The number of bytes to read
 * @param usartn   - The enumerated usart port
 * @return the number of bytes read
 */
int readBytes(char *dst, uint16_t bytes, uint8_t usartn);

/*
 * @breif Write an array of bytes to the serial line. This uses non blocking
 * write calls, and will fail if the resource becomes unavailable.
 *
 * @param dst - Pointer to an source array
 * @param bytes - The number of bytes to write
 * @param usartn   - The enumerated usart port
 * @return the number of bytes written
 */
int writeBytes(char *src, uint16_t bytes, uint8_t usartn);

/*
 * @brief Flushes the rx serial buffer
 *
 * @param usartn   - The enumerated usart port
 */
void flushSerial(uint8_t usartn);

#endif
