#include <M5Unified.h>
#include <TinyGPS++.h>

extern void setup_locate(void);

typedef struct  
{        float lat; 
         float lng; 
         short bearing;
         char cardinal[8];
         char onStreet[30]; 
         char crossStreet[30];
} GPS_ENTRY2;

extern TinyGPSPlus gps;

extern GPS_ENTRY2 *closestCam;
extern GPS_ENTRY2 *nextClosestCam;

// distance to closest camera is returned.
extern int findNearestCamera(float vehicleLat, float vehicleLng);
extern int32_t writeToSD(char* filename = "backup.db" );

typedef struct gpsLocation { double lng; double lat; };

typedef struct gpsMisc 
{
	float speed;
	const char *cardinal;
	
	float qual;
	const char  *cQuality;
	
	float Kmph;
	float course;		//direction in float degrees
	
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
};
	
extern gpsLocation iLocation;
extern gpsMisc     iMisc;
extern gpsLocation cameraLocation;
extern gpsLocation endLocation;
extern gpsLocation gpsAverage;
extern bool bNewTarget;

