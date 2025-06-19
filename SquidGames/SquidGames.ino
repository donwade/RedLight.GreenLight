//strip all "M5" refereces from this file. May rebase to t-beam later

#include <M5Core2.h>
#include <TinyGPS++.h>
#include "watchdogs.h"
#include "wav2spkr.h"

#include "m5Core2-only.h"

#include "soc/rtc_wdt.h"

static portMUX_TYPE my_mutex;

IRAM_ATTR void xsetup() {
  rtc_wdt_protect_off();
  rtc_wdt_disable();
  disableCore0WDT();
  disableLoopWDT();
}


extern void setup_GPS(void);
extern void loop_GPS(void *);
extern TaskHandle_t speak_file(char *waveFilename);

// The TinyGPS++ object
TinyGPSPlus gps;

//=============================================================
void smartDelay(unsigned long ms) {
    unsigned long start = millis();
    do {
        while (Serial2.available() > 0)	gps.encode(Serial2.read());
		///////////////////////////////////
		delay((9600/1000)); // 9600baud in mS allow any task dogs !!!!!
		///////////////////////////////////
    } while (millis() - start < ms);
    //clear();
}

//=============================================================
void displayInfo() {

	lclear();
	
    lsetCursor(0, 40, 4); // font=4
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

//=============================================================

void setup() {
	Serial.begin(115200);

	while(!Serial) delay(100); // in event of crash loop
	Serial.println("----");
	delay(3000);
	
	setup_M5();	
	
    /*   kMBusModeOutput,powered by USB or Battery
    kMBusModeInput,powered by outside input need to fill in this Otherwise
    M5Core2 will not work properly
	*/
    Serial2.begin(9600, SERIAL_8N1, 13, 14);

	lsetTextColor(GREEN, BLACK);
    lsetCursor(0, 0, 4); // font=4

	//setup_ORIG();

	//test_watchDogs();

	spawnTaskAndDog( loop_GPS, 		//(void * not_used)TaskFunction_t pvTaskCode,
                     "loop_GPS",    //const char * const pcName,
                     1024 * 3,		//const uint32_t usStackDepth,
                     NULL,			//void * const pvParameters,
                     4           	//UBaseType_t uxPriority)
                     );
	TaskHandle_t hSpkThread = speak_file(WAV_FILE_NAME);

}


void loop() {
#if 0
	displayInfo();
#else
    //loop_GPS(NULL);
#endif
	delay(1000);

}

