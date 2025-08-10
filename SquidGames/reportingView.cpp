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
	static bool bAnnounced0;
	static bool bAnnounced20;
	static bool bAnnounced40;
	static bool bAnnounced60;
	static bool bAnnounced80;
	static bool bAnnounced100;
	static bool bAnnounced150;
	static bool bAnnounced200;
	static bool bAnnounced300;

	int i;
	
#ifdef LONGVIEW
	if (bTargetHasChanged || distNow > 300)
#else
	if (bTargetHasChanged || distNow > 150)
#endif
	{
		// '300' always largest than farthest reporting distance
		bTargetHasChanged = false;
		bAnnounced0 = false;
		bAnnounced20 = false;
		bAnnounced40 = false;
		bAnnounced60 = false;
		bAnnounced80 = false;
		bAnnounced100 = false;
		bAnnounced150 = false;
		bAnnounced200 = false;
		bAnnounced300 = false;

		//toggleLeftRight(_BLACK,_BLACK);
		return;
	}	

	//toggleLeftRight(_RED,_BLUE);

	if (0)
	{
	}
#ifdef LONGVIEW
	else if ( !bAnnounced300 && distNow < 300)
	{
		add_to_playlist("three.wav");
		add_to_playlist("hundred.wav");
		bAnnounced300 = true;
	}
	else if ( !bAnnounced200 && distNow < 200)
	{
		add_to_playlist("dangerAhead.wav");
		add_to_playlist("two.wav");
		add_to_playlist("hundred.wav");
		bAnnounced200 = true;
	}
#endif
	else if ( !bAnnounced150 && distNow < 150)
	{
		add_to_playlist("one.wav");
		add_to_playlist("hundred.wav");
		add_to_playlist("fifty.wav");
		bAnnounced150 = true;
	}
	else if ( !bAnnounced100 && distNow < 100)
	{
		add_to_playlist("glideSlope.wav");
		add_to_playlist("delay100.wav");
		add_to_playlist("one.wav");
		add_to_playlist("hundred.wav");
		bAnnounced100 = true;
	}

	else if (!bAnnounced80 && distNow < 80)
	{
		add_to_playlist("eighty.wav");
		bAnnounced80 = true;
	}
	else if (!bAnnounced60 && distNow < 60)
	{
		add_to_playlist("sixty.wav");
		bAnnounced60 = true;
	}
	else if (!bAnnounced40 && distNow < 40)
	{
		add_to_playlist("toolow.wav");
		add_to_playlist("delay100.wav");
		add_to_playlist("fourty.wav");
		bAnnounced40 = true;
	}
	else if (!bAnnounced20 && distNow < 20)
	{
		add_to_playlist("twenty.wav");
		bAnnounced20 = true;
	}
	else if (!bAnnounced0 && distNow < 10)
	{
		add_to_playlist("danger.wav");
		bAnnounced0 = true;
	}
}


//-------------------------------------------------------------
static bool bHaveAway = false;
static bool bHaveCamera = false;
static bool bFirstPressAway = false;
static bool bFirstPressCamera = false;

void * reportingMode(BUTTON_EVENT some_key)
{
	int dist;
	int course;
	const char *dir;
	static KEY_STATE here;
	const char *cardinal;
	int 	cam_course;
	char 	const *veh_cardinal;


	if (!(int)iLocation.lat || !(int) iLocation.lng)
	{
		//Serial.printf("ilat=%f ilng=%f\n", iLocation.lat, iLocation.lng);

		// when gps is stable, then process keys.		
		button_push(some_key);
		
		return (void*)reportingMode;
	}
	
	dist = findNearestCamera(iLocation.lat, iLocation.lng);

	// if vehicle location not known, a negative dist is returned
	if (dist < 0) return (void*) reportingMode;

	course = (int)gps.courseTo(iLocation.lat, iLocation.lng, closestCam->lat, closestCam->lng);
	cardinal = gps.cardinal(course);

	//speakSpeed(iMisc.Kmph);
	speakDistance(dist);
	
	cprintf(_WHITE, 0, "%s", closestCam->onStreet);
	cprintf(_WHITE, 1, "%s",  closestCam->crossStreet);
	cprintf(dist > 100 ? _GREEN : _YELLOW, 2, "DIST=%4d m %3d %s", dist, course, cardinal);

	xprintf(3, "%VEH=%03d kph Qual=%s", (int)iMisc.Kmph, iMisc.cQuality);

	cprintf(_GREEN, 4, "NOW LA=%+9.7f", gpsAverage.lat);
	cprintf(_GREEN, 5, "NOW LO=%+9.7f", gpsAverage.lng);

    bool isCharging = M5.Power.isCharging();
    int percent = M5.Power.getBatteryVoltage() * 100/ 3700; // 3.7 v bat max
    int vol = M5.Power.getBatteryVoltage();
    int cur = M5.Power.getBatteryCurrent();
	
	cprintf(_YELLOW, 6, "%3.1fv %03d%% %4dmA %s", 
						(float)vol/1000.,
						percent,
						cur,
						isCharging ? "CHG":"DIS");
	
	// all display updates done ... just keys left
	if (some_key == DISPLAY_REFRESH) return (void*) reportingMode;

	Serial.printf("handled key %d\n", some_key);

	switch (some_key)
	{
		case BUTTON_INIT:
			lfillRect(0,0, 50, 50, _RED);
			threeButtonText("AWAY", "SAVE", "CAMERA");
			//threeButtonText("QUIET", "OK", "BYTEME");
			
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
				if (bHaveAway)
				{
					// double press, cancel both
					bHaveAway = false;
					bHaveCamera = false;
					bFirstPressAway = false;
					bFirstPressCamera = false;
					cprintf(_ORANGE, 7, "NEXT CAMERA or AWAY");
					setToggleColors(_BLACK, _BLACK);
				}
				else
				{
					if (!bFirstPressCamera ) bFirstPressAway = true;
					bHaveAway = true;
					awayLocation = gpsAverage;
					setToggleColors(_GREEN, bHaveCamera ? _RED : _BLACK, 10);
					cprintf(_ORANGE, 7, "NEXT CAMERA or SAVE");
				}
			}

		break;

		case RBUTTON_UP:
		case RBUTTON_DN:

			if (some_key == RBUTTON_DN)
			{
				if (bHaveCamera )
				{
					// double press, cancel both
					bHaveAway = false;
					bHaveCamera = false;
					bFirstPressAway = false;
					bFirstPressCamera = false;
					setToggleColors(_BLACK, _BLACK);
					cprintf(_ORANGE, 7, "NEXT CAMERA or AWAY");
				}
				else
				{
					if (!bFirstPressAway) bFirstPressCamera = true;
					bHaveCamera = true;
					cameraLocation = iLocation;
					setToggleColors(_RED, bHaveAway? _GREEN : _BLACK, 10);
					cprintf(_ORANGE, 7, "NEXT AWAY or SAVE");
				}
			}
		break;

		case MBUTTON_DN:
		case MBUTTON_UP:

			if (some_key == MBUTTON_DN)
			{
				if (bHaveAway && bHaveCamera)
				{
					Serial.printf("first key pressed was %s\n", bFirstPressAway ? "AWAY" : "CAMERA");
					setToggleColors(_BLACK, _BLACK);
					bHaveAway = false;
					bHaveCamera = false;
					bFirstPressAway = false;
					bFirstPressCamera = false;

					double delta_dist = gps.distanceBetween(cameraLocation.lat, cameraLocation.lng, 
															awayLocation.lat,   awayLocation.lng );
					if (bFirstPressAway)
						cam_course = (int)gps.courseTo(	cameraLocation.lat, cameraLocation.lng,
													   	awayLocation.lat, awayLocation.lng);
					else
						cam_course = (int)gps.courseTo(	awayLocation.lat, awayLocation.lng, 
														cameraLocation.lat, cameraLocation.lng);

					veh_cardinal = gps.cardinal(cam_course);

					GPS_ENTRY2 userData;
					userData.lat = cameraLocation.lat;
					userData.lng = cameraLocation.lng;
					userData.bearing = cam_course;
					
					strcpy(userData.cardinal, veh_cardinal);
					strcpy(userData.onStreet, "TBD");
					strcpy(userData.crossStreet, "TBD");
					LINE;
					
					removeNearbyCamera(cameraLocation.lat, cameraLocation.lng);
					
					addToCameraList(&userData);
				}
				else
				{
					if (!bHaveAway && !bHaveCamera)
					{
						cprintf(_ORANGE, 7, "need CAMERA *AND* AWAY");
					}
					else if (bHaveAway)
						cprintf(_ORANGE, 7, "NO! STILL NEED CAMERA");
					else
						cprintf(_ORANGE, 7, "NO! STILL NEED AWAY");
				}
				//return (void*) learningMode;
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
