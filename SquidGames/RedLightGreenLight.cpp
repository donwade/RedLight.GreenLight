/*****************************************
  ESP32 GPS VKEL 9600 Bds
This version is for T22_v01 20190612 board
As the power management chipset changed, it
require the axp20x library that can be found
https://github.com/lewisxhe/AXP202X_Library
You must import it as gzip in sketch submenu
in Arduino IDE
This way, it is required to power up the GPS
module, before trying to read it.

Also get TinyGPS++ library from: 
https://github.com/mikalhart/TinyGPSPlus
******************************************/


// Bluetooth for Arduino (C) 2020 Phil Schatzmann
// https://github.com/pschatzmann/ESP32-A2DP.git
// https://github.com/pschatzmann/arduino-audio-tools.git

#define LINE Serial.printf("%s:%d\n", __FUNCTION__, __LINE__)


#include <TinyGPS++.h>
#include <M5Unified.h>

#include "m5Core2-only.h"
#include "watchdogs.h"
#include "viewController.h"

#include <iostream>
#include <cstring>
#include <string>

#include <SPI.h>
#include <Wire.h>  

//#include "BluetoothA2DPSource.h"
#include <math.h> 

#include <assert.h>
#include "lookup.h"   //table of targets

#include <mutex>
#include "locate.h"
#include "viewController.h"

#define LAT_MIN  44.
#define LAT_MAX  46.

#define LNG_MAX -74.
#define LNG_MIN -76.


#define BUILTIN_LED 4  // TIP t-beam
extern void smartDelay(unsigned long ms);

extern TinyGPSPlus gps;

#define MIN_SPEED_KPH 9 // dont do compass if speed lower than this.
//------------------------------------------------------------------

gpsLocation iLocation;
gpsMisc     iMisc;

static const char *qual[] = {
	"EXEL",
	"GOOD",
	"POOR",
	"SICK"
};


//-----------------------------------------------------------------

//#define SIMULATOR

bool getData(void)
{
#ifdef SIMULATOR
	String cppStr;
	char *cstr;
	char charo[100];
	char notUsed[30];
	char notUsed1[30];
	char clat[20];
	char clng[20];
	char cSpeed[5];
	char  cDeg[6];

	
	while(!Serial.available()) { delay(10);}

	// or many lines get read
	cppStr = Serial.readStringUntil('\n');  

	// convert 'String' to C-String
	cstr = new char [cppStr.length()+1];
	std::strcpy (cstr, cppStr.c_str());

	//18:31:02 @ +45.2944592 -75.8636137 ^  14 kph dir 110 ESE

	// sscanf  %f not available on embedded systems without hard work 	

	sscanf((char*) cstr, "%s %s %s %s %s %s %s %s %s %s\n", 
						 &notUsed, &notUsed, 
						 &clat, &clng, 
						 &notUsed,
						 &cSpeed,
						 &notUsed,
						 &notUsed1,
						 &cDeg,
						 &notUsed
						 );


	iLocation.lat = atof(clat);
	iLocation.lng = atof(clng);

	iMisc.Kmph = atof(cSpeed);
	iMisc.course = atof( cDeg);
	iMisc.cardinal = gps.cardinal(iMisc.course);

	if (cstr) delete [] cstr;

#if 0
	Serial.printf("\n\n-------start-----\n");
	Serial.println(cstr);
	Serial.printf("lat=%10.7f \n", iLocation.lat);
	Serial.printf("lng=%10.7f \n", iLocation.lng);

	Serial.printf(" k/c/c %6.4f %6.4f %s\n", 
			iMisc.Kmph, 
			iMisc.course, 
			iMisc.cardinal);
	
	Serial.println(cDeg);
	Serial.printf("-------done-----\n");
#endif
	return true;

#else

	double Tlat, Tlng;
	Tlat = gps.location.lat();
	Tlng = gps.location.lng();
	
	if (Tlat < LAT_MIN || Tlat > LAT_MAX)
	{
		Serial.printf("%s:%d GPS bad LAT= %11.8f < %11.8f < %11.8f\n",
					__FUNCTION__,__LINE__, LAT_MIN, Tlat, LAT_MAX);
		return false;
	}
	
	if ( Tlng < LNG_MIN || Tlng > LNG_MAX )
	{
		Serial.printf("%s:%d GPS bad LON= %11.8f < %11.8f < %11.8f  \n",
					__FUNCTION__,__LINE__, LNG_MIN, Tlng, LNG_MAX);
		return false;
	}
	
	iLocation.lat = Tlat;
	iLocation.lng = Tlng;
	iMisc.hour = gps.time.hour();
	iMisc.minute = gps.time.minute();
	iMisc.second = gps.time.second();
	iMisc.Kmph = gps.speed.kmph();
	iMisc.qual = gps.hdop.hdop();
	iMisc.course = gps.course.deg();

	/*
		HDOP < 2: Excellent accuracy, suitable for critical applications. 
		2 < HDOP < 5: Good accuracy, sufficient for most tasks. 
		HDOP > 5: Poor accuracy, may require alternative or redundant systems. 
		HDOP > 10: Considered poor and indicates a low accuracy GPS fix. 
	*/
	if (iMisc.qual <= 2.0)
		iMisc.cQuality = qual[0];
	else if (iMisc.qual <= 5.0)
		iMisc.cQuality = qual[1];
	else if (iMisc.qual <= 10.0)
		iMisc.cQuality = qual[2];
	else
		iMisc.cQuality = qual[3];
		
	return true;
	
#endif
}


gpsLocation gpsAverage;

#define GPS_SAMPLE_RATE 250  //mS
#define GPS_SAMPLE_SIZE 12

gpsLocation samples [ GPS_SAMPLE_SIZE ];
uint8_t sIndex;

char BT_SSID[17] = "== none ====";
//---------------------------------------------------------

extern void smartDelay(unsigned long ms);

bool cardinalSin(int16_t windowCenter, uint8_t width, int16_t test)
{
	int16_t LHS, RHS, TEST;
	bool bInside;
	
	// Serial.printf("center=%d, width=%d, test=%d\n", windowCenter, width, test);
	
	TEST = (test + 360) % 360;
	windowCenter = (windowCenter + 360) % 360;

	LHS = (windowCenter - width);
	RHS = (windowCenter + width);
	// Serial.printf("Window RAW   LHS=%3d < X < RHS=%3d\n", LHS, RHS);
	
	// convert any coord that went negative to all positive
	LHS = ( LHS + 360) % 360;
	RHS = ( RHS + 360) % 360;

	// Serial.printf("Window RANGE LHS=%3d < X < RHS=%3d\n", LHS, RHS);

	if ( LHS < RHS )
	{
		//classic no adj needed.
	}
	else
	{
		// the window straddles about the 0 point somewhere
		// RHS will be low value, LHS hi value.
		// Promote RHS into unmodulo 360
		RHS += 360; 
		
		// Serial.printf("    RHS TWEAKED LHS=%3d < X < RHS=%3d\n", LHS, RHS);

		// where does the test point sit on the straddle line?
		// if the test sits in the wrapped area (0...low) then promote it
		
		if (TEST + 360 <= 360) // adjustment past the RHS is illegal
		{	
			TEST += 360;
			// Serial.printf("    TEST TWEAKED LHS=%3d < X < RHS=%3d\n", LHS, RHS);
		}
	}

	bInside = ( LHS <= TEST && TEST <= RHS );
	
	// Serial.printf("Window  TEST LHS=%3d < %3d < RHS=%3d\n", LHS, TEST, RHS);
	// Serial.printf( "%d/%d you are %s", test, TEST, bInside ? "INSIDE" : "NOT INSIDE");
	// Serial.println("\n");
	
	return bInside;
	
}

//------------------------------------------------------------------
void getOldestSample(gpsLocation *result)
{
	result->lat = samples[sIndex].lat;
	result->lng = samples[sIndex].lng;
}

//------------------------------------------------------------------
void calcGPSaverage(void)
{
	gpsLocation result;
	
	result.lat = 0.0;
	result.lng = 0.0;
	
	for (int i= 0; i < GPS_SAMPLE_SIZE; i++)
	{
		result.lat += samples[i].lat;
		result.lng += samples[i].lng;
	}
	result.lat /= float(GPS_SAMPLE_SIZE);
	result.lng /= float(GPS_SAMPLE_SIZE);
	gpsAverage = result;
}
//------------------------------------------------------------------

u_int8_t char_height = 0;

gpsLocation cameraLocation;
gpsLocation endLocation;

static int veh_course;
static const char *veh_cardinal = "???";

extern void * reportingMode(BUTTON_EVENT some_key);
extern void * learningMode(BUTTON_EVENT some_key);

//------------------------------------------------------
typedef  void* (*pStateFunction)(BUTTON_EVENT);


pStateFunction stateMachines[] =
{
	learningMode,
	reportingMode
};

void stateDisplay(BUTTON_EVENT some_key)
{
	static volatile pStateFunction lastCall = reportingMode;
	//static volatile pStateFunction lastCall = learningMode;
	pStateFunction nowCall;

	kickDog();
	nowCall = (pStateFunction)lastCall(some_key);

	if (nowCall != lastCall)
	{

		buttonQueue.enqueue(BUTTON_INIT);
		xSemaphoreGive(keyCountingSemaphore);

		Serial.printf("enquing BUTTON_INIT (%d)\n", BUTTON_INIT);

		lastCall = nowCall;
	}
	
}


//---------------------------------------------------------

void gpsGetDataTask(void *not_used)
{
	char msg[30];
	unsigned long startProileTime;
	unsigned long difftime;
	static uint8_t oneIn4;
	static gpsLocation oldLocation;
	
	static unsigned long lastProfileTime; 


	smartDelay(GPS_SAMPLE_RATE);
	
	if (!getData())
	{
		return;
	}	
	// update rolling history

	samples[sIndex].lat = iLocation.lat;
	samples[sIndex].lng = iLocation.lng;

	// sIndex is left pointing to NEXT position to write to on the next pass
	// therefore sIndex points to oldest entry by time

	if (++sIndex == GPS_SAMPLE_SIZE) sIndex = 0;

	calcGPSaverage();
	
	double delta_dist = gps.distanceBetween(iLocation.lat, iLocation.lng, oldLocation.lat, oldLocation.lng );
	oldLocation = iLocation;
	
	xprintf(7, "diff=%7.4f s=%d", delta_dist, gps.satellites.value());
	
	// get direction only if going fast enough
	// otherwise it points all over the place
	
	if (iMisc.Kmph > MIN_SPEED_KPH )
	{
		gpsLocation oldest;
		getOldestSample(&oldest);
		veh_course = (int)gps.courseTo(oldest.lat, oldest.lng, iLocation.lat, iLocation.lng );
		veh_cardinal = gps.cardinal(veh_course);
	}

#ifdef CHATTY		
	Serial.printf("%2d:%02d:%02d @ %+9.7f %+9.7f ^ %3d kph dir %3d %s\n", 
			iMisc.hour,iMisc.minute,iMisc.second,
			iLocation.lat, iLocation.lng,
			(int)iMisc.Kmph, (int)iMisc.course, gps.cardinal(iMisc.course)
			);
#endif

}

//---------------------------------------------------------

void runDisplayTask(void *not_used)
{
	if (iMisc.Kmph > MIN_SPEED_KPH )
	{
		gpsLocation oldest;
		getOldestSample(&oldest);
		veh_course = (int)gps.courseTo(oldest.lat, oldest.lng, iLocation.lat, iLocation.lng );
		veh_cardinal = gps.cardinal(veh_course);
	}

#ifdef CHATTY
	Serial.printf("%2d:%02d:%02d @ %+9.7f %+9.7f ^ %3d kph dir %3d %s\n", 
			iMisc.hour,iMisc.minute,iMisc.second,
			iLocation.lat, iLocation.lng,
			(int)iMisc.Kmph, (int)iMisc.course, gps.cardinal(iMisc.course)
			);
#endif
	BUTTON_EVENT abutton;

	while (xSemaphoreTake( keyCountingSemaphore, pdMS_TO_TICKS(250) ) == pdTRUE)
	{
		abutton = buttonQueue.dequeue();
		stateDisplay(abutton);
	}
	
	stateDisplay(DISPLAY_REFRESH);
}


//-----------------------------------------------------------
// BLUETOOTH

#define left_freq  13.
#define right_freq 40.

uint32_t callback_ctr =0;
uint32_t tick_ctr = 0;

/*
#if 0  // TESTING set cardinal view range
	#define STEP 20
	for (int x = 0; x < 360; x +=STEP)
	{
		Serial.println(cardinalSin(x, 20, x + 21));  // test for just outside RHS
	}
	Serial.println("---------");
	
	for (int x = 0; x < 360; x +=STEP)
	{
		Serial.println(cardinalSin(x, 20, x - 21));  // test for just outside LHS
	}
	Serial.println("---------");

	for (int x = 0; x < 360; x +=STEP)
	{
		Serial.println(cardinalSin(x, 20, x - 19));  // test for just inside LHS
	}
	Serial.println("---------");

	for (int x = 0; x < 360; x +=STEP)
	{
		Serial.println(cardinalSin(x, 20, x + 19));  // test for just inside RHS
	}
	Serial.println("---------");

	for (int x = 0; x < 360; x +=STEP)
	{
		Serial.println(cardinalSin(x, 20, x + 20));  // test for on the line
	}
	Serial.println("---------");
#endif


*/
