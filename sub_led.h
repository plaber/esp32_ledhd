#ifndef SUB_LED_H
#define SUB_LED_H

#include <SPI.h>
#include <NeoPixelBrightnessBus.h>
#include <Preferences.h>

extern struct RgbColor black;
extern struct RgbColor white;
extern struct RgbColor dwhite;
extern struct RgbColor red;
extern struct RgbColor yellow;
extern struct RgbColor orange;
extern struct RgbColor green;
extern struct RgbColor wgreen;
extern struct RgbColor blue;

void led_calccont(int confcont);
void led_blink(int r, int g, int b);
void led_blinkudp();
void led_drawip(int d);
void led_brgn();
void led_brgn(int brgn);
void led_init();
void led_setpx(int i, RgbColor &color);
void led_setpx(int i, RgbColor &color, int str);
void led_setpxall(int i, RgbColor &color);
void led_show();
void led_clear();
void led_clearto(char r, char g, char b);

#endif
