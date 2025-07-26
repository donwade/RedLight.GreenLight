extern uint32_t loadGpsDb(char *database = "gps.db");

typedef struct  
{        float lat; 
         float lng; 
         short bearing;
         char cardinal[8];
         char onStreet[30]; 
         char crossStreet[30];
} GPS_ENTRY2;



