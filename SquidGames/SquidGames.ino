//strip all "M5" refereces from this file. May rebase to t-beam later

//##include <M5Core2.h>
#include <M5Unified.h>
#include <TinyGPS++.h>
#include "watchdogs.h"
#include "wavePlayer.h"

#include "m5Core2-only.h"

#include "soc/rtc_wdt.h"
#include "esp_debug_helpers.h"

extern int  xprintf(uint8_t lineNo, const char *format, ...); 
extern m5::touch_detail_t touchDetail;
extern LGFX_Button button_pre, button_play, button_next;

#define LINE Serial.printf("%s:%d\n", __FUNCTION__, __LINE__)


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
	Serial.printf ("\033c");
	
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
	
	spawnTaskAndDogV2( loop_test1, 		//(void * not_used)TaskFunction_t pvTaskCode,
                     "loop_test1",    //const char * const pcName,
                     1024 * 10,		//const uint32_t usStackDepth,
                     NULL,			//void * const pvParameters,
                     4           	//UBaseType_t uxPriority)
                     );
	delay(2000);
	spawnTaskAndDogV2( gui_loop, 		//(void * not_used)TaskFunction_t pvTaskCode,
                     "gui_loop",    //const char * const pcName,
                     1024 * 10,		//const uint32_t usStackDepth,
                     NULL,			//void * const pvParameters,
                     3           	//UBaseType_t uxPriority)
                     );


	delay(2000);

	spawnTaskAndDogV2( loop_GPS, 		//(void * not_used)TaskFunction_t pvTaskCode,
                     "loop_GPS",    //const char * const pcName,
                     1024 * 3,		//const uint32_t usStackDepth,
                     NULL,			//void * const pvParameters,
                     4           	//UBaseType_t uxPriority)
                     );

}


void loop_test1(void *NOTUSED)
{
		static unsigned cnt = 0;
        kickDog();
        if (cnt == 0) run_wavePlayer();
		xprintf(5,"count1=%d", cnt++);
        Tdelay(1000);
}

void gui_loop(void *NOTUSED)
{
    M5.update();
    touchDetail = M5.Touch.getDetail();

    if (touchDetail.isPressed())
	{
        if(button_pre.contains(touchDetail.x, touchDetail.y))
		{
			LINE;
            Serial.println("Btn_pre pressed");
        }
        else if(button_play.contains(touchDetail.x, touchDetail.y))
		{
			LINE;
            Serial.println("Btn_play pressed");
        }
        else if(button_next.contains(touchDetail.x, touchDetail.y))
		{
			LINE;
            Serial.println("Btn_next pressed");
        }
    }

}



void loop() {
#if 0
	displayInfo();
#else
    //loop_GPS(NULL);
#endif
	delay(1000);

}

