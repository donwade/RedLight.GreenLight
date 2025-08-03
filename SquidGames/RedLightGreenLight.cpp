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

#define BUILTIN_LED 4  // TIP t-beam
extern void smartDelay(unsigned long ms);

extern TinyGPSPlus gps;

#define MIN_SPEED_KPH 9 // dont do compass if speed lower than this.
//------------------------------------------------------------------

typedef struct gpsLocation { double lng; double lat; };

typedef struct gpsMisc 
{
	float speed;
	const char *cardinal;
	float qual;
	float Kmph;
	float course;		//direction in float degrees
	
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
};
	
gpsLocation iLocation;
gpsMisc     iMisc;

//-----------------------------------------------------------------

//#define SIMULATOR

void getData(void)
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

#else

	iLocation.lat = gps.location.lat();
	iLocation.lng = gps.location.lng();
	iMisc.hour = gps.time.hour();
	iMisc.minute = gps.time.minute();
	iMisc.second = gps.time.second();
	iMisc.Kmph = gps.speed.kmph();
	iMisc.qual = gps.hdop.hdop();
	iMisc.course = gps.course.deg();

	/* not required. tbeam builds char by char 
	if (millis() > 5000 && gps.charsProcessed() < 10)
		Serial.println(F("No GPS data received: check wiring"));
	else
		Serial.printf("got reading\n");		
	*/
	
#endif
}


gpsLocation gpsAverage;
bool bButtonPressed	= false;


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


static void * reportingMode(BUTTON_EVENT some_key);

static void * learningMode(BUTTON_EVENT some_key)
{
	int dist;
	int course;
	const char *dir;
	static KEY_STATE here;
	
#if 0
	if (iMisc.Kmph > 5)
		xprintf(2, "%3d %s", veh_course, veh_cardinal);
	else
		xprintf(2, "%2d/%2d/%4d S=%2d", gps.date.day(), gps.date.month(), 
				gps.date.year(), gps.satellites.value());
#endif	
	cprintf(_WHITE, 0, "NOW    LA=%+10.7f", gpsAverage.lat);
	cprintf(_WHITE, 1, "NOW    LN=%+10.7f", gpsAverage.lng);
	cprintf(_RED, 2,   "CAMERA LA=%+9.7f", cameraLocation.lat);
	cprintf(_RED ,3,   "CAMERA LO=%+9.7f", cameraLocation.lng);
	cprintf(_GREEN, 4, "AWAY   LA=%+9.7f", endLocation.lat);
	cprintf(_GREEN, 5, "AWAY   LO=%+9.7f", endLocation.lng);

	dist = gps.distanceBetween(cameraLocation.lat, cameraLocation.lng, endLocation.lat, endLocation.lng);
	course = (int)gps.courseTo(cameraLocation.lat, cameraLocation.lng, endLocation.lat, endLocation.lng);
	dir = gps.cardinal(course);
	
	cprintf(_YELLOW, 6, "course = %d dir=%3s", course, dir);
	
	// all display updates done ... just keys left
	if (some_key == DISPLAY_REFRESH) return (void*) learningMode;
	
	Serial.printf("handled key %d\n", some_key);

	switch (some_key)
	{
		case BUTTON_INIT:
			lfillRect(0,0, 50, 50, _BLUE);
			threeButtonText("AWAY", "SAVE", "CAMERA");
			cprintf(_RED,	2, "LA=%+9.7f", gpsAverage.lat);
			cprintf(_RED ,	3, "LO=%+9.7f", gpsAverage.lng);
			cprintf(_GREEN, 4, "LA=%+9.7f", gpsAverage.lat);
			cprintf(_GREEN, 5, "LO=%+9.7f", gpsAverage.lng);
			cprintf(_ORANGE,6, "CAMERA or AWAY");
		break;	
			
		case LBUTTON_UP:
		case LBUTTON_DN:
			if (some_key == LBUTTON_DN)
			{
				endLocation = gpsAverage;
				cprintf(_ORANGE, 6, "NEXT CAMERA or SAVE");
			}
			
		break;

		case RBUTTON_UP:
		case RBUTTON_DN:
			if (some_key == RBUTTON_DN)
			{
				cameraLocation = gpsAverage;
				cprintf(_ORANGE, 6, "NEXT AWAY or SAVE");
			}

		break;

		case MBUTTON_DN:
		case MBUTTON_UP:
			
			if (some_key == MBUTTON_DN)
			{
				return (void*) reportingMode;
			}			
			break;
		break;

	}
	return (void*) learningMode;
}


static void * reportingMode(BUTTON_EVENT some_key)
{
	int dist;
	int course;
	const char *dir;
	static KEY_STATE here;
	const char *cardinal;


	findNearestCamera(iLocation.lat, iLocation.lng);
	
	course = (int)gps.courseTo(iLocation.lat, iLocation.lng, closestCam->lat, closestCam->lng);
	cardinal = gps.cardinal(course);

	dist = (int) gps.distanceBetween(iLocation.lat, iLocation.lng, closestCam->lat, closestCam->lng);
	
	cprintf(_WHITE, 0, "%s", closestCam->onStreet);
	cprintf(_WHITE, 1, "%s",  closestCam->crossStreet);
	cprintf(dist > 100 ? _GREEN : _RED, 2, "DIST=%4d m %3d %s", dist, course, cardinal);

	xprintf(3, "%VEH=%3d kph %3.1f%%", (int)iMisc.Kmph, iMisc.qual);


	cprintf(_GREEN, 4, "NOW LA=%+9.7f", gpsAverage.lat);
	cprintf(_GREEN, 5, "NOW LO=%+9.7f", gpsAverage.lng);

	
	// all display updates done ... just keys left
	if (some_key == DISPLAY_REFRESH) return (void*) reportingMode;

	Serial.printf("handled key %d\n", some_key);

	switch (some_key)
	{
		case BUTTON_INIT:
			lfillRect(0,0, 50, 50, _RED);
			threeButtonText("QUIET", "OK", "BYTEME");
			
			cprintf(_RED,	2, "TODO        ");
			cprintf(_RED ,	3, "TODO        ");
			cprintf(_GREEN, 4, "TODO        ");
			cprintf(_GREEN, 5, "TODO        ");
			cprintf(_ORANGE,6, "TODO        ");
			
		break;	
			
		case LBUTTON_UP:
		case LBUTTON_DN:

			if (some_key == LBUTTON_DN)
			{
				cprintf(_ORANGE, 6, "TODO LEFT");
			}
			
		break;

		case RBUTTON_UP:
		case RBUTTON_DN:

			if (some_key == RBUTTON_DN)
			{
				cprintf(_ORANGE, 6, "TODO RIGHT");
			}

		break;

		case MBUTTON_DN:
		case MBUTTON_UP:

			if (some_key == MBUTTON_DN)
			{
				return (void*) learningMode;
			}			
			
		break;

	}

/*
	What is HDOP 
	< 1 Ideal

	1-2 Excellent
	Highest possible confidence level to be used for applications demanding the highest possible precision at all times.
	At this confidence level, positional measurements are considered accurate enough to meet all but the most sensitive applications.

	2-5 Good

	Represents a level that marks the minimum appropriate for making accurate decisions. Positional measurements could be used to make reliable in-route navigation suggestions to the user.
	Positional measurements could be used for calculations, but the fix quality could still be improved. A more open view of the sky is

	5-10 Moderate

	10-20 Fair
	Represents a low confidence level. Positional measurements should be discarded or used only to indicate a very rough estimate

	>20 Poor At this level, measurements should be discarded
*/

	return (void *) reportingMode;
}	

//------------------------------------------------------
typedef  void* (*pStateFunction)(BUTTON_EVENT);


pStateFunction stateMachines[] =
{
	learningMode,
	reportingMode
};

void stateDisplay(BUTTON_EVENT some_key)
{
	static volatile pStateFunction lastCall = learningMode;
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

	{
		getData();
		
		{
			// update rolling history

			samples[sIndex].lat = iLocation.lat;
			samples[sIndex].lng = iLocation.lng;

			// sIndex is left pointing to NEXT position to write to on the next pass
			// therefore sIndex points to oldest entry by time

			if (++sIndex == GPS_SAMPLE_SIZE) sIndex = 0;
		}	

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

		// profile loop time. So far about 3ms total		
		//difftime =  micros() - startProfileTime;
		//Serial.printf("profile = %d uS\n", difftime);

		smartDelay(250);
	}
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
