#include <M5Unified.h>
#include "m5Core2-only.h"
#include "viewController.h"

static uint8_t vert = 0;

void lsetCursor(uint16_t X, uint16_t Y)
{
	//int16_t horz =M5.Lcd.textWidth(" ", font);
	//printf("charX=%d\n", horz);

	if (!vert) vert = M5.Lcd.fontHeight() + 4;
	//printf("charY=%d\n", vert);
	
	M5.Lcd.setCursor(X, Y * vert);
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
//--------------------------------------------------
#include <Adafruit_GFX.h>    // Core graphics library
// below must follow above
#include <Fonts/FreeMonoBoldOblique12pt7b.h>
#include <Fonts/FreeMono12pt7b.h>

//--------------------------------------------------


void setup_M5(void)
{
	M5.begin();
	M5.Power.setExtOutput(true);  // enable external bus

	// confusing. this sets font for background display
    M5.Lcd.setTextFont(DEFAULT_FONT);

	// confusing. this sets font for buttons
	M5.Lcd.setFont(WIDGET_FONT);
	
	setup_button();
}

void lsetTextColor(unsigned FGND, unsigned BKGND)
{
    M5.Lcd.setTextFont(DEFAULT_FONT);
	M5.Lcd.setTextColor(FGND,BKGND);
	
	//w = M5.Lcd.width();
    //h = M5.Lcd.height();
    //M5.Lcd.setTextColor(TFT_BLACK);
    //M5.Lcd.fillScreen(WHITE);
    //M5.Display.setRotation(1);
    //M5.Display.setTextColor(TFT_BLACK);
    //M5.Display.setTextDatum(top_center);
    //M5.Display.drawString("HI MOM", w / 2, 0, &fonts::FreeMonoBold12pt7b);
    
}

