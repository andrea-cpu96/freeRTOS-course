/*
 * rtc.c
 *
 *  Created on: Dec 6, 2024
 *      Author: andre
 */


#include "main.h"
#include "stdio.h"
#include "string.h"


void show_time_date(void)
{

	static char showTime[40];
	static char showDate[40];

	RTC_DateTypeDef rtc_date;
	RTC_TimeTypeDef rtc_time;

	static char *time = showTime;
	static char *date = showDate;

	memset(&rtc_date,0 , sizeof(rtc_date));
	memset(&rtc_time, 0, sizeof(rtc_time));

	// Get the RTC current time
	HAL_RTC_GetTime(&hrtc, &rtc_time, RTC_FORMAT_BIN);
	// Get the RTC current date
	HAL_RTC_GetDate(&hrtc, &rtc_date, RTC_FORMAT_BIN);

	char *format;
	format = ( rtc_time.TimeFormat == RTC_HOURFORMAT12_AM ) ? "AM" : "PM";

	// Display time format : hh:mm:ss [AM/PM]
	sprintf((char *)showTime, "%s:\t%02d:%02d:%02d [%s]", "\n\rCurrent Time&Date", rtc_time.Hours, rtc_time.Minutes, rtc_time.Seconds, format);
	xQueueSend(qprint, &time, portMAX_DELAY);

	// Display date format : date-month-year
	sprintf((char *)showDate, "\t%02d-%02d-%02d\n\r", rtc_date.Month, rtc_date.Date, ( 2000 + rtc_date.Year ));
	xQueueSend(qprint, &date, portMAX_DELAY);
}


void show_time_date_itm(void)
{
	RTC_DateTypeDef rtc_date;
	RTC_TimeTypeDef rtc_time;

	memset(&rtc_date,0,sizeof(rtc_date));
	memset(&rtc_time,0,sizeof(rtc_time));

	/* Get the RTC current Time */
	HAL_RTC_GetTime(&hrtc, &rtc_time, RTC_FORMAT_BIN);
	/* Get the RTC current Date */
	HAL_RTC_GetDate(&hrtc, &rtc_date, RTC_FORMAT_BIN);

	char *format;
	format = (rtc_time.TimeFormat == RTC_HOURFORMAT12_AM) ? "AM" : "PM";


	printf("%02d:%02d:%02d [%s]",rtc_time.Hours, rtc_time.Minutes, rtc_time.Seconds,format);
	printf("\t%02d-%02d-%2d\n\r",rtc_date.Month, rtc_date.Date, 2000 + rtc_date.Year);

}


void rtc_configure_time(RTC_TimeTypeDef *time)
{

	time->TimeFormat = RTC_HOURFORMAT12_AM;

	HAL_RTC_SetTime(&hrtc, time, RTC_FORMAT_BIN);

}


void rtc_configure_date(RTC_DateTypeDef *date)
{

	HAL_RTC_SetDate(&hrtc, date, RTC_FORMAT_BIN);

}


int validate_rtc_information(RTC_TimeTypeDef *time , RTC_DateTypeDef *date)
{
	if(time){
		if( (time->Hours > 12) || (time->Minutes > 59) || (time->Seconds > 59) )
			return 1;
	}

	if(date){
		if( (date->Date > 31) || (date->WeekDay > 7) || (date->Year > 99) || (date->Month > 12) )
			return 1;
	}

	return 0;
}
