#include <SD.h>
#include <cstring>
#include <M5Unified.h>
#include "watchdogs.h"
//#include <cppQueue.h>
#include <ArduinoQueue.h>

#include <TinyGPS++.h>

#include <LinkedList.h>
#include "locate.h"

#include <esp_log.h>

extern 	TinyGPSPlus gps;
#define LINE Serial.printf("%s:%d\n", __FUNCTION__, __LINE__)

//static constexpr const gpio_num_t SDCARD_CSPIN = GPIO_NUM_4;

static SemaphoreHandle_t hLocationMutex;

static File root;

//------------------------------------------------
static LinkedList <GPS_ENTRY2 *> cameras;

//-------------------------------------------------------------

static int32_t readFromSD(const char* filename)
{
	String item;
	char *cstr;
	char fname[80];
	int cnt=0;
	GPS_ENTRY2 *aCamera;
	
	if (xSemaphoreTake(hLocationMutex, portMAX_DELAY) == pdTRUE)
	{
		strcpy(&fname[1], filename);
		fname[0]='/';

		Serial.printf("%s open %s for reading\n", __FUNCTION__, fname);
		auto file = SD.open(fname);

		if (!file) { return false; }

		while (file.available())
		{
			aCamera = new(GPS_ENTRY2);
		
			cnt++;
			item = file.readStringUntil('\n');

			//convert 'String' to C-String
			cstr = new char [item.length()+1];
			std::strcpy (cstr, item.c_str());
			Serial.printf("%d %s\n", cnt, cstr);
			
			//+45.2948422,-75.8642632 ,  71, "ENE", "Bridlewood" , "Aintree"

			float   flat,flon;
			int		iDir;
			char	clat[20];
			char	clon[20];
			char	cDir[10];

			//https://stackoverflow.com/questions/15091284/read-comma-separated-input-with-scanf
			int ret = sscanf(cstr, "%[^,],%[^,],%[^,],%[^,],%[^,],%[^,]", 
							clat, clon, cDir, 
							aCamera->cardinal, 
							aCamera->onStreet, 
							aCamera->crossStreet);
			
			if (ret != 6) continue; // bad data
			
			iDir = atoi(cDir);
			flat = atof(clat);	
			flon = atof(clon);	

			aCamera->lat = flat;
			aCamera->lng = flon;
			aCamera->bearing = iDir;
			
			//Serial.printf("xx %f %f %d\n", flat, flon, iDir);
			//Serial.printf("%s on %s \n", aCamera->onStreet, aCamera->crossStreet);

			cameras.add(aCamera);
			delete cstr;
		}
		
		file.close();

		for (int i = 0; i < cameras.size(); i++)
		{
			Serial.print("Element at index ");
			Serial.print(i);
			Serial.print(": ");
			aCamera = cameras.get(i);
			Serial.printf("%f/%f on=%s ac=%s\n", 
					aCamera->lat,
					aCamera->lng,
					aCamera->onStreet,
					aCamera->crossStreet);
		}
		
		xSemaphoreGive(hLocationMutex);
	}	

	return cnt;
}


//-------------------------------------------------------------

int32_t writeToSD(char* filename)
{
	char fname[80];
	int cnt=0;
	char bigMessage[150];
	int i;
	
	GPS_ENTRY2 *aCamera;

	if (xSemaphoreTake(hLocationMutex, portMAX_DELAY) == pdTRUE)
	{
		strcpy(&fname[1], filename);
		fname[0]='/';

		Serial.printf("%s open %s for writing\n", __FUNCTION__, fname);
		auto file = SD.open(fname, FILE_WRITE);

		if (!file) { return false; }

		for (i = 0; i < cameras.size(); i++)
		{
			aCamera = cameras.get(i);

			//+45.2948422,-75.8642632 ,  71, "ENE", "Bridlewood" , "Aintree"
			sprintf(bigMessage, "%f,%f , %d, %s , %s, %s ", 
				aCamera->lat,
				aCamera->lng,
				aCamera->bearing,
				aCamera->cardinal,
				aCamera->onStreet,
				aCamera->crossStreet);
			
			file.println(bigMessage);
			
			//Serial.printf("xyz: %s\n", bigMessage);

		}

		file.close();
		Serial.printf("%s closed  %d items written\n", fname, i);
		xSemaphoreGive(hLocationMutex);
	}	

	return cnt;
}

//-------------------------------------------------------------
static int32_t addCamera(GPS_ENTRY2 *data)
{
	String item;
	char *cstr;
	char fname[80];
	int cnt=0;
	GPS_ENTRY2 *aCamera;

	if (xSemaphoreTake(hLocationMutex, portMAX_DELAY) == pdTRUE)
	{
		char bigString[120];
		
		aCamera = new(GPS_ENTRY2);
		aCamera = data;
		
		//+45.2948422,-75.8642632 ,  71, "ENE", "Bridlewood" , "Aintree"

		sprintf(bigString, "%f,%f , %d, \"%s\", \"%s\", \"%s\" ", 
			aCamera->lng,
			aCamera->lat,
			aCamera->bearing,
			aCamera->cardinal,
			aCamera->onStreet,
			aCamera->crossStreet);

					
		cameras.add(aCamera);
	}
		
	xSemaphoreGive(hLocationMutex);
	return cameras.size();
}



GPS_ENTRY2 *closestCam;
GPS_ENTRY2 *nextClosestCam;


int findNearestCamera(float vehicleLat, float vehicleLng)
{

	GPS_ENTRY2 *aCamera;
	int closestDist = INT_MAX;

	// do not do any GPS with 0.0 it will hang (hi GD).
	if (!(int)vehicleLat )
	{
		Serial.printf("%s:%d skipping ... zero lat or long\n", __FUNCTION__, __LINE__);
		return -1; // not ready (negative distance is not possible)
	}
	
	if (xSemaphoreTake(hLocationMutex, portMAX_DELAY) == pdTRUE)
	{
		int dist;
		int course;
		int i;
			
		for (int i = 0; i < cameras.size(); i++)
		{
			aCamera = cameras.get(i);
			
			//Serial.printf("%+9.7f  %+9.7f\n",  cameraLocations[i].lat, cameraLocations[i].lng);
			//course = (int)gps.courseTo(vehicleLat, vehicleLng, cameraLocations[i].lat, cameraLocations[i].lng);
			//cardinal = gps.cardinal(course);
	
			dist = (int) gps.distanceBetween(vehicleLat, vehicleLng, aCamera->lat, aCamera->lng);
	
			if ( dist < closestDist )
			{
				nextClosestCam = closestCam;
				closestDist = dist;
				closestCam = aCamera;
			}
	
		}
		
#ifdef CHATTY
		Serial.println();
		Serial.printf("lat=%9.7f lng=%9.7f \n", vehicleLat, vehicleLng);
		
		for (int i = 0; i < cameras.size(); i++)
		{
			const char *cardinal;
			int course;
			
			aCamera = cameras.get(i);
			
			dist = (int)gps.distanceBetween(vehicleLat, vehicleLng, aCamera->lat, aCamera->lng);
			course = (int)gps.courseTo(vehicleLat, vehicleLng, aCamera->lat, aCamera->lng);
			cardinal = gps.cardinal(course);
			
			char star;
	
			star = (aCamera == closestCam) ? '1' : ' ';
			if ( star != '1' ) star = (aCamera == nextClosestCam) ? '2' : ' ';
			
			if (star != ' ') Serial.printf("%c [%2d] dist=%4d course=%3d cardinal=%s\n",
				star, i,  dist, course, cardinal);
			
		}
#endif

byebye:
		xSemaphoreGive(hLocationMutex);
	}	

	return closestDist;
}

//-------------------------------------------------------------

void setup_locate(void)
{
	hLocationMutex = xSemaphoreCreateMutex();
	readFromSD("gps.db");
	// testing writeToSD("backup.db");
}


