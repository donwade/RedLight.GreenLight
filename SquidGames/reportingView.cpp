#include "locate.h"
#include "viewController.h"
#include "wavePlayer.h"

#define LINE Serial.printf("%s:%d\n", __FUNCTION__, __LINE__)
extern void * learningMode(BUTTON_EVENT x);
extern void * savingMode(BUTTON_EVENT x);

void speakSpeed(int Kmph)
{
	
}

void speakDistance(int distNow, int16_t AOA)
{
	static bool bAnnounced20;
	static bool bAnnounced40;
	static bool bAnnounced60;
	static bool bAnnounced80;
	static bool bAnnounced100;
	static bool bAnnounced150;
	static bool bAnnounced200;
	static bool bAnnounced300;
	static bool bAOAsounded;

	int i;
	
#ifdef LONGVIEW
	if (bTargetHasChanged || distNow > 300)
#else
	if (bTargetHasChanged || distNow > 150)
#endif
	{
		// '300' always largest than farthest reporting distance
		bTargetHasChanged = false;
		bAnnounced20 = false;
		bAnnounced40 = false;
		bAnnounced60 = false;
		bAnnounced80 = false;
		bAnnounced100 = false;
		bAnnounced150 = false;
		bAnnounced200 = false;
		bAnnounced300 = false;
		bAOAsounded = false;

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
#endif
	else if ( !bAnnounced200 && distNow < 200)
	{
		add_to_playlist("bankAngle.wav");
		add_to_playlist("two.wav");
		add_to_playlist("hundred.wav");
		bAnnounced200 = true;
	}
	else if ( !bAnnounced150 && distNow < 150)
	{
		add_to_playlist("one.wav");
		add_to_playlist("hundred.wav");
		add_to_playlist("fifty.wav");
		bAnnounced150 = true;
	}
	else if ( !bAnnounced100 && distNow < 100)
	{
		bAnnounced100 = true;
		add_to_playlist("one.wav");
		add_to_playlist("hundred.wav");
		
		if (!bAOAsounded)
		{
			bAOAsounded = true;
			Serial.printf("AOA = %d\n", AOA);
			if (abs(AOA) > 160 )
				add_to_playlist("dangerAhead.wav");
			else if (abs(AOA < 20))
				add_to_playlist("behindYou.wav");
			else
				add_to_playlist("crossStreetWarning.wav");
		}
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
		add_to_playlist("danger.wav");
		bAnnounced20 = true;
	}
}


//-------------------------------------------------------------
static bool bHaveAway = false;
static bool bHaveCamera = false;
static bool bFirstPressAway = false;
static bool bFirstPressCamera = false;

void saveCamera(void)
{
	int 	cam_course;
	char 	const *veh_cardinal;
	
	double delta_dist = gps.distanceBetween(cameraLocation.lat, cameraLocation.lng, 
											awayLocation.lat,	awayLocation.lng );
	if (bFirstPressAway)
		cam_course = (int)gps.courseTo( cameraLocation.lat, cameraLocation.lng,
										awayLocation.lat, awayLocation.lng);
	else
		cam_course = (int)gps.courseTo( awayLocation.lat, awayLocation.lng, 
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
	copyCameraListToSD("gps.db");

	bHaveAway = false;
	bHaveCamera = false;
	bFirstPressAway = false;
	bFirstPressCamera = false;
	
}

int vehicalDirection;
const char *vehicalCardinal;
int targetDistance;
int targetBearing;
const char *targetCardinal;


void * reportingMode(BUTTON_EVENT some_key)
{
	static KEY_STATE here;

	if (!(int)iLocation.lat || !(int) iLocation.lng)
	{
		//Serial.printf("ilat=%f ilng=%f\n", iLocation.lat, iLocation.lng);

		// when gps is stable, then process keys.		
		button_push(some_key);

		colourBarX(_RED, 7);
		showPower();
		
		return (void*)reportingMode;
	}
	
	targetDistance = findNearestCamera(iLocation.lat, iLocation.lng);

	// if vehicle location not known, a negative targetDistance is returned
	if (targetDistance < 0) return (void*) reportingMode;

	targetBearing = (int)gps.courseTo(iLocation.lat, iLocation.lng, 
								targetCamera.lat, targetCamera.lng);
	targetCardinal = gps.cardinal(targetBearing);

	vehicalDirection = (int)gps.course.deg();
	vehicalCardinal =  gps.cardinal(gps.course.deg());

	//angle of attack
	int16_t AOA =	angle_diff(vehicalDirection,targetBearing);
	
	//speakSpeed(iMisc.Kmph);
	speakDistance(targetDistance, AOA);
	
	cprintf(_WHITE, 0, "%s", targetCamera.onStreet);
	cprintf(_WHITE, 1, "%s",  targetCamera.crossStreet);

	if (iMisc.cQuality) cprintf(_MAGENTA, 2, "sats= %02d qual = %s", iMisc.sats, iMisc.cQuality);
	
	cprintf(_CYAN,  3, "VEH  %3d Kph %3s %3d", (int)iMisc.Kmph, vehicalCardinal, vehicalDirection);
	cprintf(_CYAN,  4, "TGT %4d m   %3s %3d", targetDistance, targetCardinal, targetBearing);

	cprintf(targetDistance > 100 ? _GREEN : _YELLOW, 5, "Angle=%d Dist=%5dm",
				min(targetDistance,999) , AOA);
	
	//cprintf(_GREEN, 4, "NOW LA=%+9.7f", gpsAverage.lat);
	//cprintf(_GREEN, 5, "NOW LO=%+9.7f", gpsAverage.lng);

	
	showPower();
	
	// all display updates done ... just keys left
	if (some_key == DISPLAY_REFRESH) return (void*) reportingMode;

	//Serial.printf("handled key %d\n", some_key);

	switch (some_key)
	{
		case BUTTON_INIT:
			//lfillRect(0,0, 50, 50, _RED);
			threeButtonText("AWAY", "NEXT", "CAMERA");
			
			cprintf(_RED,	2, "TODO           ");
			cprintf(_RED ,	3, "TODO           ");
			cprintf(_GREEN, 4, "TODO           ");
			cprintf(_GREEN, 5, "TODO           ");
			cprintf(_ORANGE,6, "TODO           ");
			cprintf(_ORANGE, 7, "need CAMERA *AND* AWAY");
			setToggleColors(_BLACK, _BLACK);
			
		break;	
		
		// aka 'away button'
		case LBUTTON_UP:
		case LBUTTON_DN:
			if (some_key == LBUTTON_DN)
			{
				awayLocation = gpsAverage;
				
				if (bHaveCamera)
				{
					LINE;
					// save operation
					bHaveAway = false;
					bHaveCamera = false;
					bFirstPressAway = false;
					bFirstPressCamera = false;
					
					cprintf(_ORANGE, 7, "SELECT CAMERA or AWAY");
					setToggleColors(_CYAN, _CYAN, 2);

					saveCamera();
					break;
				}
				
				if (!bHaveAway)
				{
					LINE;
					bHaveAway = true;
					if (!bFirstPressCamera) bFirstPressAway = true;
					
					
					cprintf(_ORANGE, 7, "MOVE 2 AND MARK CAMERA");
					setToggleColors(_RED, _BLACK, 10);
					break;
				}
				
				else
				{
					LINE;
					// cancel op
					bHaveAway = false;
					bFirstPressAway = false;
					setToggleColors(_BLACK , _BLACK, 10);
					cprintf(_ORANGE, 7, "SELECT CAMERA or AWAY");
				}
			}

		break;

		// camera button
		case RBUTTON_UP:
		case RBUTTON_DN:

			if (some_key == RBUTTON_DN)
			{
				cameraLocation = iLocation;
				if (bHaveAway)
				{
					LINE;
					// save operation
					bHaveAway = false;
					bHaveCamera = false;
					bFirstPressAway = false;
					bFirstPressCamera = false;
					setToggleColors(_CYAN, _CYAN, 2);
					cprintf(_ORANGE, 7, "SELECT CAMERA or AWAY");

					saveCamera();
					break;
				}
				
				if (!bHaveCamera)
				{
					LINE;
					bHaveCamera = true;
					if (!bFirstPressAway) bFirstPressCamera = true;

					cprintf(_ORANGE, 7, "MARK AWAY FROM CAM");
					setToggleColors(_GREEN, _BLACK, 10);
					break;
				}
				
				else
				{
					LINE;
					// cancel op
					bHaveCamera = false;
					bFirstPressCamera= false;
					setToggleColors(_BLACK , _BLACK, 10);
					cprintf(_ORANGE, 7, "SELECT CAMERA or AWAY");
				}

			}
		break;

		case MBUTTON_DN:
		case MBUTTON_UP:

			if (some_key == MBUTTON_DN)
			{
				setToggleColors(_BLACK, _BLACK);
				bHaveAway = false;
				bHaveCamera = false;
				bFirstPressAway = false;
				bFirstPressCamera = false;
				
				return (void*) savingMode;
			}			
			
		break;

	}
	return (void *) reportingMode;
}	
