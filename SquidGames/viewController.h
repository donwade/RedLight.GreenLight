#ifndef VIEW_CONTROLLER
#define VIEW_CONTROLLER

#include "cppQueue.h"

#define _BLACK       0x000000
#define _WHITE       0xFFFFFF
#define _RED         0xFF0000
#define _GREEN       0x00FF00
#define _BLUE        0x0000FF

#define _CYAN        (_GREEN | _BLUE)
#define _MAGENTA     (_RED   | _BLUE)
#define _YELLOW      (_RED   | _GREEN)

#define _ORANGE      (_RED | (_GREEN/2))
#define _NAVY        (_BLUE/2)
#define _MAROON      (_RED/2)
#define _DGREEN      (_GREEN/2)
#define _DCYAN       (_CYAN/2)
#define _OLIVE       ( (_RED/2) | (_GREEN/2))
#define _PINK        ( _RED | _GREEN | (_BLUE/20))

typedef enum KEY_STATE { KEY_UNKNOWN, KEY_DOWN, KEY_UP};

extern int  xprintf(uint8_t lineNo, const char *format, ...); 
extern int  cprintf(uint32_t color, uint8_t lineNo, const char *format, ...);

extern cppQueue buttonQueue;
extern SemaphoreHandle_t keyCountingSemaphore;

void touchPanel_impl(void);
void setup_button(void);
void button_create(void);

typedef enum { 
	MT, //nothing
    BUTTON_INIT,
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

void threeButtonText(
	char *leftButtonText, 
	char *middleButtonText, 
	char *rightButtonText);

#endif
