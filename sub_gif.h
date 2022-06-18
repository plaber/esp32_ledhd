#ifndef SUB_GIF_H
#define SUB_GIF_H

#include <Arduino.h>
#if FILESYSTEM == FFat
#include <FFat.h>
#endif
#if FILESYSTEM == SPIFFS
#include <SPIFFS.h>
#endif
#include <malloc.h>

struct gifheader
{
	char gif[3];
	char mod[3];
	uint16_t w;
	uint16_t h;
	uint8_t flag;
	uint8_t bkgr;
	uint8_t ratio;
	int cdp;
	uint8_t fb;
	uint16_t row;
	uint16_t col;
	uint16_t wb;
	uint16_t hb;
	uint8_t flagb;
	uint8_t lzw;
	uint8_t sz;
	int fsz;
	int fps;
};

struct gifheader gif_load(String path, bool ld);
void gif_load(String path);

#endif
