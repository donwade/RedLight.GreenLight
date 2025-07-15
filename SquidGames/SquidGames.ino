//strip all "M5" refereces from this file. May rebase to t-beam later

//##include <M5Core2.h>
#include <M5Unified.h>
#include <TinyGPS++.h>
#include "watchdogs.h"
#include "wavePlayer.h"
#include "viewController.h"

#include "m5Core2-only.h"

#include "soc/rtc_wdt.h"
#include "esp_debug_helpers.h"

extern int  xprintf(uint8_t lineNo, const char *format, ...); 
#define LINE Serial.printf("%s:%d\n", __FUNCTION__, __LINE__)


static portMUX_TYPE my_mutex;

IRAM_ATTR void xsetup() {
  rtc_wdt_protect_off();
  rtc_wdt_disable();
  disableCore0WDT();
  disableLoopWDT();
}


extern void setup_GPS(void);
extern void runGpsTask(void *);
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
	delay(1000);
	for (int j= 0; j++; j < 10) Serial.println();
	Serial.printf("BUILT ON %s %s *** ESP-IDF VER = %s ***\n", __DATE__, __TIME__, esp_get_idf_version());
	delay(3000);


	setup_M5();	
    setup_wavePlayer();
	
    /*   kMBusModeOutput,powered by USB or Battery
    kMBusModeInput,powered by outside input need to fill in this Otherwise
    M5Core2 will not work properly
	*/
    Serial2.begin(9600, SERIAL_8N1, 13, 14);

	lsetTextColor(TFT_YELLOW, TFT_BLACK);
    lsetCursor(0, 0, 4); // font=4

	//setup_ORIG();

	//dumper();
	//esp_backtrace_print(2);
	//testDump("hi mom");
	//delay(-1);


	spawnTaskAndDogV2( runWavPlayerTask, 		//(void * not_used)TaskFunction_t pvTaskCode,
                     "runWavPlayerTask",    //const char * const pcName,
                     1024 * 10,		//const uint32_t usStackDepth,
                     NULL,			//void * const pvParameters,
                     4           	//UBaseType_t uxPriority)
                     );
	delay(200);

	spawnTaskAndDogV2( runMenuTask, 		//(void * not_used)TaskFunction_t pvTaskCode,
                     "runMenuTask",    //const char * const pcName,
                     1024 * 10,		//const uint32_t usStackDepth,
                     NULL,			//void * const pvParameters,
                     3           	//UBaseType_t uxPriority)
                     );


	delay(200);

	spawnTaskAndDogV2( runGpsTask, 		//(void * not_used)TaskFunction_t pvTaskCode,
                     "runGpsTask",    //const char * const pcName,
                     1024 * 10,		//const uint32_t usStackDepth,
                     NULL,			//void * const pvParameters,
                     4           	//UBaseType_t uxPriority)
                     );

}


void runWavPlayerTask(void *NOTUSED)
{
		static unsigned cnt = 1111;
        kickDog();
		Tdelay(2000);
        if (cnt == 0) run_wavePlayer();
		xprintf(5,"count1=%d", cnt++);
}

void runMenuTask(void *NOTUSED)
{
	menu_task();
}



void loop() {
#if 0
	displayInfo();
#include "esp_system.h"

	const int button = 0;         //gpio to use to trigger delay
	const int wdtTimeout = 3000;  //time in ms to trigger the watchdog
	hw_timer_t *timer = NULL;

	void ARDUINO_ISR_ATTR resetModule() {
	  ets_printf("reboot\n");
	  esp_restart();
	}

	void setup() {
	  Serial.begin(115200);
	  Serial.println();
	  Serial.println("running setup");

	  pinMode(button, INPUT_PULLUP);                    //init control pin
	  timer = timerBegin(0, 80, true);                  //timer 0, div 80
	  timerAttachInterrupt(timer, &resetModule, true);  //attach callback
	  timerAlarmWrite(timer, wdtTimeout * 1000, false); //set time in us
	  timerAlarmEnable(timer);                          //enable interrupt
	}

	void loop() {
	  Serial.println("running main loop");

	  timerWrite(timer, 0); //reset timer (feed watchdog)
	  long loopTime = millis();
	  //while button is pressed, delay up to 3 seconds to trigger the timer
	  while (!digitalRead(button)) {
	    Serial.println("button pressed");
	    delay(500);
	  }
	  delay(1000); //simulate work
	  loopTime = millis() - loopTime;
	  
	  Serial.print("loop time is = ");
	  Serial.println(loopTime); //should be under 3000
	}

#else
    //runGpsTask(NULL);
#endif
	delay(100);
	//kickDog();
	vTaskDelete(NULL);

}

