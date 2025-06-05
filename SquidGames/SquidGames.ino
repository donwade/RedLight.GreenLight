//strip all "M5" refereces from this file. May rebase to t-beam later

#include <M5Core2.h>
#include <TinyGPS++.h>
#include "m5Core2-only.h"

#include "soc/rtc_wdt.h"
#include "esp_int_wdt.h"
#include "esp_task_wdt.h"

static portMUX_TYPE my_mutex;

IRAM_ATTR void xsetup() {
  rtc_wdt_protect_off();
  rtc_wdt_disable();
  disableCore0WDT();
  disableLoopWDT();
}


extern void setup_ORIG(void);

// The TinyGPS++ object
TinyGPSPlus gps;

//=============================================================
void smartDelay(unsigned long ms) {
    unsigned long start = millis();
    do {
        while (Serial2.available() > 0) gps.encode(Serial2.read());
    } while (millis() - start < ms);
    //clear();
}

//=============================================================
void displayInfo() {
	clear();
    setCursor(0, 40, 4);
    print(F("Latitude:    "));
    if (gps.location.isValid()) {
        print(gps.location.lat(), 6);

    } else {
        print(F("INVALID"));
    }

    println();
    print(F("Longitude:    "));
    if (gps.location.isValid()) {
        print(gps.location.lng(), 6);
    } else {
        print(F("INVALID"));
    }

    println();
    print(F("Altitude:    "));
    if (gps.altitude.isValid()) {
        print(gps.altitude.meters());
    } else {
        print(F("INVALID"));
    }

    println();
    print(F("Satellites:    "));
    if (gps.satellites.isValid()) {
        print(gps.satellites.value());
    } else {
        print(F("INVALID"));
    }

    println();
    print(F("Date: "));
    if (gps.date.isValid()) {
        print(gps.date.month());
        print(F("/"));
        print(gps.date.day());
        print(F("/"));
        print(gps.date.year());
    } else {
        print(F("INVALID"));
    }

    println();
    print(F("Time: "));
    if (gps.time.isValid()) {
        if (gps.time.hour() < 10) print(F("0"));
        print(gps.time.hour());
        print(F(":"));
        if (gps.time.minute() < 10) print(F("0"));
        print(gps.time.minute());
        print(F(":"));
        if (gps.time.second() < 10) print(F("0"));
        print(gps.time.second());
        print(F("."));
        if (gps.time.centisecond() < 10) print(F("0"));
        print(gps.time.centisecond());
    } else {
        print(F("INVALID"));
    }
}

//=============================================================
extern "C" void setup_dogs();

void setup() {
	Serial.begin(115200);

	while(!Serial) delay(100); // in event of crash loop
	delay(2000);
	
	setup_M5();	
	
    /*   kMBusModeOutput,powered by USB or Battery
    kMBusModeInput,powered by outside input need to fill in this Otherwise
    M5Core2 will not work properly
	*/
    Serial2.begin(9600, SERIAL_8N1, 13, 14);

	setTextColor(GREEN, BLACK);

	setup_ORIG();

	setup_dogs();
	//disableCore0WDT();

    //  while (*gpsStream)
    //    if (gps.encode(*gpsStream++))
    //      displayInfo();
}

extern void loop_ORIG(void *);

void loop() {
#if 1
    delay(1000);
	//displayInfo();
#else
    loop_ORIG(NULL);

#endif

    smartDelay(1000);
}

