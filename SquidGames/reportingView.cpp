#include "locate.h"
#include "viewController.h"

#define LINE Serial.printf("%s:%d\n", __FUNCTION__, __LINE__)
extern void * learningMode(BUTTON_EVENT x);

void * reportingMode(BUTTON_EVENT some_key)
{
	int dist;
	int course;
	const char *dir;
	static KEY_STATE here;
	const char *cardinal;


	if (!(int)iLocation.lat || !(int) iLocation.lng)
	{
		Serial.printf("ilat=%f ilng=%f\n", 
			iLocation.lat,
			iLocation.lng);
		return (void*)reportingMode;
	}
	
	int ret = findNearestCamera(iLocation.lat, iLocation.lng);

	// if vehicle location not known, return negative dist.
	if (ret < 0) return (void*) reportingMode;

/*
	Serial.printf("ilat=%f ilng=%f pGPS=%p clat=%f clng=%f\n", 
		iLocation.lat,
		iLocation.lng,
		closestCam,
		closestCam->lat,
		closestCam->lng);
*/

	course = (int)gps.courseTo(iLocation.lat, iLocation.lng, closestCam->lat, closestCam->lng);
	cardinal = gps.cardinal(course);

	dist = (int) gps.distanceBetween(iLocation.lat, iLocation.lng, closestCam->lat, closestCam->lng);
	
	cprintf(_WHITE, 0, "%s", closestCam->onStreet);
	cprintf(_WHITE, 1, "%s",  closestCam->crossStreet);
	cprintf(dist > 100 ? _GREEN : _RED, 2, "DIST=%4d m %3d %s", dist, course, cardinal);

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
