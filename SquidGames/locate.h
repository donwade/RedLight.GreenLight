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

// distance to closest camera is returned.
extern int     findNearestCamera(float vehicleLat, float vehicleLng);
extern int32_t copyCameraListToSD(char* filename = "backup.db" );
extern int32_t addToCameraList(GPS_ENTRY2 *data);
extern int quickSearchDistance(float userLat, float userLng);
extern int removeNearbyCamera(float userLat, float userLng);

extern GPS_ENTRY2 targetCamera;  // allow anyone to see closest cam



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
extern gpsLocation awayLocation;

extern gpsLocation gpsAverage;
extern bool bTargetHasChanged;


