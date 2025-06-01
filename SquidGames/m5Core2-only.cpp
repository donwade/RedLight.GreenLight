#include <M5Core2.h>
#include "m5Core2-only.h"

void setCursor(uint16_t X, uint16_t BKGND, uint8_t font)
{
	M5.Lcd.setCursor(X, BKGND, font);
}

void print(const __FlashStringHelper *x)
{
	M5.Lcd.print(x);
}

void print(uint8_t x)
{
	M5.Lcd.print(x);
}

void print(double x, int y)
{
	M5.Lcd.print(x,y);
}

void println(void)
{
	M5.Lcd.println();
}

void println(char *x)
{
	M5.Lcd.println(x);
}

void clear(void)
{
	M5.Lcd.clear();
}

void setup_M5(void)
{
	M5.begin(true, true, true, false, kMBusModeInput);
}

void setTextColor(unsigned FGND, unsigned BKGND)
{
	M5.Lcd.setTextColor(FGND,BKGND);
}

