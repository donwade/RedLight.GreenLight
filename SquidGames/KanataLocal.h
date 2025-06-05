
typedef struct  
{        float lat; 
         float lng; 
         short bearing;
         const char *cardinal;
         const char *onStreet; 
         const char *crossStreet;
} GPS_ENTRY;

const GPS_ENTRY cameraLocations[] = 
{
   { +45.2945245,-75.8617220 , 211, "SSW", "Aintree" , "Bridlewood" }, 
   { +45.2966599,-75.8601638 , 207, "SSW", "Steeplechase" , "Yoho" }, 
   { +45.2968685,-75.8598877 , 117, "ESE", "Yoho" , "Steeplechase" }, 
   { +45.2992856,-75.8582625 , 213, "SSW", "Steeplechase" , "SpringWater" }, 
   { +45.3004119,-75.8538287 , 302, "WNW", "Steeplechase" , "MattawaPk" }, 
   { +45.2982076,-75.8532214 , 265,   "W", "Forllon" , "SteepleChase" }, 
   { +45.2968523,-75.8528757 , 134,  "SE", "Pinehill" , "Steeplechase" }, 
   { +45.2942913,-75.8568575 ,  36,  "NE", "SteepleChase" , "Sauble" }, 
   { +45.2913518,-75.8587188 ,   7,   "N", "SteepleChase" , "Roundabout" }, 
   { +45.2901882,-75.8511637 , 296, "WNW", "Stonehaven" , "Furlong" }, 
   { +45.2889270,-75.8491796 , 321,  "NW", "Stonehavem" , "Forest" }, 
   { +45.2838663,-75.8458203 , 351,   "N", "Stonehaven" , "Roch PS" }, 
   { +45.2806748,-75.8402059 , 177,   "S", "PineLock" , "Stonehaven" }, 
   { +45.2808447,-75.8377944 , 267,   "W", "Stonehaven" , "Old Richmond" }, 
   { +45.2952270,-75.8346416 ,  12, "NNE", "Old Richmond" , "NCC trail" },

   { +45.2871659,-75.8473584 , 155, "SSE", "Stonehaven" , "Elizabeth PS" }, 
   { +45.2900872,-75.8506806 , 135,  "SE", "Stonehaven" , "PineHill" }, 
   { +45.2911094,-75.8572635 ,  91,   "E", "Stonehaven" , "BridgeStone" }, 
   { +45.2967850,-75.8532165 , 236,  "SW", "SteepleChase" , "PineHill" },

   { +45.3008710,-75.8543878 , 130,  "SE", "SteepleChase" , "Mattawa" }, 
   { +45.2996409,-75.8580272 ,  49,  "NE", "SteppleChase" , "SpringWater" }, 
   { +45.2948422,-75.8642632 ,  71, "ENE", "Bridlewood" , "Aintree" }, 
};

#define  NUM_GPS_ENTRIES (sizeof(cameraLocations)/sizeof(cameraLocations[0]))

