#include <SD.h>
#include <cstring>
#include <M5Unified.h>
#include "watchdogs.h"
//#include <cppQueue.h>
#include <ArduinoQueue.h>
#include "wavePlayer.h"
#include <TinyGPS++.h>

#include <LinkedList.h>
#include "locate.h"

#include <esp_log.h>

extern 	TinyGPSPlus gps;
#define LINE Serial.printf("%s:%d\n", __FUNCTION__, __LINE__)

//static constexpr const gpio_num_t SDCARD_CSPIN = GPIO_NUM_4;

static SemaphoreHandle_t hLocationMutex;

static File root;

GPS_ENTRY2 targetCamera;  // allow anyone to see closest cam

//------------------------------------------------
static LinkedList <GPS_ENTRY2 *> cameraList;

//-------------------------------------------------------------
static void trimEnds(char *who)
{
	while (who[0] == ' ' ) strcpy (who, who+1);
	while (who[strlen(who)-1] == ' ' ) who[strlen(who)-1] = '\0';
}
//-------------------------------------------------------------

static int32_t copySDtoCameraList(const char* filename)
{
	String item;
	char *cstr;
	char fname[80];
	int cnt=0;
	GPS_ENTRY2 *aCamera;
	
	if (xSemaphoreTake(hLocationMutex, portMAX_DELAY) == pdTRUE)
	{
		if (filename[0] != '/' )
		{
			strcpy(&fname[1], filename);
			fname[0]='/';
		}
		else
			strcpy(fname, filename);

		Serial.printf("%s open %s for reading\n", __FUNCTION__, fname);
		auto file = SD.open(fname);

		if (!file) 
		{
			Serial.printf("%s FAILED %s for reading\n", __FUNCTION__, fname);
			xSemaphoreGive(hLocationMutex);
			return false; 
		}

		while (file.available())
		{
			int x;
			x = x +5 ;
			
			//kickDog();
			
			aCamera = new(GPS_ENTRY2);
		
			cnt++;
			item = file.readStringUntil('\n');

			//convert 'String' to C-String
			cstr = new char [item.length()+1];
			std::strcpy (cstr, item.c_str());
			Serial.printf("%4d %s\n", cnt, cstr);
			
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

			trimEnds(cDir);
			trimEnds(clat);
			trimEnds(clon);
			trimEnds(aCamera->cardinal);
			trimEnds(aCamera->onStreet);
			trimEnds(aCamera->crossStreet);
			
			if (ret != 6)
			{
				Serial.printf("%s:%d bad data\n", __FUNCTION__, __LINE__);
				delay(3000);
				assert(ret == 6);
				
				continue; // bad data
			}
			
			iDir = atoi(cDir);
			flat = atof(clat);	
			flon = atof(clon);	

			aCamera->lat = flat;
			aCamera->lng = flon;
			aCamera->bearing = iDir;
			
			//Serial.printf("xx %f %f %d\n", flat, flon, iDir);
			//Serial.printf("%s on %s \n", aCamera->onStreet, aCamera->crossStreet);

			cameraList.add(aCamera);
			delete cstr;
		}
		
		file.close();

		#if 0
		for (int i = 0; i < cameraList.size(); i++)
		{
			Serial.print("Element at index ");
			Serial.print(i);
			Serial.print(": ");
			aCamera = cameraList.get(i);
			Serial.printf("%f/%f on=%s ac=%s\n", 
					aCamera->lat,
					aCamera->lng,
					aCamera->onStreet,
					aCamera->crossStreet);
		}
		#endif
		
		xSemaphoreGive(hLocationMutex);
	}	
	Serial.printf("%s:%d  %d (=%d?) items loaded sd->db\n",
				  __FUNCTION__, __LINE__, cnt, cameraList.size());
	return cnt;
}


//-------------------------------------------------------------

int32_t copyCameraListToSD(char* filename)
{
	char fname[80];
	int cnt=0;
	char bigMessage[150];
	int i;
	
	GPS_ENTRY2 *aCamera;

	if (xSemaphoreTake(hLocationMutex, portMAX_DELAY) == pdTRUE)
	{
		if ( filename[0] != '/')
		{
			strcpy(&fname[1], filename);
			fname[0]='/';
		}
		else
			strcpy(fname, filename);
			
		Serial.printf("%s open %s for writing\n", __FUNCTION__, fname);
		auto file = SD.open(fname, FILE_WRITE);

		if (!file) 
		{
			Serial.printf("FAIL: could not open %s for writing\n");
			xSemaphoreGive(hLocationMutex);
			return false;
		}

		for (i = 0; i < cameraList.size(); i++)
		{
			aCamera = cameraList.get(i);

			//+45.2948422,-75.8642632 ,  71, "ENE", "Bridlewood" , "Aintree"
			sprintf(bigMessage, "%f,%f,%d,%s,%s,%s", 
				aCamera->lat,
				aCamera->lng,
				aCamera->bearing,
				aCamera->cardinal,
				aCamera->onStreet,
				aCamera->crossStreet);
			
			file.println(bigMessage);
			
			Serial.printf("\twriting :[%3d]  %s\n", i, bigMessage);

		}

		file.close();
		Serial.printf("%s closed  %d items written\n", fname, i);
		xSemaphoreGive(hLocationMutex);
	}	

	return cnt;
}

//-------------------------------------------------------------

int32_t addToCameraList(GPS_ENTRY2 *userData)
{
	String item;
	char *cstr;
	int dist;
	
	char bigString[120];
	GPS_ENTRY2 *aCamera = new(GPS_ENTRY2);
	
	//no no no! this is a shallow copy.
	//data will dissapear as the stack is washed! :(
	//aCamera = userData; 

	// deep copy.
	aCamera->lat = userData->lat;
	aCamera->lng = userData->lng;
	aCamera->bearing = userData->bearing;
	strcpy(aCamera->cardinal, userData->cardinal);
	strcpy(aCamera->crossStreet, userData->crossStreet);
	strcpy(aCamera->onStreet, userData->onStreet);
	
	//+45.2948422,-75.8642632 ,  71, "ENE", "Bridlewood" , "Aintree"

	dist = quickSearchDistance(aCamera->lat, aCamera->lng);

	sprintf(bigString, "%f,%f , %d, \"%s\", \"%s\", \"%s\" ", 
		aCamera->lat,
		aCamera->lng,
		aCamera->bearing,
		aCamera->cardinal,
		aCamera->onStreet,
		aCamera->crossStreet);
	
	Serial.printf("adding %s\n", bigString);
	
	
	if (dist < 100)
		Serial.printf("%s:%d adding camera close to another %dm\n",
			__FUNCTION__,__LINE__, dist);

	if (xSemaphoreTake(hLocationMutex, portMAX_DELAY) == pdTRUE)
	{
		cameraList.add(aCamera);
		xSemaphoreGive(hLocationMutex);
	}			
	Serial.printf("%s:%d %d items in list\n", 
				  __FUNCTION__,__LINE__,
				  cameraList.size());
		
	return cameraList.size();
}

//--------------------------------------------------------------

int removeNearbyCamera(float userLat, float userLng)
{

	GPS_ENTRY2 *aCamera, *cCamera;
	int closestDist = -1;

	// do not do any GPS with 0.0 it will hang (hi GD).
	if (!(int)userLat )
	{
		return -1;
	}
	
	Serial.printf("%s:%d %d items in list\n", __FUNCTION__, __LINE__, cameraList.size());
	
	if (xSemaphoreTake(hLocationMutex, portMAX_DELAY) == pdTRUE)
	{
		int dist;
		int course;
		
		int index;
		int closestIndex = -1;

		int end = cameraList.size();
	
		for (index = 0; index < end; index++)
		{
			aCamera = cameraList.get(index);
			
			dist = (int) gps.distanceBetween(userLat, userLng, aCamera->lat, aCamera->lng);
	
			if ( dist < 150 )
			{
				closestDist = dist;
				closestIndex = index;
				cCamera = aCamera;
			}
		}

		if (closestIndex < 0)
		{
			Serial.printf("%s:%d nothing close in 150M found\n", __FUNCTION__, __LINE__);
		}
		else
		{
			Serial.printf("%s:%d removing lat=%11.8f lng=%11.8f ci=%d dist=%d\n", 
						__FUNCTION__, __LINE__, 
						cCamera->lat, cCamera->lng,
						 closestIndex,closestDist);
			cameraList.remove(closestIndex);
			
			M5.Speaker.tone(1000, 200);
			delay(200);
			M5.Speaker.tone(2000, 300);				
			delay(200);
			M5.Speaker.tone(900 , 200);				
		}	
 		xSemaphoreGive(hLocationMutex);
	}	

	Serial.printf("%s:%d %d items in list\n", __FUNCTION__, __LINE__, cameraList.size());
	return closestDist;
}

//--------------------------------------------------------------

bool bTargetHasChanged = true;

// scan for closest location do not update any globals
int quickSearchDistance(float userLat, float userLng)
{

	GPS_ENTRY2 *dbCamera;
	GPS_ENTRY2 *closeCam;
	
	int nearestDist = INT_MAX;

	// do not do any GPS with 0.0 it will hang (hi GD).
	if (!(int)userLat )
	{
		Serial.printf("%s:%d skipping ... zero lat or long\n", __FUNCTION__, __LINE__);
		return -1; // not ready (negative distance is not possible)
	}
	
	if (xSemaphoreTake(hLocationMutex, portMAX_DELAY) == pdTRUE)
	{
		int dist;
		int i;
		
		for (int i = 0; i < cameraList.size(); i++)
		{
			dbCamera = cameraList.get(i);
			dist = (int) gps.distanceBetween(userLat, userLng, dbCamera->lat, dbCamera->lng);
	
			if ( dist < nearestDist )
			{
				nearestDist = dist;
				closeCam = dbCamera;
			}
		}
		xSemaphoreGive(hLocationMutex);
	}	
	
	return nearestDist;
}

int findNearestCamera(float vehicleLat, float vehicleLng)
{

	GPS_ENTRY2 *aCamera = NULL;
	GPS_ENTRY2 *closestCam = NULL;
	
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

		int end = cameraList.size();
	
		for (i = 0; i < end; i++)
		{
			aCamera = cameraList.get(i);
			
			//Serial.printf("%s:%d %d = %+9.7f  %+9.7f\n", __FUNCTION__, __LINE__, i, aCamera->lat, aCamera->lng);
			//course = (int)gps.courseTo(vehicleLat, vehicleLng, cameraLocations[i].lat, cameraLocations[i].lng);
			//cardinal = gps.cardinal(course);
	
			dist = (int) gps.distanceBetween(vehicleLat, vehicleLng, aCamera->lat, aCamera->lng);
	
			if ( dist < closestDist )
			{
				closestDist = dist;
				closestCam = aCamera;
			}
		}
		
		if (closestCam && targetCamera.lat != closestCam->lat)
		{
			targetCamera = *closestCam;
			add_to_playlist("informationOnly.wav");
			add_to_playlist("delay100.wav");
			add_to_playlist("allClear.wav");
			bTargetHasChanged = true;
		}
		
 		xSemaphoreGive(hLocationMutex);
	}	

	return closestDist;
}

//-------------------------------------------------------------

void setup_locate(void)
{
	hLocationMutex = xSemaphoreCreateMutex();
	copySDtoCameraList("gps.db");
	// testing copyCameraListToSD("backup.db");
}


