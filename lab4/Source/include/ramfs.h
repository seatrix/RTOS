/*
  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE. */

#ifndef _AVR_RAMFS_H_
#define _AVR_RAMFS_H_ 1

#include <stddef.h>				// size_t
#include <stdint.h>  			// has to be added to use uint8_t
#include <avr/io.h>

#include <spi.h>
#include <ext_ram.h>

/**
    This header file declares the interface to some simple library
    routines suitable for handling RAMFS contained in the
    AVR micro controllers.

    - All of the read/write functions first make sure the RAMFS is
    ready to be accessed.  Since this may cause long delays if a
    write operation is still pending, time-critical applications
    should first poll the RAMFS e. g. using ramfs_is_ready() before
    attempting any actual I/O.

    - As these functions modify IO registers, they are known to be
    non-reentrant.  If any of these functions are used from both,
    standard and interrupt context, the applications must ensure
    proper protection (e.g. by disabling interrupts before accessing
    them).

 */

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega328P__)
#ifndef defined(portEXT_RAMFS)		// The client Arduino doesn't have XRAMSTART and XRAMEND, so define these here
									// Pointer to the start and end of extended memory.
#define XRAMSTART					(uint8_t *) 0x8000
#define XRAMEND						(uint8_t *) 0xFFFF		// XRAMEND is a system define on 2560 & 2561, but defaults to RAMEND on 328p
#endif
#endif

#define configTOTAL_RAMFS_SIZE 		XRAMEND - XRAMSTART		// and continue for (32k -1)Byte.  XRAMEND is a system define on 2560 & 2561

#define HIGH_BITS					(uint8_t) 0xFF	// All bits set to one.

#define RAMFSCALLQUEUEDEPTH			16		// depth of calls the Supervisor will maintain. There can't be more than 16 clients, each holding one request.

#define CLIENT_CALL_US				30		// time that the Client (Arduino) holds its SS line low to call Master (Supervisor)
											// this should be longer than the length of time the Master needs to respond to an interrupt.
											// but it should be fairly constant, even under varying load, provided the Supervisor is not in portCRITICAL tasks.
											// Measured maximum value with no other tasks competing is 31us.

#define WAIT_FOR_SPIF				!(SPSR & _BV(SPIF))			// Wait for SPIF, and
#define CHECK_FOR_MY_SS				(SPI_PORT_PIN & SPI_BIT_SS)	// check we still have SS low (we're selected). Use this everywhere for SPI wait loop.


typedef enum {
	Huh	   = 0, // Client didn't issue us a command, so just break.
	Read   = 1, // read from RAMFS (Supervisor) & write to SPI
	Write  = 2, // read from SPI (Supervisor) & write to RAMFS
	Swap   = 3, // read from both RAMFS & SPI (Supervisor), and swap the contents
	Test   = 4  // do something else, to be determined
} RAMFSCommand; // from point of view of the client (Arduino 328p)


typedef struct						/* structure to hold the RAMFS info */
{
	RAMFSCommand	ram_cmd;		// Read / Write / Swap / Test
	size_t   		ram_addr;		// Address of first byte of RAM in a RAMFS (greater than RAM_START_ADDR)
	uint16_t   		ram_size;		// Size of RAM block in RAMFS (less than RAM_COUNT or 32kByte)
	uint8_t    		ram_crc8;		// Calculated CRC of stored data
} xRAMFSarray, * pRAMFSarray;


#if defined(__AVR_ATmega640__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega1281__) || defined(__AVR_ATmega2560__) || defined(__AVR_ATmega2561__)

/*
 * Declare a variable of type xQueueHandle.
 * One queue will have the RAMFS requests written to it.
 */
xQueueHandle xRAMFSCallQueue;

void init16PCINTpins(void);

#elif defined(__AVR_ATmega168__) || defined(__AVR_ATmega328P__)

/** Returns 1 if RAMFS RAM transfer successful for read/write operation, 0 if not.
    Check that we can use the SPI bus.
    Check that the RAMFS server can service us, by pulsing the SS low
    to trigger an interrupt on the server.
    Release the SS line so that the server can issue a transfer command.
 */
uint8_t ramfs_transfer_block(pRAMFSarray pRAMFS_block, uint8_t *data);

/*
 * Memory management routines required for RAMFS.
 */
size_t vRAMFSMalloc( size_t xSize );
void vRAMFSFree( size_t v );
void vRAMFSInitialiseBlocks( void );
size_t xRAMFSGetFreeSize( void );

#endif

#ifdef __cplusplus
}
#endif


#endif	/* !_AVR_ramfs_H_ */
