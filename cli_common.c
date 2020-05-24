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

#include <cli_common.h>

#include "ntshell/ntshell.h"
#include "ntshell/ntlibc.h"
#include "telemetry.h"
#include <stdint.h>

#include "tasks/tsk_priority.h"
#include "tasks/tsk_uart.h"
#include "tasks/tsk_cli.h"
#include "tasks/tsk_overlay.h"
#include "helper/teslaterm.h"
#include "helper/printf.h"
#include "math.h"

#include <string.h>



#define UNUSED_VARIABLE(N) \
	do {                   \
		(void)(N);         \
	} while (0)
        

	
typedef struct {
	const char *text;
	uint8_t (*commandFunction)(char *commandline, port_str *ptr);
	const char *help;
} command_entry;

uint8_t command_help(char *commandline, port_str *ptr);
uint8_t command_get(char *commandline, port_str *ptr);
uint8_t command_set(char *commandline, port_str *ptr);
uint8_t command_eprom(char *commandline, port_str *ptr);
uint8_t command_status(char *commandline, port_str *ptr);
uint8_t command_tasks(char *commandline, port_str *ptr);
uint8_t command_load_default(char *commandline, port_str *ptr);
uint8_t command_tterm(char *commandline, port_str *ptr);
uint8_t command_reset(char *commandline, port_str *ptr);
uint8_t command_config_get(char *commandline, port_str *ptr);
uint8_t command_signals(char *commandline, port_str *ptr);
uint8_t command_shutdown(char *commandline, port_str *ptr);

uint8_t callback_ConfigFunction(parameter_entry * params, uint8_t index, port_str *ptr);
uint8_t callback_DefaultFunction(parameter_entry * params, uint8_t index, port_str *ptr);
uint8_t callback_ComFunction(parameter_entry * params, uint8_t index, port_str *ptr);




/*****************************************************************************
* Initializes parameters with default values
******************************************************************************/
void init_config(){


}


// clang-format off

/*****************************************************************************
* Parameter struct
******************************************************************************/
int32_t test0;
int32_t test1;
int32_t test2;

parameter_entry confparam[] = {
    //       Parameter Type ,"Text   "         , Value ptr                     ,Min    ,Max    ,Div             ,Callback Function           ,Help text
    ADD_PARAM(PARAM_DEFAULT ,"speed"           , test0                         ,-1000  ,1000   ,0               ,NULL     					 ,"Motor Speed")
    ADD_PARAM(PARAM_CONFIG  ,"ctrl_mode"       , test1                         ,0      ,3      ,0               ,callback_ConfigFunction     ,"Ctrl mode [1] voltage [2] Speed [3] Torque")
    ADD_PARAM(PARAM_CONFIG  ,"curr_max"        , test2                         ,0      ,32000  ,0               ,callback_ConfigFunction     ,"Maximum phase current [A]")  //100A
};

/*****************************************************************************
* Command struct
******************************************************************************/
command_entry commands[] = {
    ADD_COMMAND("cls"		    ,command_cls            ,"Clear screen")
    ADD_COMMAND("eeprom"	    ,command_eprom          ,"Save/Load config [load/save]")
	ADD_COMMAND("get"		    ,command_get            ,"Usage get [param]")
    ADD_COMMAND("help"          ,command_help           ,"This text")
    ADD_COMMAND("load_default"  ,command_load_default   ,"Loads the default parameters")
    ADD_COMMAND("reset"         ,command_reset          ,"Resets the uC")
	ADD_COMMAND("set"		    ,command_set            ,"Usage set [param] [value]")
    ADD_COMMAND("status"	    ,command_status         ,"Displays status")
    ADD_COMMAND("tasks"	        ,command_tasks          ,"Show running Tasks")
    ADD_COMMAND("tterm"	        ,command_tterm          ,"Changes terminal mode")
    ADD_COMMAND("config_get"    ,command_config_get     ,"Internal use")
    ADD_COMMAND("signals"       ,command_signals        ,"For debugging")
	ADD_COMMAND("shutdown"      ,command_shutdown       ,"Shutdown")
};

uint8_t eeprom_load(port_str *ptr){
    return EEPROM_read_conf(confparam, PARAM_SIZE(confparam) ,0,ptr);
}



// clang-format on





extern uint8_t enable;

/*****************************************************************************
* Callback if a configuration relevant parameter is changed
******************************************************************************/
uint8_t callback_ConfigFunction(parameter_entry * params, uint8_t index, port_str *ptr){


	return 1;
}

/*****************************************************************************
* Callback if a communication relevant parameter is changed
******************************************************************************/
uint8_t callback_ComFunction(parameter_entry * params, uint8_t index, port_str *ptr){


	return 1;
}

/*****************************************************************************
* Default function if a parameter is changes (not used)
******************************************************************************/
uint8_t callback_DefaultFunction(parameter_entry * params, uint8_t index, port_str *ptr){

	return 1;
}



/*****************************************************************************
* Sends the configuration to teslaterm
******************************************************************************/
uint8_t command_config_get(char *commandline, port_str *ptr){
    char buffer[80];
	for (uint8_t current_parameter = 0; current_parameter < sizeof(confparam) / sizeof(parameter_entry); current_parameter++) {

		print_param_buffer(buffer, confparam, current_parameter);
		send_config(buffer,confparam[current_parameter].help, ptr);

    }
    send_config("NULL","NULL", ptr);
    return 1; 
}


uint8_t command_shutdown(char *commandline, port_str *ptr){

	return 1;
}


/*****************************************************************************
* Prints the task list needs to be enabled in FreeRTOSConfig.h
* only use it for debugging reasons
******************************************************************************/
uint8_t command_tasks(char *commandline, port_str *ptr) {
    #if configUSE_STATS_FORMATTING_FUNCTIONS && configUSE_TRACE_FACILITY && configGENERATE_RUN_TIME_STATS
        char * buff = pvPortMalloc(sizeof(char)* 40 * uxTaskGetNumberOfTasks());
        if(buff == NULL){
            SEND_CONST_STRING("Malloc failed\r\n", ptr);
            return 0;
        }
        
	    SEND_CONST_STRING("**********************************************\n\r", ptr);
	    SEND_CONST_STRING("Task            State   Prio    Stack    Num\n\r", ptr);
	    SEND_CONST_STRING("**********************************************\n\r", ptr);
	    vTaskList(buff);
	    send_string(buff, ptr);
	    SEND_CONST_STRING("**********************************************\n\r\n\r", ptr);
        SEND_CONST_STRING("**********************************************\n\r", ptr);
	    SEND_CONST_STRING("Task            Abs time        % time\n\r", ptr);
	    SEND_CONST_STRING("**********************************************\n\r", ptr);
        vTaskGetRunTimeStats( buff );
        send_string(buff, ptr);
        SEND_CONST_STRING("**********************************************\n\r\r\n", ptr);
        sprintf(buff, "Free heap: %d\r\n",xPortGetFreeHeapSize());
        send_string(buff,ptr);
        vPortFree(buff);
        return 0;
    #endif
    
    #if !configUSE_STATS_FORMATTING_FUNCTIONS && !configUSE_TRACE_FACILITY
	    SEND_CONST_STRING("Taskinfo not active, activate it in FreeRTOSConfig.h\n\r", ptr);
	    char buff[30];
        sprintf(buff, "Free heap: %d\r\n",xPortGetFreeHeapSize());
        send_string(buff,ptr);
        return 0;
	#endif
    return 0;
	
}

/*****************************************************************************
* Get a value from a parameter or print all parameters
******************************************************************************/
uint8_t command_get(char *commandline, port_str *ptr) {
	SKIP_SPACE(commandline);
    
	if (*commandline == 0 || commandline == 0) //no param --> show help text
	{
		print_param_help(confparam, PARAM_SIZE(confparam), ptr);
		return 1;
	}

	for (uint8_t current_parameter = 0; current_parameter < sizeof(confparam) / sizeof(parameter_entry); current_parameter++) {
		if (ntlibc_stricmp(commandline, confparam[current_parameter].name) == 0) {
			//Parameter found:
			print_param(confparam,current_parameter,ptr);
			return 1;
		}
	}
	Term_Color_Red(ptr);
	SEND_CONST_STRING("E: unknown param\r\n", ptr);
	Term_Color_White(ptr);
	return 0;
}

/*****************************************************************************
* Set a new value to a parameter
******************************************************************************/
uint8_t command_set(char *commandline, port_str *ptr) {
	SKIP_SPACE(commandline);
	char *param_value;

	if (commandline == NULL) {
		//if (!port)
		Term_Color_Red(ptr);
		SEND_CONST_STRING("E: no name\r\n", ptr);
		Term_Color_White(ptr);
		return 0;
	}

	param_value = ntlibc_strchr(commandline, ' ');
	if (param_value == NULL) {
		Term_Color_Red(ptr);
		SEND_CONST_STRING("E: no value\r\n", ptr);
		Term_Color_White(ptr);
		return 0;
	}

	*param_value = 0;
	param_value++;

	if (*param_value == '\0') {
		Term_Color_Red(ptr);
		SEND_CONST_STRING("E: no val\r\n", ptr);
		Term_Color_White(ptr);
		return 0;
	}

	for (uint8_t current_parameter = 0; current_parameter < sizeof(confparam) / sizeof(parameter_entry); current_parameter++) {
		if (ntlibc_stricmp(commandline, confparam[current_parameter].name) == 0) {
			//parameter name found:

			if (updateDefaultFunction(confparam, param_value,current_parameter, ptr)){
                if(confparam[current_parameter].callback_function){
                    if (confparam[current_parameter].callback_function(confparam, current_parameter, ptr)){
                        Term_Color_Green(ptr);
                        SEND_CONST_STRING("OK\r\n", ptr);
                        Term_Color_White(ptr);
                        return 1;
                    }else{
                        Term_Color_Red(ptr);
                        SEND_CONST_STRING("ERROR: Callback\r\n", ptr);
                        Term_Color_White(ptr);
                        return 1;
                    }
                }else{
                    Term_Color_Green(ptr);
                    SEND_CONST_STRING("OK\r\n", ptr);
                    Term_Color_White(ptr);
                    return 1;
                }
			} else {
				Term_Color_Red(ptr);
				SEND_CONST_STRING("NOK\r\n", ptr);
				Term_Color_White(ptr);
				return 1;
			}
		}
	}
	Term_Color_Red(ptr);
	SEND_CONST_STRING("E: unknown param\r\n", ptr);
	Term_Color_White(ptr);
	return 0;
}



#define ADDR_FLASH_PAGE_63    ((uint32_t)0x0800FC00) /* Base @ of Page 63, 1 Kbytes */

/*****************************************************************************
* Saves confparams to eeprom
******************************************************************************/
uint8_t command_eprom(char *commandline, port_str *ptr) {
	SKIP_SPACE(commandline);
    CHECK_NULL(commandline);
    
	if (ntlibc_stricmp(commandline, "save") == 0) {

	    EEPROM_write_conf(confparam, PARAM_SIZE(confparam),0, ptr);

		return 0;
	}
	if (ntlibc_stricmp(commandline, "load") == 0) {

		EEPROM_read_conf(confparam, PARAM_SIZE(confparam) ,0,ptr);
        

		return 0;
	}
	HELP_TEXT("Usage: eprom [load|save]\r\n");
}

/*****************************************************************************
* Prints the help text
******************************************************************************/
uint8_t command_help(char *commandline, port_str *ptr) {
	UNUSED_VARIABLE(commandline);
	SEND_CONST_STRING("\r\nCommands:\r\n", ptr);
	for (uint8_t current_command = 0; current_command < (sizeof(commands) / sizeof(command_entry)); current_command++) {
		SEND_CONST_STRING("\t", ptr);
		Term_Color_Cyan(ptr);
		send_string((char *)commands[current_command].text, ptr);
		Term_Color_White(ptr);
		if (ntlibc_strlen(commands[current_command].text) > 7) {
			SEND_CONST_STRING("\t-> ", ptr);
		} else {
			SEND_CONST_STRING("\t\t-> ", ptr);
		}
		send_string((char *)commands[current_command].help, ptr);
		SEND_CONST_STRING("\r\n", ptr);
	}

	SEND_CONST_STRING("\r\nParameters:\r\n", ptr);
	for (uint8_t current_command = 0; current_command < sizeof(confparam) / sizeof(parameter_entry); current_command++) {
		SEND_CONST_STRING("\t", ptr);
		Term_Color_Cyan(ptr);
		send_string((char *)confparam[current_command].name, ptr);
		Term_Color_White(ptr);
		if (ntlibc_strlen(confparam[current_command].name) > 7) {
			SEND_CONST_STRING("\t-> ", ptr);
		} else {
			SEND_CONST_STRING("\t\t-> ", ptr);
		}

		send_string((char *)confparam[current_command].help, ptr);
		SEND_CONST_STRING("\r\n", ptr);
	}

	SEND_CONST_STRING("\r\nConfiguration:\r\n", ptr);
	for (uint8_t current_command = 0; current_command < sizeof(confparam) / sizeof(parameter_entry); current_command++) {
		SEND_CONST_STRING("\t", ptr);
		Term_Color_Cyan(ptr);
		send_string((char *)confparam[current_command].name, ptr);
		Term_Color_White(ptr);
		if (ntlibc_strlen(confparam[current_command].name) > 7) {
			SEND_CONST_STRING("\t-> ", ptr);
		} else {
			SEND_CONST_STRING("\t\t-> ", ptr);
		}

		send_string((char *)confparam[current_command].help, ptr);
		SEND_CONST_STRING("\r\n", ptr);
	}

	return 0;
}




/*****************************************************************************
* Loads the default parametes out of flash
******************************************************************************/
uint8_t command_load_default(char *commandline, port_str *ptr) {
    SEND_CONST_STRING("Default parameters loaded\r\n", ptr);
    init_config();
    return 1;
}

/*****************************************************************************
* Reset of the controller
******************************************************************************/
uint8_t command_reset(char *commandline, port_str *ptr){

    return 1;
}




/*****************************************************************************
* Helper function for spawning the overlay task
******************************************************************************/
void start_overlay_task(port_str *ptr){
	telemetry = 1;
}


/*****************************************************************************
* Helper function for killing the overlay task
******************************************************************************/
void stop_overlay_task(port_str *ptr){
	telemetry = 0;
}

/*****************************************************************************
* 
******************************************************************************/
uint8_t command_status(char *commandline, port_str *ptr) {
    SKIP_SPACE(commandline);
    CHECK_NULL(commandline);
	

	if (ntlibc_stricmp(commandline, "start") == 0) {
		start_overlay_task(ptr);
        return 1;
	}
	if (ntlibc_stricmp(commandline, "stop") == 0) {
		stop_overlay_task(ptr);
        return 1;
	}

	HELP_TEXT("Usage: status [start|stop]\r\n");
}

/*****************************************************************************
* Initializes the teslaterm telemetry
* Spawns the overlay task for telemetry stream generation
******************************************************************************/
void init_tt(uint8_t with_chart, port_str *ptr){
    send_gauge_config(0, GAUGE0_MIN, GAUGE0_MAX, GAUGE0_NAME, ptr);
    send_gauge_config(1, GAUGE1_MIN, GAUGE1_MAX, GAUGE1_NAME, ptr);
    send_gauge_config(2, GAUGE2_MIN, GAUGE2_MAX, GAUGE2_NAME, ptr);
    send_gauge_config(3, GAUGE3_MIN, GAUGE3_MAX, GAUGE3_NAME, ptr);
    send_gauge_config(4, GAUGE4_MIN, GAUGE4_MAX, GAUGE4_NAME, ptr);
    send_gauge_config(5, GAUGE5_MIN, GAUGE5_MAX, GAUGE5_NAME, ptr);
    send_gauge_config(6, GAUGE6_MIN, GAUGE6_MAX, GAUGE6_NAME, ptr);
    
    if(with_chart==pdTRUE){
        send_chart_config(0, CHART0_MIN, CHART0_MAX, CHART0_OFFSET, CHART0_UNIT, CHART0_NAME, ptr);
        send_chart_config(1, CHART1_MIN, CHART1_MAX, CHART1_OFFSET, CHART1_UNIT, CHART1_NAME, ptr);
        send_chart_config(2, CHART2_MIN, CHART2_MAX, CHART2_OFFSET, CHART2_UNIT, CHART2_NAME, ptr);
        send_chart_config(3, CHART3_MIN, CHART3_MAX, CHART3_OFFSET, CHART3_UNIT, CHART3_NAME, ptr);
    }
    start_overlay_task(ptr);
}

uint8_t command_tterm(char *commandline, port_str *ptr){
    SKIP_SPACE(commandline);
    CHECK_NULL(commandline);
    
	if (ntlibc_stricmp(commandline, "start") == 0) {
        ptr->term_mode = PORT_TERM_TT;
        init_tt(pdTRUE,ptr);
        return 1;

    }
	if (ntlibc_stricmp(commandline, "stop") == 0) {
        ptr->term_mode = PORT_TERM_VT100;
        stop_overlay_task(ptr);
        return 1;
	} 
    
    HELP_TEXT("Usage: tterm [start|stop]\r\n");
}

/*****************************************************************************
* Clears the terminal screen and displays the logo
******************************************************************************/
uint8_t command_cls(char *commandline, port_str *ptr) {
    //tsk_overlay_chart_start();
	Term_Erase_Screen(ptr);

    SEND_CONST_STRING("\r\n\r\n",ptr);
	return 1;
}

/*****************************************************************************
* Signal debugging
******************************************************************************/

void send_signal_state(uint8_t signal, uint8_t inverted, port_str *ptr){
    if(inverted) signal = !signal; 
    if(signal){
        Term_Color_Red(ptr);
        SEND_CONST_STRING("true \r\n",ptr);
        Term_Color_White(ptr);  
    }else{
        Term_Color_Green(ptr);
        SEND_CONST_STRING("false\r\n",ptr);
        Term_Color_White(ptr);
    }
}
void send_signal_state_wo(uint8_t signal, uint8_t inverted, port_str *ptr){
    if(inverted) signal = !signal; 
    if(signal){
        Term_Color_Red(ptr);
        SEND_CONST_STRING("true ",ptr);
        Term_Color_White(ptr);  
    }else{
        Term_Color_Green(ptr);
        SEND_CONST_STRING("false",ptr);
        Term_Color_White(ptr);
    }
}


uint8_t command_signals(char *commandline, port_str *ptr) {
    SKIP_SPACE(commandline);
    Term_Disable_Cursor(ptr);
    Term_Erase_Screen(ptr);
    char buf[60];

    while(getch(ptr,100 /portTICK_RATE_MS) != 'q'){

		Term_Move_Cursor(1,1,ptr);
		SEND_CONST_STRING("Signal state (q for quit):\r\n", ptr);
		SEND_CONST_STRING("**************************\r\n", ptr);


    }
    Term_Enable_Cursor(ptr);
	return 1;
}


/*****************************************************************************
* Interprets the Input String
******************************************************************************/
void nt_interpret(char *text, port_str *ptr) {
    int8_t max_len=-1;
    int8_t max_index=-1;
    
    char* p_text=text;
    while(*p_text){
        if(*p_text==' ') break;
        *p_text=ntlibc_tolower(*p_text);
        p_text++;
    }
    
	for (uint8_t current_command = 0; current_command < (sizeof(commands) / sizeof(command_entry)); current_command++) {
        uint8_t text_len = ntlibc_strlen(commands[current_command].text);
		if (memcmp(text, commands[current_command].text, text_len) == 0) {
            if(text_len > max_len){
			    max_len = text_len;
                max_index = current_command;
            }
		}
	}
    
    if(max_index != -1){
       commands[max_index].commandFunction((char *)strchr(text, ' '), ptr);
       return;
        
    }
 
	if (*text) {
		Term_Color_Red(ptr);
		SEND_CONST_STRING("Unknown Command: ", ptr);
		send_string(text, ptr);
		SEND_CONST_STRING("\r\n", ptr);
		Term_Color_White(ptr);
	}
}


