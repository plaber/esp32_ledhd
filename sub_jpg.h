#ifndef SUB_JPG_H
#define SUB_JPG_H

#include <Arduino.h>
#if FILESYSTEM == FFat
#include <FFat.h>
#endif
#if FILESYSTEM == SPIFFS
#include <SPIFFS.h>
#endif
#include <malloc.h>

#include <TJpg_Decoder.h>

struct jpgheader
{
	uint16_t w;
	uint16_t h;
	uint32_t size;
};

struct jpgheader jpg_header(String path, bool rundec);
void jpg_load(String path);

#endif
