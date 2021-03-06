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

#include "tasks/tsk_cli.h"
#include "tasks/tsk_priority.h"
#include "tasks/tsk_overlay.h"
#include "cli_basic.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

xTaskHandle UART_Terminal_TaskHandle;


/* ------------------------------------------------------------------------ */
/*
 * Place user included headers, defines and task global data in the
 * below merge region section.
 */
/* `#START USER_INCLUDE SECTION` */

#include "cli_common.h"
#include "ntshell\ntshell.h"
#include <tasks/tsk_uart.h>


#define UNUSED_VARIABLE(N) \
	do {                   \
		(void)(N);         \
	} while (0)
void *extobjt = 0;

static int write(const char *buf, int cnt, void *extobj);
void initialize_cli(ntshell_t *ptr, port_str *port);
static int nt_callback(const char *text, void *extobj);
/* `#END` */
/* ------------------------------------------------------------------------ */
/*
 * User defined task-local code that is used to process commands, control
 * operations, and/or generrally do stuff to make the taks do something
 * meaningful.
 */
/* `#START USER_TASK_LOCAL_CODE` */

void initialize_cli(ntshell_t *ptr, port_str *port) {
    ntshell_init(ptr, write, nt_callback, port);
	ntshell_set_prompt(ptr, ":>");
	ntshell_show_promt(ptr);
}

static int write(const char *buf, int cnt, void *extobj) {
    port_str *port = extobj;
    xStreamBufferSend(port->tx,buf, cnt,portMAX_DELAY);
	return cnt;
}

static int nt_callback(const char *text, void *extobj) {
	port_str *port = extobj;
	nt_interpret((char*)text, port);
	return 0;
}


uint8_t handle_terminal(ntshell_t *ptr, port_str *port) {
	char c;
	if (xStreamBufferReceive(port->rx, &c,1, portMAX_DELAY)) {
		if (xSemaphoreTake(port->term_block, portMAX_DELAY)) {
			ntshell_execute_nb(ptr, c);
			xSemaphoreGive(port->term_block);
		}
	}
	return 0;
}

/* `#END` */
/* ------------------------------------------------------------------------ */
/*
 * This is the main procedure that comprises the task.  Place the code required
 * to preform the desired function within the merge regions of the task procedure
 * to add functionality to the task.
 */


void tsk_cli_TaskProc(void *pvParameters) {
	/*
	 * Add and initialize local variables that are allocated on the Task stack
	 * the the section below.
	 */
	/* `#START TASK_VARIABLES` */



	ntshell_t ntsh;
	port_str port;

    port.type = PORT_TYPE_SERIAL;
    port.term_mode = PORT_TERM_VT100;
    port.term_block = xSemaphoreCreateBinary();
    port.rx = xStreamBufferCreate(STREAMBUFFER_RX_SIZE,1);
    port.tx = xStreamBufferCreate(STREAMBUFFER_TX_SIZE,1);
    xSemaphoreGive(port.term_block);

    tsk_uart_Start(&port);
    tsk_overlay_Start(&port);

    init_config();

    eeprom_load(&port);


    /* `#END` */

	/*
	 * Add the task initialzation code in the below merge region to be included
	 * in the task.
	 */
	/* `#START TASK_INIT_CODE` */

	initialize_cli(&ntsh, &port);

    /* `#END` */

	for (;;) {
		/* `#START TASK_LOOP_CODE` */
        handle_terminal(&ntsh,&port);

        /* `#END` */
	}
}
/* ------------------------------------------------------------------------ */
void tsk_cli_Start(void) {
/*
	 * Insert task global memeory initialization here. Since the OS does not
	 * initialize ANY global memory, execute the initialization here to make
	 * sure that your task data is properly 
	 */
/* `#START TASK_GLOBAL_INIT` */

/* `#END` */


	/*
	* Create the task and then leave. When FreeRTOS starts up the scheduler
	* will call the task procedure and start execution of the task.
	*/
	xTaskCreate(tsk_cli_TaskProc, "UART-CLI", STACK_TERMINAL, NULL, PRIO_TERMINAL, &UART_Terminal_TaskHandle);

}
/* ------------------------------------------------------------------------ */
/* ======================================================================== */
/* [] END OF FILE */
