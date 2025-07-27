#include <M5Unified.h>
#include "viewController.h"
#include "watchdogs.h"

m5::touch_detail_t touchDetail;
LGFX_Button buttonLeft, buttonMiddle, buttonRight;


static unsigned char buttonWidth = 60;
static unsigned char buttonHeight = 60;

static unsigned int phyDispWidth;
static unsigned int phyDispHeigth;


ptrKeyWrite g_evLeftNotify;
ptrKeyWrite g_evMiddleNotify;
ptrKeyWrite g_evRightNotify;

int8_t g_stateLeftButton = -1;
int8_t g_stateRightButton = -1;
int8_t g_stateMiddleButton = -1;

void button_create()
{
	buttonWidth = phyDispWidth /3;
	
	// coordinates specify center of button hence odd math
	buttonLeft.initButton(&M5.Lcd,	buttonWidth * 0 + buttonWidth/2, 215, buttonWidth, buttonHeight, TFT_WHITE, TFT_GREEN, TFT_BLACK, "CAMERA", 1, 1);
	buttonLeft.drawButton();
	buttonMiddle.initButton(&M5.Lcd, buttonWidth * 1 + buttonWidth/2, 215, buttonWidth, buttonHeight, TFT_WHITE, TFT_YELLOW, TFT_BLACK, "AWAY", 1, 1);
	buttonMiddle.drawButton();
	
	buttonRight.initButton(&M5.Lcd, buttonWidth * 2 + buttonWidth/2 ,215, buttonWidth, buttonHeight, TFT_WHITE, TFT_RED, TFT_BLACK, "SAVE", 1, 1);
	buttonRight.drawButton();
	
}


void threeButtonMenu(
	char *leftButtonText, 
	ptrKeyWrite pLeftNotify,
	char *middleButtonText, 
	ptrKeyWrite pMiddleNotify,
	char *rightButtonText, 
	ptrKeyWrite pRightNotify
	)
{
	buttonWidth = phyDispWidth /3;
	
	// coordinates specify center of button hence odd math
	buttonLeft.initButton(&M5.Lcd,	buttonWidth * 0 + buttonWidth/2, 210, buttonWidth, buttonHeight, TFT_WHITE, TFT_GREEN, TFT_BLACK, leftButtonText, 1, 1);
	buttonLeft.drawButton();
	g_evLeftNotify = pLeftNotify;	
	g_stateLeftButton = KEY_UNKNOWN;
	if (pLeftNotify) *pLeftNotify = KEY_UNKNOWN;
	
	buttonMiddle.initButton(&M5.Lcd, buttonWidth * 1 + buttonWidth/2, 210, buttonWidth, buttonHeight, TFT_WHITE, TFT_YELLOW, TFT_BLACK, middleButtonText, 1, 1);
	buttonMiddle.drawButton();
	g_evMiddleNotify = pMiddleNotify;	
    g_stateMiddleButton = KEY_UNKNOWN;
	if (pMiddleNotify) *pLeftNotify = KEY_UNKNOWN;

	buttonRight.initButton(&M5.Lcd, buttonWidth * 2 + buttonWidth/2 ,210, buttonWidth, buttonHeight, TFT_WHITE, TFT_RED, TFT_BLACK, rightButtonText, 1, 1);
	buttonRight.drawButton();
	g_evRightNotify = pRightNotify;	
	g_stateRightButton = KEY_UNKNOWN;
	if (pRightNotify) *pRightNotify = KEY_UNKNOWN;
	
}

void twoButtonMenu(
	char *leftButtonText, 
	ptrKeyWrite evLeftNotify,
	char *rightButtonText, 
	ptrKeyWrite evRightNotify
	)
{
	buttonWidth = phyDispWidth /2;
	
	// coordinates specify center of button hence odd math
	buttonLeft.initButton(&M5.Lcd,	buttonWidth * 0 + buttonWidth/2, 210, buttonWidth, buttonHeight, TFT_WHITE, TFT_GREEN, TFT_BLACK, leftButtonText, 1, 1);
	buttonLeft.drawButton();
	g_evLeftNotify = evLeftNotify;	
	g_stateLeftButton = KEY_UNKNOWN;
	if (evLeftNotify) *evLeftNotify = KEY_UNKNOWN;
	
	g_evMiddleNotify = NULL;	
    g_stateMiddleButton = -1;

	buttonRight.initButton(&M5.Lcd, buttonWidth * 1 + buttonWidth/2 ,210, buttonWidth, buttonHeight, TFT_WHITE, TFT_RED, TFT_BLACK, rightButtonText, 1, 1);
	buttonRight.drawButton();
	g_evRightNotify = evRightNotify;	
	g_stateRightButton = KEY_UNKNOWN;
	if (evRightNotify) *evRightNotify = KEY_UNKNOWN;
}


KEY_STATE keyDest;

void setup_button()
{
	phyDispWidth = M5.Lcd.width();
	phyDispHeigth = M5.Lcd.height();
	//threeButtonMenu("LEFT", &keyDest, "MIDDLE", &keyDest , "RIGHT", &keyDest);
	//twoButtonMenu("LEFTX", &keyDest, "RIGHTX", &keyDest);
}

void touchPanel_task()
{
	kickDog();

	M5.update();
	touchDetail = M5.Touch.getDetail();

	if (touchDetail.isPressed())
	{
		if(buttonLeft.contains(touchDetail.x, touchDetail.y))
		{
			if (g_evLeftNotify && g_stateLeftButton != KEY_DOWN)
			{
				Serial.println("Left pressed");
				*g_evLeftNotify = KEY_DOWN;
				g_stateLeftButton = KEY_DOWN;
			}
		}
		else if(buttonMiddle.contains(touchDetail.x, touchDetail.y))
		{
			if (g_evMiddleNotify && g_stateMiddleButton != KEY_DOWN)
			{
				Serial.println("Middle pressed");
				*g_evMiddleNotify = KEY_DOWN;
				g_stateMiddleButton = KEY_DOWN;
			}
		}
		else if(buttonRight.contains(touchDetail.x, touchDetail.y))
		{
			if (g_evRightNotify && g_stateRightButton != KEY_DOWN)
			{
				Serial.println("Right pressed");
				*g_evRightNotify = KEY_DOWN;
				g_stateRightButton = KEY_DOWN;
			}
		}
	}

	if (touchDetail.isReleased())
	{
		if(buttonLeft.contains(touchDetail.x, touchDetail.y))
		{
			if (g_evLeftNotify && g_stateLeftButton != KEY_UP)
			{
				Serial.println("Left released");
				*g_evLeftNotify = KEY_UP;
				g_stateLeftButton = KEY_UP;
			}
		}
		else if(buttonMiddle.contains(touchDetail.x, touchDetail.y))
		{
			if (g_evMiddleNotify && g_stateMiddleButton != KEY_UP)
			{
				Serial.println("Middle released");
				*g_evMiddleNotify = KEY_UP;
				g_stateMiddleButton = KEY_UP;
			}
		}
		else if(buttonRight.contains(touchDetail.x, touchDetail.y))
		{
			if (g_evRightNotify && g_stateRightButton != KEY_UP)
			{
				Serial.println("Right released");
				*g_evRightNotify = KEY_UP;
				g_stateRightButton = KEY_UP;
			}
		}
	}

}

