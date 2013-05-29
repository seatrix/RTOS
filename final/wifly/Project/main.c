#include <stdint.h>
#include <avr/io.h>
#include <string.h>
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "wifly.h"
#include "serial.h"

// Global variables
struct wifly wf;

void UARTTask(void *args) 
{
   while (1) {
      wifly_uart_rx_tx(&wf); // Poll the UART rx/tx buffer
      vTaskDelay(0); // Allow this task to yeild
   }
}

void WiflyTask(void *args)
{
   while (1) {
      wifly_check_state(&wf); // Increment the wifly state machine
      // Task delays are given within the wifly state check function
   }
}

void ReceiveTask(void *args)
{
   int cnt;
   char buff[128];

   while (1) {

      // Receive command type bytes
      if (3 == (cnt = wifly_receive(&wf, buff, 3))) {
         writeBytes(">>>", 3, USART0);
         writeBytes(buff, 3, USART0);
         writeBytes("\r\n", 2, USART0);
         
         // Color change command
         if (!strncmp(buff, "COL", 3)) {
            if (6 == (cnt = wifly_receive(&wf, buff, 6))) {
               writeBytes("$COLOR=", 7, USART0);
               writeBytes(buff, cnt, USART0);
               writeBytes("\r\n", 2, USART0);
            }
            
         // Toggle display command
         } else if (!strncmp(buff, "TOG", 3)) {
            writeBytes("$TOGGLE\r\n", 9, USART0);
         }
      }

      wifly_flush(&wf);
   }
}

int main(void)
{
   setupSerial(9600, USART0);
   setupSerial(9600, USART1);
   wifly_setup(&wf);

   xTaskCreate(UARTTask, (const signed char *) "uart", 100, NULL, 1, NULL);
   xTaskCreate(WiflyTask, (const signed char *) "wifly", 100, NULL, 2, NULL);
   xTaskCreate(ReceiveTask, (const signed char *) "receive", 200, NULL, 2, NULL);
   vTaskStartScheduler();

   return 0;
}
