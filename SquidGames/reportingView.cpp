#include "locate.h"
#include "viewController.h"
#include "wavePlayer.h"

#define LINE Serial.printf("%s:%d\n", __FUNCTION__, __LINE__)
extern void * learningMode(BUTTON_EVENT x);
extern void * savingMode(BUTTON_EVENT x);

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
	int Vdist;
	int Vcourse;
	const char *dir;
	static KEY_STATE here;
	static int ticker;
	const char *Vcardinal;
	int 	cam_course;
	char 	const *veh_cardinal;


	if (!(int)iLocation.lat || !(int) iLocation.lng)
	{
		//Serial.printf("ilat=%f ilng=%f\n", iLocation.lat, iLocation.lng);

		// when gps is stable, then process keys.		
		button_push(some_key);

		colourBarX(_RED, 7);
		showPower();
		
		return (void*)reportingMode;
	}
	
	Vdist = findNearestCamera(iLocation.lat, iLocation.lng);

	// if vehicle location not known, a negative Vdist is returned
	if (Vdist < 0) return (void*) reportingMode;

	Vcourse = (int)gps.courseTo(iLocation.lat, iLocation.lng, 
								targetCamera.lat, targetCamera.lng);
	Vcardinal = gps.cardinal(Vcourse);

	//speakSpeed(iMisc.Kmph);
	//speakDistance(Vdist);
	
	cprintf(_WHITE, 0, "%s", targetCamera.onStreet);
	cprintf(_WHITE, 1, "%s",  targetCamera.crossStreet);

	cprintf(Vdist > 100 ? _GREEN : _YELLOW, 2, "TDIST=%4d VSPD=%3d", Vdist, (int)iMisc.Kmph);
	
	cprintf(_CYAN,  3, "VEH %3d %s", Vcourse, Vcardinal);
	cprintf(_CYAN,  4, "TGT %3d %s", targetCamera.bearing, targetCamera.cardinal);

	xprintf(5, "Qual=%5s %d", iMisc.cQuality, ticker++);
	//cprintf(_GREEN, 4, "NOW LA=%+9.7f", gpsAverage.lat);
	//cprintf(_GREEN, 5, "NOW LO=%+9.7f", gpsAverage.lng);

	
	showPower();
	
	// all display updates done ... just keys left
	if (some_key == DISPLAY_REFRESH) return (void*) reportingMode;

	//Serial.printf("handled key %d\n", some_key);

	switch (some_key)
	{
		case BUTTON_INIT:
			lfillRect(0,0, 50, 50, _RED);
			threeButtonText("AWAY", "SAVE", "CAMERA");
			
			cprintf(_RED,	2, "TODO        ");
			cprintf(_RED ,	3, "TODO        ");
			cprintf(_GREEN, 4, "TODO        ");
			cprintf(_GREEN, 5, "TODO        ");
			cprintf(_ORANGE,6, "TODO        ");
			setToggleColors(_BLACK, _BLACK);
			
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

					
					return (void*) reportingMode;
				}
				else
				{
					if (!bHaveAway && !bHaveCamera)
					{
						cprintf(_ORANGE, 7, "need CAMERA *AND* AWAY");

						// hitting save with no endpoints ?
						// assume delete nearest camera to current veh location
						
						removeNearbyCamera(gpsAverage.lat, gpsAverage.lng);
						
						// ensure deletion sticks across next reboot
						copyCameraListToSD("gps.db");
						
						return (void*) reportingMode;
					}
					else if (bHaveAway)
						cprintf(_ORANGE, 7, "NO! STILL NEED CAMERA");
					else
						cprintf(_ORANGE, 7, "NO! STILL NEED AWAY");
				}
			}			
			
		break;

	}
	return (void *) reportingMode;
}	
