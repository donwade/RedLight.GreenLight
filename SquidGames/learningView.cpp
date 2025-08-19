#include "locate.h"
#include "viewController.h"

extern void * reportingMode(BUTTON_EVENT x);

#define LINE Serial.printf("%s:%d\n", __FUNCTION__, __LINE__)

void * learningMode(BUTTON_EVENT some_key)
{
	int dist;
	int course;
	const char *dir;
	static KEY_STATE here;
	
#if 0
	if (iMisc.Kmph > 5)
		xprintf(2, "%3d %s", veh_course, veh_cardinal);
	else
		xprintf(2, "%2d/%2d/%4d S=%2d", gps.date.day(), gps.date.month(), 
				gps.date.year(), gps.satellites.value());
#endif	
	cprintf(_WHITE, 0, "NOW    LA=%+10.7f", gpsAverage.lat);
	cprintf(_WHITE, 1, "NOW    LN=%+10.7f", gpsAverage.lng);
	cprintf(_RED, 2,   "CAMERA LA=%+9.7f", cameraLocation.lat);
	cprintf(_RED ,3,   "CAMERA LO=%+9.7f", cameraLocation.lng);
	cprintf(_GREEN, 4, "AWAY   LA=%+9.7f", awayLocation.lat);
	cprintf(_GREEN, 5, "AWAY   LO=%+9.7f", awayLocation.lng);

	dist = gps.distanceBetween(cameraLocation.lat, cameraLocation.lng, awayLocation.lat, awayLocation.lng);
	course = (int)gps.courseTo(cameraLocation.lat, cameraLocation.lng, awayLocation.lat, awayLocation.lng);
	dir = gps.cardinal(course);
	
	cprintf(_YELLOW, 6, "course = %d dir=%3s", course, dir);
	
	// all display updates done ... just keys left
	if (some_key == DISPLAY_REFRESH) return (void*) learningMode;
	
	Serial.printf("handled key %d\n", some_key);

	switch (some_key)
	{
		case BUTTON_INIT:
			lfillRect(0,0, 50, 50, _BLUE);
			threeButtonText("AWAY", "NEXT", "CAMERA");
			cprintf(_RED,	2, "LA=%+9.7f", gpsAverage.lat);
			cprintf(_RED ,	3, "LO=%+9.7f", gpsAverage.lng);
			cprintf(_GREEN, 4, "LA=%+9.7f", gpsAverage.lat);
			cprintf(_GREEN, 5, "LO=%+9.7f", gpsAverage.lng);
			cprintf(_ORANGE,6, "CAMERA or AWAY");
		break;	
			
		case LBUTTON_UP:
		case LBUTTON_DN:
			if (some_key == LBUTTON_DN)
			{
				awayLocation = gpsAverage;
				colourBarX(_GREEN, 10);
				cprintf(_ORANGE, 6, "SELECT CAMERA or NEXT");
			}
			
		break;

		case RBUTTON_UP:
		case RBUTTON_DN:
			if (some_key == RBUTTON_DN)
			{
				cameraLocation = gpsAverage;
				colourBarX(_RED, 10);
				cprintf(_ORANGE, 6, "SELECT AWAY or NEXT");
			}

		break;

		case MBUTTON_DN:
		case MBUTTON_UP:
			
			if (some_key == MBUTTON_DN)
			{
				colourBarX(_YELLOW, 10);
				return (void*) reportingMode;
			}			
			break;
		break;

	}
	return (void*) learningMode;
}

