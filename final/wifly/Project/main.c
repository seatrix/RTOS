#include <stdint.h>
#include <avr/io.h>
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
   char buff[4];

   while (1) {
      if (0 < (cnt = wifly_receive(&wf, &buff, 4))) {
         writeBytes(buff, cnt, 0);
      }
   }
}

int main(void)
{
   setupSerial(9600, USART0);
   setupSerial(9600, USART1);
   wifly_setup(&wf);

   xTaskCreate(UARTTask, (const signed char *) "uart", 100, NULL, 1, NULL);
   xTaskCreate(WiflyTask, (const signed char *) "wifly", 100, NULL, 2, NULL);
   //xTaskCreate(ReceiveTask, (const signed char *) "receive", 100, NULL, 2, NULL);
   vTaskStartScheduler();

   return 0;
}
