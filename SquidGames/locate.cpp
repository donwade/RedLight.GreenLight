#include <SD.h>
#include <cstring>
#include <M5Unified.h>
#include "watchdogs.h"
#include <cppQueue.h>

#include <LinkedList.h>
#include "locate.h"

#include <esp_log.h>

#define LINE Serial.printf("%s:%d\n", __FUNCTION__, __LINE__)

static constexpr const gpio_num_t SDCARD_CSPIN = GPIO_NUM_4;

static constexpr const size_t buf_num = 3;
static constexpr const size_t buf_size = 1024;

static SemaphoreHandle_t xCountingSemaphore;


static File root;

#define MAX_FILES_CACHED 200

#define MAX_FILENAME_LEN 50
#define MAX_FILES_QUEUED 8


static int numActiveSndFiles = 0;


//------------------------------------------------
static LinkedList <GPS_ENTRY2 *> cameras;

static int32_t readFromSD(const char* filename)
{
	String item;
	char *cstr;
	char fname[80];
	int cnt=0;
	GPS_ENTRY2 *aCamera;
	
	strcpy(&fname[1], filename);
	fname[0]='/';
	
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

	
	return cnt;
}

uint32_t loadGpsDb(char *database)
{
	readFromSD(database);
}


//------------------------------------------------
#if 0
static cppQueue playlistQ(MAX_FILENAME_LEN, 8, FIFO);

void junkTask(void *NOTUSED)
{
	char playThisFile[MAX_FILENAME_LEN+1];
	
	while (true)
	{
		if (xSemaphoreTake( xCountingSemaphore, pdMS_TO_TICKS(1000) ) == pdTRUE)
		{

			playlistQ.pop(playThisFile);
			Serial.printf("popping %s\n", playThisFile);
			readFromSD(playThisFile);
		}
		else
		{
			kickDog();
		}
	}
}
#endif

