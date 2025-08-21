#include "RTC.h"

// gps may require cold start assistance from
// the RTC. So we put this routine in the GPS file.

uint32_t getUTCfromRTC()
{

	time_t UTC; 		// a time stamp
	
	// cold read the RTC. Did the time stick?
	m5::rtc_time_t TimeStruct;
	m5::rtc_date_t DateStruct;
	tmElements_t tmpTime;	//Time elements structure
	
	M5.Rtc.getTime(&TimeStruct);
	M5.Rtc.getDate(&DateStruct);
	
	Serial.printf("COLD READ RTC time %02d:%02d:%02d\n",
	TimeStruct.hours,
	TimeStruct.minutes,
	TimeStruct.seconds);
	
	Serial.printf("COLD READ RTC date %02d/%02d/%04d\n",
	DateStruct.date,
	DateStruct.month,
	DateStruct.year);

	tmpTime.Hour = TimeStruct.hours;
	tmpTime.Minute = TimeStruct.minutes;
	tmpTime.Second = TimeStruct.seconds;
	
	tmpTime.Day = DateStruct.date;
	tmpTime.Month = DateStruct.month;
	
	// time starts from 1970 ... aka year 0
	tmpTime.Year = DateStruct.year - 1970;

	//thank god I dont have to calc seconds in month 
	UTC =  makeTime(tmpTime);
	
	Serial.printf("calculated UTC from RTC = %d\n", UTC);
	return UTC;
}
//---------------------------------------------------------------------------

void SetRTC(void)
{
		// set RTC		
		m5::rtc_time_t TimeStruct;
		m5::rtc_date_t DateStruct;
		//Serial.println("After TZ tweak");
		//Serial.print("now() = ");
		//Serial.println(now());
		print_date_time();

		// stuff into RTC chip
		TimeStruct.hours = hour();
		TimeStruct.minutes = minute();
		TimeStruct.seconds = gps.time.second();
		
		DateStruct.year = year();
		DateStruct.month = month();
		DateStruct.date = gps.date.day();
		
		M5.Rtc.setTime(&TimeStruct);
		M5.Rtc.setDate(&DateStruct);
}
