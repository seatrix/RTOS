/*
    FreeRTOS V6.1.0 

    This is a basic RTOS program. It turns on the LEDs. 
	Nothing too exciting. 

*/ 

#include <stdint.h>
#include <avr/io.h>

#include "FreeRTOS.h"
#include "task.h"

void vTaskFunction_1(void *pvParameters); 
void vIO_init(void); 

/*-----------------------------------------------------------*/

int main( void )
{  	
    //- wimpy variable defs
	uint8_t val[2] = {0x3F, 0xFE}; 
    uint8_t *val0, *val1; 

	//- avoid pesky warnings... 
	val0 = val + 0;  
	val1 = val + 1;  

    //- init IO with goodness
	vIO_init(); 


	//- Create a task
	xTaskCreate( (pdTASK_CODE) vTaskFunction_1, (signed char *) "T0", configMINIMAL_STACK_SIZE, 
	             (void *) val1, 1, NULL );

	//- Create a task
	xTaskCreate( (pdTASK_CODE) vTaskFunction_1, (signed char *) "xT0", configMINIMAL_STACK_SIZE, 
	             (void *) val0, 1, NULL );

    //- kick off the scheduler*
	vTaskStartScheduler();

	return 0;
}
/*-----------------------------------------------------------*/


void vTaskFunction_1(void *pvParameters)
{
   uint8_t *val = (unsigned char *) pvParameters; 

   for (;;)  {
      PORTB = *val; 
   }
}


void vIO_init(void) 
{
    //- set PortB as output
	DDRB = 0xFF; 
    PORTB = 0xFF; 
}


