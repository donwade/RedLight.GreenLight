#ifndef VIEW_CONTROLLER
#define VIEW_CONTROLLER

#include "cppQueue.h"
typedef enum KEY_STATE { KEY_UNKNOWN, KEY_DOWN, KEY_UP};


extern int  xprintf(uint8_t lineNo, const char *format, ...); 

extern cppQueue buttonQueue;
extern SemaphoreHandle_t keyCountingSemaphore;

void touchPanel_impl(void);
void setup_button(void);
void button_create(void);

typedef enum { 
	MT, //nothing
	LBUTTON_UP, LBUTTON_DN, 
	MBUTTON_UP, MBUTTON_DN, 
	RBUTTON_UP, RBUTTON_DN } BUTTON_EVENT;

typedef struct {
	BUTTON_EVENT key;
} BUTTON_MESSAGE;

void threeButtonMenu(
	char *leftButtonText, 
	char *middleButtonText, 
	char *rightButtonText 
	);


void twoButtonMenu(
    char *leftButtonText, 
    char *rightButtonText 
    );


#endif
