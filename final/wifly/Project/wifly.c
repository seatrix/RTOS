#include <stdio.h>
#include <string.h>
#include "wifly.h"
#include "serial.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

#if (INCLUDE_vTaskSuspend != 1)
#error INCLUDE_vTaskSuspend must be set to 1 to compile wifly.c
#endif

// Hard coded WLAN constants
#define SSID "paradise"
#define PW   "kiwibird"

// Delay values in ms
#define RESET_DELAY  100
#define BOOT_DELAY   300
#define FLUSH_DELAY  1000
#define RX_DELAY     100
#define IDLE_DELAY   30*1000

// State enumerations
#define CMDMODE   1 // Enter command mode
#define UARTNE    2 // Disable UART rx echo
#define DHCPC     3 // Enable DHCP
#define QUIET     4 // Disable verbose wifly output
#define JWAIT     5 // Extend join wait to 5000ms
#define AUTH      6 // Set auth to mixed wpa1-wpa2psk
#define SETPW     7 // Set password (hardcoded)
#define SETSSID   8 // Set SSID (hardcoded)
#define HIDEKEY   9 // Hide password
#define AUTOJOIN  10 // Set wifly to autojoin
#define DATMODE   11 // Exit command mode
#define FLUSH     12 // Pre idle mode flush
#define IDLE      127 // Do nothing

// Wifly pinout defines
#define WF_USART     USART1
#define WF_DDR       DDRD
#define WF_PORT      PORTD
#define WF_RESET_PIN (1 << PD4)

// Special function defines
#define SF_NONE         (0 << 0) // Do nothing
#define SF_RESET        (1 << 0) // Hard reset the wifly
#define SF_DFLUSH       (1 << 1) // Flush the uart rx buffer
#define SF_RXTX_ENABLE  (1 << 2) // Enable wifly transmissions
#define SF_RXTX_DISABLE (1 << 3) // Disable wifly transmissions
#define SF_IDLE         (1 << 4) // Idle
#define SF_STARTFLAGS   (SF_RXTX_DISABLE | SF_RESET | SF_DFLUSH)

// Debuging string defs
#define STR_INVSTATE        "wifly - State not found\r\n"
#define STR_CMDMODE         "wifly - Command mode\r\n"
#define STR_UARTNE          "wifly - Disabling uart rx echo\r\n"
#define STR_QUIET           "wifly - Quiet mode enabled\r\n"
#define STR_DHCPC           "wifly - Enable DHCP\r\n"
#define STR_JWAIT           "wifly - Setting join wait to 5000ms\r\n"
#define STR_AUTH            "wifly - Setting auth to WPA1-WPA2psk\r\n"
#define STR_SETPW           "wifly - Setting password\r\n"
#define STR_SETSSID         "wifly - Setting SSID to "SSID"\r\n"
#define STR_HIDEKEY         "wifly - Hiding passphrase\r\n"
#define STR_AUTOJOIN        "wifly - Auto joining\r\n"
#define STR_DATMODE         "wifly - Data mode entered\r\n"
#define STR_FLUSH           "wifly - Flushing received bytes\r\n"
#define STR_IDLE            "wifly - Idling with rx/tx enabled\r\n"

static xSemaphoreHandle uartSem;

// State machine lookup-table
static struct wifly_state {
   int8_t state_enum;  // This states enumeration
   int8_t state_next;  // The next state
   int8_t state_recov; // The recovery state (where to go upon failure)
   uint8_t sfunc;      // Special function bit fields
   const char *txstr;  // The outgoing command
   int16_t txdelay;    // Delay (ms) to observe before sending tx cmd
   const char *rxstr;  // The expected receive command
   int16_t rxdelay;    // Delay (ms) to observe before reading rx cmd
} state_table[] = {
   {CMDMODE,UARTNE,CMDMODE,SF_STARTFLAGS,"$$$",0,"CMD\r\n",500},
   {UARTNE,QUIET,CMDMODE,SF_NONE,"set u m 1\r",0,"set u m 1\r\r\nAOK\r\n",50},
   {QUIET,DHCPC,CMDMODE,SF_NONE,"set s p 0\r",0,"AOK\r\n",50},
   {DHCPC,JWAIT,CMDMODE,SF_NONE,"set i d 1\r",0,"AOK\r\n",50},
   {JWAIT,AUTH,CMDMODE,SF_NONE,"set o j 5000\r",0,"AOK\r\n",50},
   {AUTH,SETPW,CMDMODE,SF_NONE,"set w a 4\r",0,"AOK\r\n",50},
   {SETPW,SETSSID,CMDMODE,SF_NONE,"set w p "PW"\r",0,"AOK\r\n",750},
   {SETSSID,HIDEKEY,CMDMODE,SF_NONE,"set w s "SSID"\r",0,"AOK\r\n",750},
   {HIDEKEY,AUTOJOIN,CMDMODE,SF_NONE,"set w h 1\r",0,"AOK\r\n",50},
   {AUTOJOIN,DATMODE,CMDMODE,SF_NONE,"set w j 1\r",0,"AOK\r\n",50},
   {DATMODE,FLUSH,CMDMODE,SF_NONE,"exit\r",0,"EXIT\r\n",50},
   {FLUSH,IDLE,IDLE,SF_DFLUSH,"",0, "",0},
   {IDLE,IDLE,IDLE,SF_RXTX_ENABLE | SF_IDLE,"",0, "",0},
   {-1,-1,-1,SF_NONE,NULL,-1,NULL,-1}
};

static struct wifly_state * lookup_state(struct wifly *wf)
{
   struct wifly_state *s;

   // Navagate LUT and return correct state pointer
   for (s = state_table; s && s->state_enum != -1; s++)
      if (s->state_enum == wf->currstate)
         return s;

   return NULL;
}

void wifly_setup(struct wifly *wf)
{
   WF_DDR |= WF_RESET_PIN;
   WF_PORT |= WF_RESET_PIN;

   vSemaphoreCreateBinary(uartSem);

   wf->rxoffset = 0;
   wf->rxbytes = 0;
   wf->txoffset = 0;
   wf->txbytes = 0;

   wf->currstate = CMDMODE;
   wf->rxtx_enabled = 0;
}

void wifly_uart_rx_tx(struct wifly *wf)
{
   xSemaphoreTake(uartSem, portMAX_DELAY);

   // Read from the receive port
   if (wf->rxoffset < wf->rxbytes)
      if (!readByte_nonblocking(&wf->rxbuffer[wf->rxoffset], WF_USART))
         wf->rxoffset++;

   // Write to the transmit port
   if (wf->txoffset < wf->txbytes)
      if (!writeByte_nonblocking(wf->txbuffer[wf->txoffset], WF_USART))
         wf->txoffset++;

   xSemaphoreGive(uartSem);
}

static int handleSpecialFunctions(struct wifly *wf, struct wifly_state *cs)
{
   // Disable upper layer rx/tx
   if (cs->sfunc & SF_RXTX_DISABLE)
      xSemaphoreTake(uartSem, portMAX_DELAY);
      wf->rxtx_enabled = 0;
      wf->rxbytes = 0;
      xSemaphoreGive(uartSem);

   // Enable upper layer rx/tx
   if (cs->sfunc & SF_RXTX_ENABLE) {
      xSemaphoreTake(uartSem, portMAX_DELAY);
      wf->rxtx_enabled = 1;
      wf->rxoffset = 0;
      wf->rxbytes = RXBUFF_SZ;
      flushSerial(WF_USART);
      xSemaphoreGive(uartSem);
   }

   // Idle this state machine
   if (cs->sfunc & SF_IDLE) {
      writeBytes(STR_IDLE, strlen(STR_IDLE), USART0);
      vTaskDelay(IDLE_DELAY / portTICK_RATE_MS);
      return 1;
   }

   // Hard reset the wifly
   if (cs->sfunc & SF_RESET) {
      WF_PORT &= ~WF_RESET_PIN;
      vTaskDelay(RESET_DELAY / portTICK_RATE_MS);
      WF_PORT |= WF_RESET_PIN;
      vTaskDelay(BOOT_DELAY / portTICK_RATE_MS);
   }

   // Flush the wifly rx buffer
   if (cs->sfunc & SF_DFLUSH) {
      vTaskDelay(FLUSH_DELAY / portTICK_RATE_MS);
      xSemaphoreTake(uartSem, portMAX_DELAY);
      wf->rxoffset = 0;
      wf->rxbytes = 0;
      flushSerial(WF_USART);
      xSemaphoreGive(uartSem);
   }

   return 0;
}

void wifly_check_state(struct wifly *wf)
{
   struct wifly_state *cs = lookup_state(wf);

   if (handleSpecialFunctions(wf, cs))
      return;

   // Debugging messages
   switch (cs->state_enum) {
      case CMDMODE: writeBytes(STR_CMDMODE, strlen(STR_CMDMODE), USART0);
         break;
      case UARTNE: writeBytes(STR_UARTNE, strlen(STR_UARTNE), USART0);
         break;
      case QUIET: writeBytes(STR_QUIET, strlen(STR_QUIET), USART0);
         break;
      case DHCPC: writeBytes(STR_DHCPC, strlen(STR_DHCPC), USART0);
         break;
      case JWAIT: writeBytes(STR_JWAIT, strlen(STR_JWAIT), USART0);
         break;
      case AUTH: writeBytes(STR_AUTH, strlen(STR_AUTH), USART0);
         break;
      case SETPW: writeBytes(STR_SETPW, strlen(STR_SETPW), USART0);
         break;
      case SETSSID: writeBytes(STR_SETSSID, strlen(STR_SETSSID), USART0);
         break;
      case HIDEKEY: writeBytes(STR_HIDEKEY, strlen(STR_HIDEKEY), USART0);
         break;
      case AUTOJOIN: writeBytes(STR_AUTOJOIN, strlen(STR_AUTOJOIN), USART0);
         break;
      case DATMODE: writeBytes(STR_DATMODE, strlen(STR_DATMODE), USART0);
         break;
      case FLUSH: writeBytes(STR_FLUSH, strlen(STR_FLUSH), USART0);
         break;
      default: writeBytes(STR_INVSTATE, strlen(STR_INVSTATE), USART0);
         return;
   }

   // Optional delay before transmit
   if (cs->txdelay)
      vTaskDelay(cs->txdelay / portTICK_RATE_MS);

   // Transmit/receive setup
   xSemaphoreTake(uartSem, portMAX_DELAY);
   flushSerial(WF_USART);
   memcpy(wf->txbuffer, cs->txstr, strlen(cs->txstr));
   wf->rxoffset = 0;
   wf->rxbytes = strlen(cs->rxstr);   
   wf->txoffset = 0;
   wf->txbytes = strlen(cs->txstr);   
   xSemaphoreGive(uartSem);
 
   // Optional delay before receive
   if (cs->rxdelay)
      vTaskDelay(cs->rxdelay / portTICK_RATE_MS);

   // Check the received string and respond appropriately
   xSemaphoreTake(uartSem, portMAX_DELAY);
   if (wf->rxoffset == strlen(cs->rxstr)
          && !memcmp(wf->rxbuffer, cs->rxstr, strlen(cs->rxstr))) {
      wf->currstate = cs->state_next;
   } else 
      wf->currstate = cs->state_recov;
   xSemaphoreGive(uartSem);
}

int wifly_transmit(struct wifly *wf, char *src, uint16_t bytes)
{
   // If the interface is not ready, return error
   if (!wf->rxtx_enabled)
      return -1;

   xSemaphoreTake(uartSem, portMAX_DELAY);
   memcpy(wf->txbuffer, src, bytes);
   wf->txbytes = bytes;
   wf->txoffset = 0;
   xSemaphoreGive(uartSem);

   return bytes;
}

int wifly_receive(struct wifly *wf, char *dst, uint16_t bytes)
{
   uint16_t ndx;

   // If the interface is not ready, return error
   if (!wf->rxtx_enabled) {
      vTaskDelay(RX_DELAY / portTICK_RATE_MS);
      return -1;
   }

   xSemaphoreTake(uartSem, portMAX_DELAY);

   // Loop until all bytes have been received
   while (wf->rxoffset < bytes) {
      xSemaphoreGive(uartSem);
      vTaskDelay(RX_DELAY / portTICK_RATE_MS);
      xSemaphoreTake(uartSem, portMAX_DELAY);
   }

   // Copy back to user buffer, adjust rx buffer to preserve extra bytes
   memcpy(dst, wf->rxbuffer, bytes);
   for (ndx = 0; ndx < wf->rxoffset - bytes; ndx++)
      wf->rxbuffer[ndx] = wf->rxbuffer[ndx + bytes];
   wf->rxoffset -= bytes;

   xSemaphoreGive(uartSem);

   return bytes;
}
