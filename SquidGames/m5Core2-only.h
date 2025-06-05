//#define setCursor(...) M5.Lcd.setCursor(__VA_ARGS__)
//#define print(...) M5.Lcd.print(__VA_ARGS__)
extern void setCursor(uint16_t X, uint16_t Y, uint8_t font);
extern void print(const __FlashStringHelper *x);
extern void print(uint8_t x);
extern void print(char *x);

extern void print(double x, int y);

extern void println(void);
extern void println(char *x);
extern void clear(void);
extern void setup_M5(void);
extern void setTextColor(unsigned FGND, unsigned BKGND);



