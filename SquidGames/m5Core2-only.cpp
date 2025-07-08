#include <M5Unified.h>
//#include <M5Core2.h>
#include "m5Core2-only.h"

m5::touch_detail_t touchDetail;
LGFX_Button button_pre, button_play, button_next;


static uint8_t lastFont = 4;
static uint8_t vert = 0;

void lsetCursor(uint16_t X, uint16_t Y, uint8_t font)
{
	lastFont = font;

	//int16_t horz =M5.Lcd.textWidth(" ", font);
	//printf("charX=%d\n", horz);

	if (!vert) vert = M5.Lcd.fontHeight() + 4;
	//printf("charY=%d\n", vert);
	
	M5.Lcd.setCursor(X, Y * vert, font);
}


void lsetCursor(uint16_t X, uint16_t Y)
{
	M5.Lcd.setCursor(X, Y * vert , lastFont);
}

void lprint(const __FlashStringHelper *x)
{
	M5.Lcd.print(x);
}

void lprint(char *x)
{
	M5.Lcd.print(x);
}

void lprint(uint8_t x)
{
	M5.Lcd.print(x);
}


void lprint(double x, int y)
{
	M5.Lcd.print(x,y);
}

void lprintln(void)
{
	M5.Lcd.println();
}

void lprintln(char *x)
{
	M5.Lcd.println(x);
}

void lclear(void)
{
	M5.Lcd.clear();
}

unsigned char BUTTON_WIDE = 60;
unsigned char BUTTON_HEIGHT = 60;

void setup_M5(void)
{
	M5.begin();
	M5.Power.setExtOutput(true);  // enable external bus

    M5.Lcd.setTextFont(&fonts::DejaVu18);
	
	unsigned int w = M5.Lcd.width();
	BUTTON_WIDE = w /3;
	
    unsigned int h = M5.Lcd.height();

	
	// coordinates specify center of button hence odd math
	button_pre.initButton(&M5.Lcd,  BUTTON_WIDE * 0 + BUTTON_WIDE/2, 215, BUTTON_WIDE, BUTTON_HEIGHT, TFT_WHITE, TFT_GREEN, TFT_BLACK, "CAMERA", 1, 1);
	button_pre.drawButton();
	button_play.initButton(&M5.Lcd, BUTTON_WIDE * 1 + BUTTON_WIDE/2, 215, BUTTON_WIDE, BUTTON_HEIGHT, TFT_WHITE, TFT_YELLOW, TFT_BLACK, "AWAY", 1, 1);
	button_play.drawButton();
	button_next.initButton(&M5.Lcd, BUTTON_WIDE * 2 + BUTTON_WIDE/2 ,215, BUTTON_WIDE, BUTTON_HEIGHT, TFT_WHITE, TFT_RED, TFT_BLACK, "SAVE", 1, 1);
	button_next.drawButton();


	//M5.begin(true, true, true, false, kMBusModeInput, true);
}

void lsetTextColor(unsigned FGND, unsigned BKGND)
{
	M5.Lcd.setTextColor(FGND,BKGND);
    M5.Lcd.setTextFont(&fonts::DejaVu18);

	
	//w = M5.Lcd.width();
    //h = M5.Lcd.height();
    //M5.Lcd.setTextColor(TFT_BLACK);
    //M5.Lcd.fillScreen(WHITE);
    //M5.Display.setRotation(1);
    //M5.Display.setTextColor(TFT_BLACK);
    //M5.Display.setTextDatum(top_center);
    //M5.Display.drawString("HI MOM", w / 2, 0, &fonts::FreeMonoBold12pt7b);
    
}

