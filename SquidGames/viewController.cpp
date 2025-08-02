#include <M5Unified.h>
#include <m5Core2-only.h>
#include "viewController.h"
#include "watchdogs.h"

static SemaphoreHandle_t displayMutex = xSemaphoreCreateMutex();
SemaphoreHandle_t keyCountingSemaphore;

//-------------------------------------------------------------
void lfillRect(uint16_t x, uint16_t y, uint16_t wide, uint16_t height, uint32_t RGB)
{
	xSemaphoreTake(displayMutex, portMAX_DELAY);
	
	uint16_t w = wide ? wide : M5.Lcd.width();
    uint16_t h = height ? height : M5.Lcd.height();
    M5.Lcd.writeFillRectPreclipped(x, y, w, h, 0x00FFFF);
	
    //M5.Lcd.fillScreen(WHITE);
	
	xSemaphoreGive(displayMutex);	
}


//---------------------------------------------------------
int  xprintf(uint8_t lineNo, const char *format, ...) 
{
	static u_int8_t linelen[10];
	va_list args;
	va_start(args, format);
	char buffer[40];
	
	vsnprintf(buffer, sizeof(buffer)-1, format, args);

	xSemaphoreTake(displayMutex, portMAX_DELAY);

	lsetTextColor(_WHITE, _BLACK);
	lsetCursor(0, lineNo); 

	// time to kill off any chars from old print
	uint32_t ll = strlen(buffer);
	
	if ( ll < linelen[lineNo])
	{
		// overstrike past text with spaces if needed
		uint32_t add = linelen[lineNo] - ll;

		// mono spaced font right :) I'm lazy.
		for (int i = 0; i < add+1; i++) strcat(buffer," ");
	}
	
	linelen[lineNo] = ll;
	lprint(buffer);

	xSemaphoreGive(displayMutex);	
	va_end(args);
	return 0;
}

int  cprintf(uint32_t color, uint8_t lineNo, const char *format, ...) 
{
	static u_int8_t linelen[10];
	va_list args;
	va_start(args, format);
	char buffer[40];
	
	vsnprintf(buffer, sizeof(buffer)-1, format, args);

	xSemaphoreTake(displayMutex, portMAX_DELAY);
	
	lsetTextColor(color, _BLACK);
	lsetCursor(0, lineNo); 

	// time to kill off any chars from old print
	uint32_t ll = strlen(buffer);
	
	if ( ll < linelen[lineNo])
	{
		// overstrike past text with spaces if needed
		uint32_t add = linelen[lineNo] - ll +1 ; //+1 doesnt clear 100.00%

		// mono spaced font right :) I'm lazy.
		for (int i = 0; i < add+1; i++) strcat(buffer," ");
	}
	
	linelen[lineNo] = ll;
	lprint(buffer);

	lsetTextColor(_WHITE, _BLACK);

	xSemaphoreGive(displayMutex);	
	va_end(args);
	return 0;
}





m5::touch_detail_t touchDetail;
LGFX_Button buttonLeft, buttonMiddle, buttonRight;

static unsigned char buttonWidth = 60;
static unsigned char buttonHeight = 60;

static unsigned int phyDispWidth;
static unsigned int phyDispHeigth;

int8_t leftButtonState = -1;
int8_t rightButtonState = -1;
int8_t middleButtonState = -1;

static uint32_t bMenuIsActive = false;

void threeButtonText(
	char *leftButtonText, 
	char *middleButtonText, 
	char *rightButtonText)
{
	xSemaphoreTake(displayMutex, portMAX_DELAY);

	if (leftButtonText) buttonLeft.setLabelText(leftButtonText);
	if (rightButtonText) buttonRight.setLabelText(rightButtonText);
	if (middleButtonText) buttonMiddle.setLabelText(middleButtonText);

	buttonLeft.drawButton();
	buttonRight.drawButton();
	buttonMiddle.drawButton();
	
	xSemaphoreGive(displayMutex);	
	
};


void threeButtonMenu(
	char *leftButtonText, 
	char *middleButtonText, 
	char *rightButtonText)
{
	xSemaphoreTake(displayMutex, portMAX_DELAY);

    M5.Lcd.setTextFont(WIDGET_FONT);

	bMenuIsActive = true;
	buttonWidth = phyDispWidth /3;
	
	// coordinates specify center of button hence odd math
	buttonLeft.initButton(&M5.Lcd,	buttonWidth * 0 + buttonWidth/2, 210, buttonWidth, buttonHeight, TFT_WHITE, TFT_GREEN, TFT_BLACK, leftButtonText, 1, 1);
	buttonLeft.drawButton();
	leftButtonState = KEY_UNKNOWN;
	
	buttonMiddle.initButton(&M5.Lcd, buttonWidth * 1 + buttonWidth/2, 210, buttonWidth, buttonHeight, TFT_WHITE, TFT_YELLOW, TFT_BLACK, middleButtonText, 1, 1);
	buttonMiddle.drawButton();
    middleButtonState = KEY_UNKNOWN;

	buttonRight.initButton(&M5.Lcd, buttonWidth * 2 + buttonWidth/2 ,210, buttonWidth, buttonHeight, TFT_WHITE, TFT_RED, TFT_BLACK, rightButtonText, 1, 1);
	buttonRight.drawButton();
	rightButtonState = KEY_UNKNOWN;

	BUTTON_MESSAGE msg;

	xSemaphoreGive(displayMutex);	
	
}

void twoButtonMenu(
	char *leftButtonText, 
	char *rightButtonText
	)
{
	xSemaphoreTake(displayMutex, portMAX_DELAY);
    M5.Lcd.setTextFont(WIDGET_FONT);

	bMenuIsActive = true;
	buttonWidth = phyDispWidth /2;
	
	// coordinates specify center of button hence odd math
	buttonLeft.initButton(&M5.Lcd,	buttonWidth * 0 + buttonWidth/2, 210, buttonWidth, buttonHeight, TFT_WHITE, TFT_GREEN, TFT_BLACK, leftButtonText, 1, 1);
	buttonLeft.drawButton();
	leftButtonState = KEY_UNKNOWN;
	
    middleButtonState = -1;

	buttonRight.initButton(&M5.Lcd, buttonWidth * 1 + buttonWidth/2 ,210, buttonWidth, buttonHeight, TFT_WHITE, TFT_RED, TFT_BLACK, rightButtonText, 1, 1);
	buttonRight.drawButton();
	rightButtonState = KEY_UNKNOWN;

	xSemaphoreGive(displayMutex);	

}

#define MAX_KEYS_QUEUED 8


KEY_STATE keyDest;

cppQueue buttonQueue(sizeof(BUTTON_MESSAGE), MAX_KEYS_QUEUED, FIFO);

void setup_button()
{
	keyCountingSemaphore = xSemaphoreCreateCounting(MAX_KEYS_QUEUED,0);

	phyDispWidth = M5.Lcd.width();
	phyDispHeigth = M5.Lcd.height();

	threeButtonMenu("LEFT", "MIDDLE", "RIGHT");
	
	BUTTON_MESSAGE msg;
	
	msg.key = BUTTON_INIT;
	buttonQueue.push(&msg);
	xSemaphoreGive(keyCountingSemaphore);
}

void touchPanel_impl()
{
	BUTTON_MESSAGE msg;
	kickDog();

	if (!bMenuIsActive)
	{
		delay(1);
		return;
	}
	
	xSemaphoreTake(displayMutex, portMAX_DELAY);
	
	// don't update if menu not running.
	M5.update();
	
	touchDetail = M5.Touch.getDetail();

	if (touchDetail.isPressed())
	{
		
		if(buttonLeft.contains(touchDetail.x, touchDetail.y))
		{
			if (leftButtonState != KEY_DOWN)
			{
				Serial.println("Left pressed");
				leftButtonState = KEY_DOWN;
				
				msg.key = LBUTTON_DN;
				buttonQueue.push(&msg);
				xSemaphoreGive(keyCountingSemaphore);
			}
		}
		else if(buttonMiddle.contains(touchDetail.x, touchDetail.y))
		{
			if (middleButtonState != KEY_DOWN)
			{
				Serial.println("Middle pressed");
				middleButtonState = KEY_DOWN;
				msg.key = MBUTTON_DN;
				buttonQueue.push(&msg);
				xSemaphoreGive(keyCountingSemaphore);
			}
		}
		else if(buttonRight.contains(touchDetail.x, touchDetail.y))
		{
			if (rightButtonState != KEY_DOWN)
			{
				Serial.println("Right pressed");
				rightButtonState = KEY_DOWN;

				msg.key = RBUTTON_DN;
				buttonQueue.push(&msg);
				xSemaphoreGive(keyCountingSemaphore);
			}
		}
		
	}

	if (touchDetail.isReleased())
	{
			
		if(buttonLeft.contains(touchDetail.x, touchDetail.y))
		{
			if ( leftButtonState != KEY_UP)
			{
				Serial.println("Left released");
				leftButtonState = KEY_UP;

				msg.key = LBUTTON_UP;
				buttonQueue.push(&msg);
				xSemaphoreGive(keyCountingSemaphore);
			}
		}
		else if(buttonMiddle.contains(touchDetail.x, touchDetail.y))
		{
			if (middleButtonState != KEY_UP)
			{
				Serial.println("Middle released");
				middleButtonState = KEY_UP;

				msg.key = MBUTTON_UP;
				buttonQueue.push(&msg);
				xSemaphoreGive(keyCountingSemaphore);
			}
		}
		else if(buttonRight.contains(touchDetail.x, touchDetail.y))
		{
			if (rightButtonState != KEY_UP)
			{
				Serial.println("Right released");
				rightButtonState = KEY_UP;

				msg.key = RBUTTON_UP;
				buttonQueue.push(&msg);
				xSemaphoreGive(keyCountingSemaphore);
			}
		}
	}

	xSemaphoreGive(displayMutex);	

}

