extern void setup_locate(void);

typedef struct  
{        float lat; 
         float lng; 
         short bearing;
         char cardinal[8];
         char onStreet[30]; 
         char crossStreet[30];
} GPS_ENTRY2;

extern GPS_ENTRY2 *closestCam;
extern GPS_ENTRY2 *nextClosestCam;

// distance to closest camera is returned.
extern int findNearestCamera(float vehicleLat, float vehicleLng);
extern int32_t writeToSD(char* filename = "backup.db" );


