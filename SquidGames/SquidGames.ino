//strip all "M5" refereces from this file. May rebase to t-beam later

//##include <M5Core2.h>
#include <M5Unified.h>
#include <TinyGPS++.h>
#include "watchdogs.h"
#include "wavePlayer.h"
#include "viewController.h"

#include "m5Core2-only.h"
#include "locate.h"

#include "rtc_wdt.h"
#include "esp_debug_helpers.h"

#define LINE Serial.printf("%s:%d\n", __FUNCTION__, __LINE__)
extern void setup_BN880(void);

static portMUX_TYPE my_mutex;

extern void setup_GPS(void);
extern void gpsGetDataTask(void *);
extern void runDisplayTask(void *not_used);

extern TaskHandle_t speak_file(char *waveFilename);

// The TinyGPS++ object
TinyGPSPlus gps;


//=============================================================
void smartDelay(unsigned long ms) {
    unsigned long start = millis();
    do {
        while (Serial2.available() > 0)	gps.encode(Serial2.read());
		///////////////////////////////////
		Tdelay((9600/1000)); // 9600baud in mS allow any task dogs !!!!!
		///////////////////////////////////
    } while (millis() - start < ms);
    //clear();
}

//=============================================================
void displayInfo() {

	lclear();
	
    lsetCursor(0, 40);
    lprint(F("Latitude:    "));
    if (gps.location.isValid()) {
        lprint(gps.location.lat(), 6);

    } else {
        lprint(F("INVALID"));
    }

    lprintln();
    lprint(F("Longitude:    "));
    if (gps.location.isValid()) {
        lprint(gps.location.lng(), 6);
    } else {
        lprint(F("INVALID"));
    }

    lprintln();
    lprint(F("Altitude:    "));
    if (gps.altitude.isValid()) {
        lprint(gps.altitude.meters());
    } else {
        lprint(F("INVALID"));
    }

    lprintln();
    lprint(F("Satellites:    "));
    if (gps.satellites.isValid()) {
        lprint(gps.satellites.value());
    } else {
        lprint(F("INVALID"));
    }

    lprintln();
    lprint(F("Date: "));
    if (gps.date.isValid()) {
        lprint(gps.date.month());
        lprint(F("/"));
        lprint(gps.date.day());
        lprint(F("/"));
        lprint(gps.date.year());
    } else {
        lprint(F("INVALID"));
    }

    lprintln();
    lprint(F("Time: "));
    if (gps.time.isValid()) {
        if (gps.time.hour() < 10) lprint(F("0"));
        lprint(gps.time.hour());
        lprint(F(":"));
        if (gps.time.minute() < 10) lprint(F("0"));
        lprint(gps.time.minute());
        lprint(F(":"));
        if (gps.time.second() < 10) lprint(F("0"));
        lprint(gps.time.second());
        lprint(F("."));
        if (gps.time.centisecond() < 10) lprint(F("0"));
        lprint(gps.time.centisecond());
    } else {
        lprint(F("INVALID"));
    }
}


void runMenuTask(void *NOTUSED)
{
	touchPanel_impl();
}

void loop() {
	delay(100);
	vTaskDelete(NULL);

}



//=============================================================
static const gpio_num_t SDCARD_CSPIN = GPIO_NUM_4;

void setup() {
	Serial.begin(115200);

	while(!Serial) delay(100); // in event of crash loop
	delay(1000);
	for (int j= 0; j++; j < 10) Serial.println();
	Serial.printf("BUILT ON %s %s *** ESP-IDF VER = %s ***\n", __DATE__, __TIME__, esp_get_idf_version());
	delay(3000);

    bool ok = SD.begin(SDCARD_CSPIN, SPI, 25000000);
	Serial.printf("SD=%d\n", ok);
	
	setup_M5();	
    setup_wavePlayer();
	setup_BN880();
	setup_locate();

	//BN880 takes care of below.
    //Serial2.begin(9600, SERIAL_8N1, 13, 14);

	lsetTextColor(TFT_YELLOW, TFT_BLACK);
    lsetCursor(0, 0);

	spawnTaskAndDogV2( wavPlayerTask, 		//(void * not_used)TaskFunction_t pvTaskCode,
                     "wavPlayerTask",    //const char * const pcName,
                     1024 * 10,		//const uint32_t usStackDepth,
                     NULL,			//void * const pvParameters,
                     4           	//UBaseType_t uxPriority)
                     );
	delay(200);

	Serial.println("dwade - run menu disabled");
	spawnTaskAndDogV2( runMenuTask, 		//(void * not_used)TaskFunction_t pvTaskCode,
                     "runMenuTask",    //const char * const pcName,
                     1024 * 10,		//const uint32_t usStackDepth,
                     NULL,			//void * const pvParameters,
                     4           	//UBaseType_t uxPriority)
                     );

	delay(200);

	spawnTaskAndDogV2( gpsGetDataTask,	//(void * not_used)TaskFunction_t pvTaskCode,
                     "gpsGetDataTask",	//const char * const pcName,
                     1024 * 10,			//const uint32_t usStackDepth,
                     NULL,				//void * const pvParameters,
                     4           		//UBaseType_t uxPriority)
                     );


	spawnTaskAndDogV2( runDisplayTask, 		//(void * not_used)TaskFunction_t pvTaskCode,
                     "runDisplayTask",    //const char * const pcName,
                     1024 * 10,		//const uint32_t usStackDepth,
                     NULL,			//void * const pvParameters,
                     4           	//UBaseType_t uxPriority)
                     );


	add_to_playlist("terrain.wav");
	add_to_playlist("speed.wav");
	add_to_playlist("whoopwoop.wav");
	add_to_playlist("terrain.wav");
/*	
	add_to_playlist("three.wav");
	add_to_playlist("thousand.wav");
	add_to_playlist("two.wav");
	add_to_playlist("hundred.wav");
	
	add_to_playlist("forty.wav");
	add_to_playlist("one.wav");
*/
}

