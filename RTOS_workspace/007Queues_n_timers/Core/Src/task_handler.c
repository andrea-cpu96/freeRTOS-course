/*
 * task_handler.c
 *
 *  Created on: Dec 6, 2024
 *      Author: andre
 */

#include "main.h"
#include "string.h"


#define HH_CONFIG 		0
#define MM_CONFIG 		1
#define SS_CONFIG 		2

#define DATE_CONFIG 	0
#define MONTH_CONFIG 	1
#define YEAR_CONFIG 	2
#define DAY_CONFIG 		3


void process_command(command_t *cmd);
int extract_command(command_t *cmd);


const char *msg_inv = "/////// IVALID OPTION ////////\n\r";


void menu_task(void *parameters)
{

	const char *msg_menu = "===============================\n\r"
						   "|              MENU            |\n\r"
						   "===============================\n\r"
						   "LED effect     --->           0\n\r"
						   "Date & Time    --->           1\n\r"
						   "Exit           --->           2\n\r"
						   "Enter your choice here ; ";
	uint32_t cmd_addr;
	command_t *cmd;
	int option;


	while(1)
	{

		xQueueSend(qprint, &msg_menu, portMAX_DELAY);

		// Wait for menu command
		xTaskNotifyWait(0, 0, &cmd_addr, portMAX_DELAY);
		cmd = (command_t *)cmd_addr;

		if(cmd->len == 1)
		{

			// Valid entry

			option = ( cmd->payload[0] - 48 );

			switch(option)
			{

				case 0:

					curr_state = sLedEffect;
					xTaskNotify(handle_led_task, 0, eNoAction);

				break;

				case 1:

					curr_state = sRtcMenu;
					xTaskNotify(handle_rtc_task, 0, eNoAction);

				break;

				case 2:

				break;

				default:

					xQueueSend(qprint, &msg_inv, portMAX_DELAY);

					continue;


			}

		}
		else
		{

			// Invalid entry

			// Notify the user
			xQueueSend(qprint, &msg_inv, portMAX_DELAY);
			continue;

		}

		xTaskNotifyWait(0, 0, &cmd_addr, portMAX_DELAY);

	}

}

void cmd_handler_task(void *parameters)
{

	BaseType_t ret;
	command_t cmd;


	while(1)
	{

		ret = xTaskNotifyWait(0, 0, NULL, portMAX_DELAY);
		if(ret == pdTRUE)
		{

			process_command(&cmd);

		}

	}

}


void print_task(void *parameters)
{

	uint32_t *msg;


	while(1)
	{

		xQueueReceive(qprint, &msg, portMAX_DELAY);
		HAL_UART_Transmit(&huart2, (uint8_t *)msg, strlen((char *)msg), HAL_MAX_DELAY);

	}

}


void led_task(void *parameters)
{

	const char *msg_led = "===============================\n\r"
						  "|          LED effect         |\n\r"
						  "===============================\n\r"
						  "(none, e1, e2, e3, e4\n\r"
						  "Enter your choice here ; ";
	uint32_t cmd_addr;
	command_t *cmd;


	while(1)
	{

		/* Wait for notification (Notify wait) */
		xTaskNotifyWait(0, 0, NULL, portMAX_DELAY);

		/* Print LED menu */
		xQueueSend(qprint, &msg_led, portMAX_DELAY);

		/* Wait for LED command (Notify wait) */
		xTaskNotifyWait(0, 0, &cmd_addr, portMAX_DELAY);
		cmd = (command_t *)cmd_addr;

		if(cmd->len <= 4)
		{

			if(! strcmp((char*)cmd->payload,"none"))
				led_effect_stop();
			else if (! strcmp((char*)cmd->payload,"e1"))
				led_effect(1);
			else if (! strcmp((char*)cmd->payload,"e2"))
				led_effect(2);
			else if (! strcmp((char*)cmd->payload,"e3"))
				led_effect(3);
			else if (! strcmp((char*)cmd->payload,"e4"))
				led_effect(4);
			else
				/* print invalid message */
				xQueueSend(qprint, &msg_inv, portMAX_DELAY);

		}
		else
		{

			/* print invalid message */
			xQueueSend(qprint, & msg_inv, portMAX_DELAY);

			/* update state variable */
			curr_state = sMainMenu;

			/* Notify menu task */
			xTaskNotify(handle_menu_task,0,eNoAction);

		}
	}

}


void rtc_task(void *parameters)
{

	#define HH_CONFIG 		0
	#define MM_CONFIG 		1
	#define SS_CONFIG 		2

	#define DATE_CONFIG 	0
	#define MONTH_CONFIG 	1
	#define YEAR_CONFIG 	2
	#define DAY_CONFIG 		3


	const char* msg_rtc1 = "========================\n\r"
						   "|         RTC          |\n\r"
						   "========================\n\r";

	const char* msg_rtc2 = "Configure Time            ----> 0\n\r"
						   "Configure Date            ----> 1\n\r"
						   "Enable reporting          ----> 2\n\r"
						   "Exit                      ----> 3\n\r"
						   "Enter your choice here : ";


	const char *msg_rtc_hh = "Enter hour(1-12):";
	const char *msg_rtc_mm = "Enter minutes(0-59):";
	const char *msg_rtc_ss = "Enter seconds(0-59):";

	const char *msg_rtc_dd  = "Enter date(1-31):";
	const char *msg_rtc_mo  ="Enter month(1-12):";
	const char *msg_rtc_dow  = "Enter day(1-7 sun:1):";
	const char *msg_rtc_yr  = "Enter year(0-99):";

	const char *msg_conf = "Configuration successful\n\r";
	const char *msg_rtc_report = "Enable time&date reporting(y/n)?: ";


	uint32_t cmd_addr;
	command_t *cmd;

	static int rtc_state = 0;
	int menu_code;

	RTC_TimeTypeDef time;
	RTC_DateTypeDef date;


	while(1){
		/* Notify wait (wait till someone notifies) */
		xTaskNotifyWait(0, 0, NULL, portMAX_DELAY);

		/* Print the menu and show current date and time information */
		xQueueSend(qprint, &msg_rtc1, portMAX_DELAY);
		show_time_date();
		xQueueSend(qprint, &msg_rtc2, portMAX_DELAY);


		while(curr_state != sMainMenu){

			/* Wait for command notification (Notify wait) */
			xTaskNotifyWait(0, 0, &cmd_addr, portMAX_DELAY);
			cmd = (command_t *)cmd_addr;

			switch(curr_state)
			{

				case sRtcMenu:

					/* process RTC menu commands */
					if(cmd->len == 1)
					{

						menu_code = ( cmd->payload[0] - 48 );
						switch(menu_code)
						{

							case 0:

								curr_state = sRtcTimeConfig;
								xQueueSend(qprint, &msg_rtc_hh, portMAX_DELAY);

							break;

							case 1:

								curr_state = sRtcDateConfig;
								xQueueSend(qprint, &msg_rtc_dd, portMAX_DELAY);

							break;

							case 2:

								curr_state = sRtcReport;
								xQueueSend(qprint, &msg_rtc_report, portMAX_DELAY);

							break;

							case 3:

								curr_state = sMainMenu;

							break;

							default:

								curr_state = sMainMenu;
								xQueueSend(qprint, &msg_inv, portMAX_DELAY);

						}

					}
					else
					{

						curr_state = sMainMenu;
						xQueueSend(qprint, &msg_inv, portMAX_DELAY);

					}

				break;

				case sRtcTimeConfig:

					/* get hh, mm, ss infor and configure RTC */
					/* take care of invalid entries */

					switch(rtc_state)
					{

						case HH_CONFIG:

							uint8_t hour = getnumber(cmd->payload , cmd->len);
							time.Hours = hour;
							rtc_state = MM_CONFIG;
							xQueueSend(qprint,&msg_rtc_mm,portMAX_DELAY);

						break;

						case MM_CONFIG:

							uint8_t min = getnumber(cmd->payload , cmd->len);
							time.Minutes = min;
							rtc_state = SS_CONFIG;
							xQueueSend(qprint,&msg_rtc_ss,portMAX_DELAY);

						break;

						case SS_CONFIG:

							uint8_t sec = getnumber(cmd->payload , cmd->len);
							time.Seconds = sec;
							if(!validate_rtc_information(&time,NULL))
							{

								rtc_configure_time(&time);
								xQueueSend(qprint,&msg_conf,portMAX_DELAY);
								show_time_date();
							}
							else
							{

								xQueueSend(qprint,&msg_inv,portMAX_DELAY);

							}

							curr_state = sMainMenu;
							rtc_state = 0;

						break;

					}

				break;

				case sRtcDateConfig:

					/*get date, month, day , year info and configure RTC */

					/*take care of invalid entries */
					switch(rtc_state)
					{

						case DATE_CONFIG:

							uint8_t d = getnumber(cmd->payload , cmd->len);
							date.Date = d;
							rtc_state = MONTH_CONFIG;
							xQueueSend(qprint,&msg_rtc_mo,portMAX_DELAY);
							break;

						case MONTH_CONFIG:

							uint8_t month = getnumber(cmd->payload , cmd->len);
							date.Month = month;
							rtc_state = DAY_CONFIG;
							xQueueSend(qprint,&msg_rtc_dow,portMAX_DELAY);

						break;

						case DAY_CONFIG:

							uint8_t day = getnumber(cmd->payload , cmd->len);
							date.WeekDay = day;
							rtc_state = YEAR_CONFIG;
							xQueueSend(qprint,&msg_rtc_yr,portMAX_DELAY);

						break;

						case YEAR_CONFIG:

							uint8_t year = getnumber(cmd->payload , cmd->len);
							date.Year = year;

							if(!validate_rtc_information(NULL,&date))
							{

								rtc_configure_date(&date);
								xQueueSend(qprint,&msg_conf,portMAX_DELAY);
								show_time_date();

							}
							else
							{

								xQueueSend(qprint,&msg_inv,portMAX_DELAY);

							}

							curr_state = sMainMenu;
							rtc_state = 0;

						break;

					}

				break;

				case sRtcReport:

					/*enable or disable RTC current time reporting over ITM printf */
					if(cmd->len == 1)
					{

						if(cmd->payload[0] == 'y')
						{

							if(xTimerIsTimerActive(rtc_timer) == pdFALSE)
								xTimerStart(rtc_timer,portMAX_DELAY);

						}
						else if(cmd->payload[0] == 'n')
						{

							xTimerStop(rtc_timer,portMAX_DELAY);

						}
						else
						{

							xQueueSend(qprint,&msg_inv,portMAX_DELAY);

						}

					}
					else
					{

					    xQueueSend(qprint,&msg_inv,portMAX_DELAY);

					}

					curr_state = sMainMenu;

				break;

				default:

					curr_state = sMainMenu;

			}// switch end

		} //while end

		/*Notify menu task */
		xTaskNotify(handle_menu_task,0,eNoAction);

	}//while super loop end

}


void process_command(command_t *cmd)
{

	extract_command(cmd);

	switch(curr_state)
	{

		case sMainMenu:
			xTaskNotify(handle_menu_task, (uint32_t)cmd, eSetValueWithOverwrite);
		break;

		case sLedEffect:
			xTaskNotify(handle_led_task, (uint32_t)cmd, eSetValueWithOverwrite);
		break;

		case sRtcMenu:
		case sRtcTimeConfig:
		case sRtcDateConfig:
		case sRtcReport:
			xTaskNotify(handle_rtc_task, (uint32_t)cmd, eSetValueWithOverwrite);
		break;

	}

}


int extract_command(command_t *cmd)
{

	uint8_t item;
	BaseType_t status;

	status = uxQueueMessagesWaiting(qdata);
	if(!status) return -1;

	uint8_t i = 0;

	do
	{

		status = xQueueReceive(qdata, &item, 0);
		if(status == pdTRUE) cmd->payload[i++] = item;

	}while(item != '\r');

	cmd->payload[i-1] = '\0';
	cmd->len = ( i - 1 );

	return 0;

}


uint8_t getnumber(uint8_t *p , int len)
{

	int value ;


	if(len > 1)
	   value =  ( ((p[0]-48) * 10) + (p[1] - 48) );
	else
		value = p[0] - 48;

	return value;

}
