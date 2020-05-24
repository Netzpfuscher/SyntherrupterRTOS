/*
 * UD3
 *
 * Copyright (c) 2018 Jens Kerrinnes
 * Copyright (c) 2015 Steve Ward
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/


#include <tasks/tsk_uart.h>
#include "utils/uartstdio.h"


xTaskHandle tsk_uart_TaskHandle;
uint8_t tsk_uart_initVar = 0u;

xSemaphoreHandle tx_Semaphore;


/* ------------------------------------------------------------------------ */
/*
 * Place user included headers, defines and task global data in the
 * below merge region section.
 */
/* `#START USER_INCLUDE SECTION` */
#include "cli_common.h"
#include "tasks/tsk_cli.h"
#include "tasks/tsk_priority.h"


/*
 * The STM32 makes receiving chars into a large circular buffer simple
 * and requires no CPU time. The UART receiver DMA must be setup as CIRCULAR.
 */


/* `#END` */
/* ------------------------------------------------------------------------ */
/*
 * User defined task-local code that is used to process commands, control
 * operations, and/or generrally do stuff to make the taks do something
 * meaningful.
 */
/* `#START USER_TASK_LOCAL_CODE` */



/* `#END` */
/* ------------------------------------------------------------------------ */
/*
 * This is the main procedure that comprises the task.  Place the code required
 * to preform the desired function within the merge regions of the task procedure
 * to add functionality to the task.
 */
void tsk_uart_TaskProc(void *pvParameters) {
	/*
	 * Add and initialize local variables that are allocated on the Task stack
	 * the the section below.
	 */
	/* `#START TASK_VARIABLES` */

	/* `#END` */

	/*
	 * Add the task initialzation code in the below merge region to be included
	 * in the task.
	 */
	/* `#START TASK_INIT_CODE` */


	port_str *port = pvParameters;

	uint8_t buffer[64];

	UARTStdioConfig(0, 115200, SYSTEM_CLOCK);
	UARTEchoSet(false);

	char c;

	/* `#END` */

	for (;;) {
		/* `#START TASK_LOOP_CODE` */

        uint8_t len = xStreamBufferReceive(port->tx, buffer, sizeof(buffer), 0);
        if (len) {
            UARTwrite(buffer, len);
        }




	    while(UARTRxBytesAvail()){
	        c = UARTgetc();
            xStreamBufferSend(port->rx, &c, 1, 0);

	    }
		vTaskDelay(pdMS_TO_TICKS(5));


		/* `#END` */
	}
}
/* ------------------------------------------------------------------------ */
void tsk_uart_Start(port_str *port) {
	/*
	 * Insert task global memeory initialization here. Since the OS does not
	 * initialize ANY global memory, execute the initialization here to make
	 * sure that your task data is properly 
	 */
	/* `#START TASK_GLOBAL_INIT` */

	/* `#END` */
    //UART_2_Start();


	if (tsk_uart_initVar != 1) {

		/*
	 	* Create the task and then leave. When FreeRTOS starts up the scheduler
	 	* will call the task procedure and start execution of the task.
	 	*/
		xTaskCreate(tsk_uart_TaskProc, "UART-Svc", STACK_UART, port, PRIO_UART, &tsk_uart_TaskHandle);
		tsk_uart_initVar = 1;


	}

}
/* ------------------------------------------------------------------------ */
/* ======================================================================== */
/* [] END OF FILE */
