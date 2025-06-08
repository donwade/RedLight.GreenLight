#include <M5Core2.h>
#include "m5Core2-only.h"

static uint8_t lastFont = 4;
static uint8_t vert;

void lsetCursor(uint16_t X, uint16_t Y, uint8_t font)
{
	lastFont = font;

	//int16_t horz =M5.Lcd.textWidth(" ", font);
	//printf("charX=%d\n", horz);

	vert = M5.Lcd.fontHeight() * 4;
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

void setup_M5(void)
{
	//M5.begin(true, true, true, false, kMBusModeInput);
	M5.begin(true, true, true, true, kMBusModeOutput, true);
}

void lsetTextColor(unsigned FGND, unsigned BKGND)
{
	M5.Lcd.setTextColor(FGND,BKGND);
}

