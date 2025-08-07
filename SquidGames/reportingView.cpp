#include "locate.h"
#include "viewController.h"
#include "wavePlayer.h"

#define LINE Serial.printf("%s:%d\n", __FUNCTION__, __LINE__)
extern void * learningMode(BUTTON_EVENT x);

void speakSpeed(int Kmph)
{
	
}

void speakDistance(int distNow)
{
	static bool bAnnounced10;
	static bool bAnnounced20;
	static bool bAnnounced30;
	static bool bAnnounced40;
	static bool bAnnounced50;
	static bool bAnnounced60;
	static bool bAnnounced70;
	static bool bAnnounced80;
	static bool bAnnounced90;
	static bool bAnnounced100;
	static bool bAnnounced150;
	static bool bAnnounced200;
	static bool bAnnounced300;

	int i;
	
	if (bNewTarget || distNow > 100)
	{
		// '300' always largest than farthest reporting distance
		bNewTarget = false;
		bAnnounced10 = false;
		bAnnounced20 = false;
		bAnnounced30 = false;
		bAnnounced40 = false;
		bAnnounced50 = false;
		bAnnounced60 = false;
		bAnnounced70 = false;
		bAnnounced80 = false;
		bAnnounced90 = false;
		bAnnounced100 = false;
		bAnnounced150 = false;
		bAnnounced200 = false;
		bAnnounced300 = false;
	}	

	if (0)
	{
	}
/*
	else if ( !bAnnounced300 && distNow < 300)
	{
		add_to_playlist("three.wav");
		add_to_playlist("hundred.wav");
		bAnnounced300 = true;
	}
	else if ( !bAnnounced200 && distNow < 200)
	{
		add_to_playlist("two.wav");
		add_to_playlist("hundred.wav");
		bAnnounced200 = true;
	}
	else if ( !bAnnounced150 && distNow < 150)
	{
		add_to_playlist("dangerAhead.wav");
		add_to_playlist("delay500.wav");
		add_to_playlist("one.wav");
		add_to_playlist("hundred.wav");
		add_to_playlist("fifty.wav");
		bAnnounced150 = true;
	}
*/	
	else if ( !bAnnounced100 && distNow < 100)
	{
		add_to_playlist("ninety.wav");
		bAnnounced100 = true;
	}

	else if (!bAnnounced90 && distNow < 90)
	{
		add_to_playlist("glideSlope.wav");
		add_to_playlist("delay100.wav");
		add_to_playlist("eighty.wav");
		bAnnounced90 = true;
	}
	else if (!bAnnounced80 && distNow < 80)
	{
		add_to_playlist("seventy.wav");
		bAnnounced80 = true;
	}
	else if (!bAnnounced70 && distNow < 70)
	{
		add_to_playlist("terrain.wav");
		add_to_playlist("delay200.wav");
		add_to_playlist("fireAlarm.wav");
		add_to_playlist("sixty.wav");
		bAnnounced70 = true;
	}
	else if (!bAnnounced60 && distNow < 60)
	{
		add_to_playlist("fifty.wav");
		bAnnounced60 = true;
	}
	else if (!bAnnounced50 && distNow < 50)
	{
		add_to_playlist("fourty.wav");
		bAnnounced50 = true;
	}
	else if (!bAnnounced40 && distNow < 40)
	{
		add_to_playlist("toolow.wav");
		add_to_playlist("delay100.wav");
		add_to_playlist("thirty.wav");
		bAnnounced40 = true;
	}
	else if (!bAnnounced30 && distNow < 30)
	{
		add_to_playlist("twenty.wav");
		bAnnounced30 = true;
	}
	else if (!bAnnounced20 && distNow < 20)
	{
		bAnnounced20 = true;
		add_to_playlist("ten.wav");
	}
	else if (!bAnnounced10 && distNow < 10)
	{
		bAnnounced10 = true;
		add_to_playlist("danger.wav");
	}
	
}


//-------------------------------------------------------------

void * reportingMode(BUTTON_EVENT some_key)
{
	int dist;
	int course;
	const char *dir;
	static KEY_STATE here;
	const char *cardinal;


	if (!(int)iLocation.lat || !(int) iLocation.lng)
	{
		//Serial.printf("ilat=%f ilng=%f\n", iLocation.lat, iLocation.lng);
		return (void*)reportingMode;
	}
	
	dist = findNearestCamera(iLocation.lat, iLocation.lng);

	// if vehicle location not known, return negative dist.
	if (dist < 0) return (void*) reportingMode;

	course = (int)gps.courseTo(iLocation.lat, iLocation.lng, closestCam->lat, closestCam->lng);
	cardinal = gps.cardinal(course);

	//speakSpeed(iMisc.Kmph);
	speakDistance(dist);
	
	cprintf(_WHITE, 0, "%s", closestCam->onStreet);
	cprintf(_WHITE, 1, "%s",  closestCam->crossStreet);
	cprintf(dist > 100 ? _GREEN : _PINK, 2, "DIST=%4d m %3d %s", dist, course, cardinal);

	xprintf(3, "%VEH=%3d kph Qual=%s", (int)iMisc.Kmph, iMisc.cQuality);

	cprintf(_GREEN, 4, "NOW LA=%+9.7f", gpsAverage.lat);
	cprintf(_GREEN, 5, "NOW LO=%+9.7f", gpsAverage.lng);

    bool isCharging = M5.Power.isCharging();
    int vol_per = M5.Power.getBatteryLevel();
    int vol = M5.Power.getBatteryVoltage();
    int cur = M5.Power.getBatteryCurrent();
	cprintf(_YELLOW, 6, "%3.1fv %d%% %dmA %s", 
						(float)vol/1000.,
						vol_per,
						cur,
						isCharging ? "CHG":"DIS");
	
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
				colourBarX(_CYAN, 10);
				cprintf(_ORANGE, 6, "TODO LEFT");
			}
			
		break;

		case RBUTTON_UP:
		case RBUTTON_DN:

			if (some_key == RBUTTON_DN)
			{
				colourBarX(_BLUE, 10);
				cprintf(_BLUE, 6, "TODO RIGHT");
			}

		break;

		case MBUTTON_DN:
		case MBUTTON_UP:

			if (some_key == MBUTTON_DN)
			{
				colourBarX(_PINK, 10);
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
