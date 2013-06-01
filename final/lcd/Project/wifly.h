#ifndef _WIFLY_H
#define _WIFLY_H
#include <stdint.h>

#define RXBUFF_SZ 128
#define TXBUFF_SZ 128

struct wifly {
   char txbuffer[TXBUFF_SZ];
   uint16_t txoffset;
   uint16_t txbytes;

   char rxbuffer[RXBUFF_SZ];
   uint16_t rxoffset;
   uint16_t rxbytes;

   uint16_t currstate;
   uint8_t rxtx_enabled;
};

void wifly_setup(struct wifly *wf);

void wifly_uart_rx_tx(struct wifly *wf);

void wifly_check_state(struct wifly *wf);

int wifly_transmit(struct wifly *wf, char *src, uint16_t bytes);

int wifly_receive(struct wifly *wf, char *dst, uint16_t bytes);

void wifly_flush(struct wifly *wf);

#endif
