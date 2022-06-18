#include "sub_led.h"
#include "conf.h"

RgbColor black(0, 0, 0);
RgbColor white(255, 255, 255);
RgbColor dwhite(128, 128, 128);
RgbColor red(255, 0, 0);
RgbColor yellow(255, 255, 0);
RgbColor orange(255, 127, 0);
RgbColor green(0, 255, 0);
RgbColor blue(0, 0, 255);
RgbColor wblue(0, 127, 255);
RgbColor cyan(0, 255, 255);

char cont[256] = {   
    0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,                              
    0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 1 , 1 , 1 , 1 ,                              
    1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 2 , 2 , 2 , 2 , 2 , 2 , 2 ,                              
    2 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 4 , 4 , 4 , 4 , 4 , 5 , 5 , 5 ,                              
    5 , 6 , 6 , 6 , 6 , 7 , 7 , 7 , 7 , 8 , 8 , 8 , 9 , 9 , 9 , 10 ,                             
   10 , 10 , 11 , 11 , 11 , 12 , 12 , 13 , 13 , 13 , 14 , 14 , 15 , 15 , 16 , 16 ,               
   17 , 17 , 18 , 18 , 19 , 19 , 20 , 20 , 21 , 21 , 22 , 22 , 23 , 24 , 24 , 25 ,               
   25 , 26 , 27 , 27 , 28 , 29 , 29 , 30 , 31 , 32 , 32 , 33 , 34 , 35 , 35 , 36 ,               
   37 , 38 , 39 , 39 , 40 , 41 , 42 , 43 , 44 , 45 , 46 , 47 , 48 , 49 , 50 , 50 ,               
   51 , 52 , 54 , 55 , 56 , 57 , 58 , 59 , 60 , 61 , 62 , 63 , 64 , 66 , 67 , 68 ,               
   69 , 70 , 72 , 73 , 74 , 75 , 77 , 78 , 79 , 81 , 82 , 83 , 85 , 86 , 87 , 89 ,               
   90 , 92 , 93 , 95 , 96 , 98 , 99 , 101 , 102 , 104 , 105 , 107 , 109 , 110 , 112 , 114 ,      
  115 , 117 , 119 , 120 , 122 , 124 , 126 , 127 , 129 , 131 , 133 , 135 , 137 , 138 , 140 , 142 ,
  144 , 146 , 148 , 150 , 152 , 154 , 156 , 158 , 160 , 162 , 164 , 167 , 169 , 171 , 173 , 175 ,
  177 , 180 , 182 , 184 , 186 , 189 , 191 , 193 , 196 , 198 , 200 , 203 , 205 , 208 , 210 , 213 ,
  215 , 218 , 220 , 223 , 225 , 228 , 231 , 233 , 236 , 239 , 241 , 244 , 247 , 249 , 252 , 255 };

int prop(int x, int x1, int x2, int y1, int y2)
{
	return y1 + (y2 - y1) * (x - x1) / (x2 - x1);
}

void led_calccont(int confcont)
{
	for (int i = 0; i < 256; i++)
	{
		if (i < confcont)
			cont[i] = 0;
		else if (i > 255 - confcont)
			cont[i] = 255;
		else
			cont[i] = prop(i, confcont, 255 - confcont, 0, 255);
	}
}

void led_blink(int r, int g, int b)
{
	RgbColor color(r, g ,b);
	led_setpxall(1, color);
	led_show();
	delay(300);
	led_setpxall(1, black);
	led_show();
	delay(300);
}

void led_drawip(int d)
{
	int n1 = d % 10; //1
	int n2 = ((d - n1) / 10) % 10; //10
	int n3 = (d - n1 - n2 * 10) / 100; //100
	int i = 0, s = 0;
	if (n3 != 0)
	{
		for (i = 0; i < n3; i++)
		{
			led_setpxall(i, red);
		}
		s += n3;
	}
	if (n2 != 0)
	{
		for (i = s; i < s + n2; i++)
		{
			led_setpxall(i, green);
		}
		s += n2;
	}
	else
	{
		if (n3 != 0)
		{
			led_setpxall(s, white);
			s++;
		}
	}
	if (n1 != 0)
	{
		for (i = s; i < s + n1; i++)
		{
			led_setpxall(i, blue);
		}
		s += n1;
	}
	else
	{
		led_setpxall(s, white);
		s++;
	}
	led_show();
}

void led_brgn()
{
	led_brgn(conf.brgn);
}

#ifndef ARDUINO_ESP32C3_DEV

NeoPixelBrightnessBus<NeoGrbFeature, NeoEsp32Rmt0Ws2812xMethod>* strip0 = NULL;
NeoPixelBrightnessBus<NeoGrbFeature, NeoEsp32Rmt1Ws2812xMethod>* strip1 = NULL;
NeoPixelBrightnessBus<NeoGrbFeature, NeoEsp32Rmt2Ws2812xMethod>* strip2 = NULL;
NeoPixelBrightnessBus<NeoGrbFeature, NeoEsp32Rmt3Ws2812xMethod>* strip3 = NULL;
NeoPixelBrightnessBus<NeoGrbFeature, NeoEsp32I2s0Ws2812xMethod>* strip4 = NULL;
NeoPixelBrightnessBus<NeoGrbFeature, NeoEsp32I2s1Ws2812xMethod>* strip5 = NULL;

void led_init()
{
	if (conf.mode != 10)
	{
		Preferences preferences;
		preferences.begin("pins", true);
		conf.pins[0] = preferences.getUInt("p0", 22);
		conf.pins[1] = preferences.getUInt("p1", 23);
		conf.pins[2] = preferences.getUInt("p2", 18);
		conf.pins[3] = preferences.getUInt("p3", 19);
		conf.pins[4] = preferences.getUInt("p4",  4);
		conf.pins[5] = preferences.getUInt("p5",  5);
		preferences.end();
		Serial.printf("pins %d %d %d %d %d %d\n", conf.pins[0], conf.pins[1], conf.pins[2], conf.pins[3], conf.pins[4], conf.pins[5]);
	}

	if (strip0 != NULL) delete strip0;
	if (strip1 != NULL) delete strip1;
	if (strip2 != NULL) delete strip2;
	if (strip3 != NULL) delete strip3;
	if (strip4 != NULL) delete strip4;
	if (strip5 != NULL) delete strip5;

	strip0 = new NeoPixelBrightnessBus<NeoGrbFeature, NeoEsp32Rmt0Ws2812xMethod> (conf.leds, conf.pins[0]);
	strip1 = new NeoPixelBrightnessBus<NeoGrbFeature, NeoEsp32Rmt1Ws2812xMethod> (conf.leds, conf.pins[1]);
	strip2 = new NeoPixelBrightnessBus<NeoGrbFeature, NeoEsp32Rmt2Ws2812xMethod> (conf.leds, conf.pins[2]);
	strip3 = new NeoPixelBrightnessBus<NeoGrbFeature, NeoEsp32Rmt3Ws2812xMethod> (conf.leds, conf.pins[3]);
	if (conf.pins[4]) strip4 = new NeoPixelBrightnessBus<NeoGrbFeature, NeoEsp32I2s0Ws2812xMethod> (conf.leds, conf.pins[4]);
	if (conf.pins[5]) strip5 = new NeoPixelBrightnessBus<NeoGrbFeature, NeoEsp32I2s1Ws2812xMethod> (conf.leds, conf.pins[5]);

	strip0->Begin();
	strip1->Begin();
	strip2->Begin();
	strip3->Begin();
	if (strip4 != NULL) strip4->Begin();
	if (strip5 != NULL) strip5->Begin();

	led_setpxall(0, green);
	if (conf.skwf) led_setpxall(0, red);
	if (conf.bt) led_setpxall(0, blue);
}

void led_blinkudp()
{
	strip0->ClearTo(red);
	strip2->ClearTo(orange);
	if (strip4 != NULL) strip4->ClearTo(yellow);
	
	strip1->ClearTo(blue);
	strip3->ClearTo(wblue);
	if (strip5 != NULL) strip5->ClearTo(cyan);

	led_show();
}

void led_setpx(int i, RgbColor &color)
{
	RgbColor colorm(cont[color.R], cont[color.G], cont[color.B]);
	int j = 80 - i - 1;
	if (j < 20) {strip0->SetPixelColor(j -  0, colorm); return;}
	if (j < 40) {strip1->SetPixelColor(j - 20, colorm); return;}
	if (j < 60) {strip2->SetPixelColor(j - 40, colorm); return;}
	if (j < 80) {strip3->SetPixelColor(j - 60, colorm); return;}
}

void led_setpx(int i, RgbColor &color, int str)
{
	RgbColor colorm(cont[color.R], cont[color.G], cont[color.B]);
	switch(str)
	{
		case 0: strip0->SetPixelColor(i, colorm); break;
		case 1: strip1->SetPixelColor(i, colorm); break;
		case 2: strip2->SetPixelColor(i, colorm); break;
		case 3: strip3->SetPixelColor(i, colorm); break;
		case 4: if (strip4 != NULL) strip4->SetPixelColor(i, colorm); break;
		case 5: if (strip5 != NULL) strip5->SetPixelColor(i, colorm); break;
	}
}

void led_setpxall(int i, RgbColor &color)
{
	if (conf.mode != 10)
	{
		RgbColor colorm(cont[color.R], cont[color.G], cont[color.B]);
		strip0->SetPixelColor(i, colorm);
		strip1->SetPixelColor(i, colorm);
		strip2->SetPixelColor(i, colorm);
		strip3->SetPixelColor(i, colorm);
		if (strip4 != NULL) strip4->SetPixelColor(i, colorm);
		if (strip5 != NULL) strip5->SetPixelColor(i, colorm);
	}
	else
	{
		led_setpx(i, color);
	}
}

void led_brgn(int brgn)
{
	brgn = brgn > 128 ? 128 : brgn;
	strip0->SetBrightness(brgn);
	strip1->SetBrightness(brgn);
	strip2->SetBrightness(brgn);
	strip3->SetBrightness(brgn);
	if (strip4 != NULL) strip4->SetBrightness(brgn);
	if (strip5 != NULL) strip5->SetBrightness(brgn);
}

void led_show()
{
	strip0->Show();
	strip1->Show();
	strip2->Show();
	strip3->Show();
	if (strip4 != NULL) strip4->Show();
	if (strip5 != NULL) strip5->Show();
}

void led_clear()
{
	strip0->ClearTo(black);
	strip1->ClearTo(black);
	strip2->ClearTo(black);
	strip3->ClearTo(black);
	if (strip4 != NULL) strip4->ClearTo(black);
	if (strip5 != NULL) strip5->ClearTo(black);
}

void led_clearto(char r, char g, char b)
{
	RgbColor color(r, g, b);
	strip0->ClearTo(color);
	strip1->ClearTo(color);
	strip2->ClearTo(color);
	strip3->ClearTo(color);
	if (strip4 != NULL) strip4->ClearTo(color);
	if (strip5 != NULL) strip5->ClearTo(color);
}

#else

NeoPixelBrightnessBus<NeoGrbFeature, Neo800KbpsMethod>* strip0 = NULL;

void led_init()
{
	uint8_t PixelPin0 = 20;

	if (strip0 != NULL) delete strip0;

	strip0 = new NeoPixelBrightnessBus<NeoGrbFeature, Neo800KbpsMethod> (conf.leds, PixelPin0);//Neo800KbpsMethod

	strip0->Begin();
	strip0->SetPixelColor(0, green);
	if (conf.skwf) strip0->SetPixelColor(0, red);
	if (conf.bt) strip0->SetPixelColor(0, blue);
}

void led_blinkudp()
{
	strip0->ClearTo(red);  led_show();delay(500);
	strip0->ClearTo(green);led_show();delay(500);
	strip0->ClearTo(blue); led_show();delay(500);
}

void led_setpx(int i, RgbColor &color)
{
	RgbColor colorm(cont[color.R], cont[color.G], cont[color.B]);
	strip0->SetPixelColor(conf.leds - i - 1, colorm);
}

void led_setpx(int i, RgbColor &color, int str)
{
	RgbColor colorm(cont[color.R], cont[color.G], cont[color.B]);
	strip0->SetPixelColor(i, colorm);
}

void led_setpxall(int i, RgbColor &color)
{
	led_setpx(i, color);
}

void led_brgn(int brgn)
{
	brgn = brgn > 128 ? 128 : brgn;
	strip0->SetBrightness(brgn);
}

void led_show()
{
	strip0->Show();
}

void led_clear()
{
	strip0->ClearTo(black);
}

void led_clearto(char r, char g, char b)
{
	RgbColor color(r, g, b);
	strip0->ClearTo(color);
}

#endif
