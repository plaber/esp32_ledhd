#ifndef SUB_BMP_H
#define SUB_BMP_H

#include <Arduino.h>
#if FILESYSTEM == FFat
#include <FFat.h>
#endif
#if FILESYSTEM == SPIFFS
#include <SPIFFS.h>
#endif
#include <malloc.h>

#pragma pack(push, 2)
struct bmpheader
{
	uint16_t mark;
	uint32_t size;
	uint16_t Reserved0;
	uint16_t Reserved1;
	uint32_t offset;
	uint32_t tp;
	uint32_t w;
	uint32_t h;
	uint16_t planes;
	uint16_t bits;
	uint32_t compres;
	char bminfo[5];
};
#pragma pack(pop)

bmpheader bmp_load(String path, bool ld);
void bmp_load(String path);
bool bmp_check(String path);
void bmp_draw(String path, unsigned long tm);
void bmp_draw_poi(String path, unsigned long tm);
void bmp_draw_mask(String path, unsigned long tm);
void bmp_draw_last(int fps);
void bmp_init();
void bmp_max();
String bmp_conf();
void bmp_savecost();
void bmp_loadcost();
void bmp_wait(int ms);
void bmp_rainbow();
void bmp_batlog();
void bmp_next();

#define CLEN1 17 + 11 + 12 + 10 + 12 + 10 + 8 + 11 + 10 + 12
#define SWIDX1 17
#define SWIDX12 80
#define FRMH1 202
#define M1 13
#define K1 45

const char cords1[] PROGMEM = {
	 9+M1,  6, 9+M1, 44,//mask
	 8+M1, 43, 8+M1, 28,
	 7+M1, 29, 7+M1, 42,
	 6+M1, 42, 6+M1, 29,
	 5+M1, 29, 5+M1, 41,
	 4+M1, 40, 4+M1, 29,
	 3+M1, 30, 3+M1, 39,
	 2+M1, 38, 2+M1, 30,
	 8+M1,  4, 8+M1, 23,
	 7+M1, 22, 7+M1,  3,
	 6+M1,  2, 6+M1, 21,
	 5+M1, 20, 5+M1,  1,
	 4+M1,  1, 4+M1, 20,
	 3+M1, 19, 3+M1,  0,
	 2+M1,  0, 2+M1, 19,
	 1+M1, 23, 1+M1,  0,
	 0+M1,  1, 0+M1, 22,
	22,  6+K1, 22, 38+K1,//chest
	21, 38+K1, 21,  5+K1,
	20,  5+K1, 20, 38+K1,
	19, 38+K1, 19,  4+K1,
	18,  3+K1, 18, 38+K1,
	17, 37+K1, 17,  2+K1,
	16,  3+K1, 16, 37+K1,
	15, 36+K1, 15,  4+K1,
	14,  5+K1, 14, 13+K1,
	14, 25+K1, 14, 36+K1,
	13, 35+K1, 13, 27+K1,
	11,  2+K1, 11,  7+K1,//arm 1
	10,  9+K1, 10,  1+K1,
	 9,  0+K1,  9, 10+K1,
	 8, 12+K1,  8,  0+K1,
	 7,  0+K1,  7, 12+K1,
	 6, 12+K1,  6,  0+K1,
	 5,  0+K1,  5, 12+K1,
	 4, 12+K1,  4,  0+K1,
	 3,  0+K1,  3, 12+K1,
	 2, 10+K1,  2,  0+K1,
	 1,  1+K1,  1,  9+K1,
	 0,  7+K1,  0,  2+K1,
	 1, 22+K1,  1, 15+K1,//arm 2
	 2, 14+K1,  2, 26+K1,
	 3, 27+K1,  3, 14+K1,
	 4, 14+K1,  4, 28+K1,
	 5, 28+K1,  5, 14+K1,
	 6, 14+K1,  6, 28+K1,
	 7, 28+K1,  7, 14+K1,
	 8, 14+K1,  8, 27+K1,
	 9, 26+K1,  9, 14+K1,
	10, 15+K1, 10, 22+K1,
	11, 32+K1, 11, 35+K1,//arm 3
	10, 41+K1, 10, 30+K1,
	 9, 30+K1,  9, 42+K1,
	 8, 43+K1,  8, 30+K1,
	 7, 30+K1,  7, 43+K1,
	 6, 43+K1,  6, 30+K1,
	 5, 30+K1,  5, 43+K1,
	 4, 43+K1,  4, 30+K1,
	 3, 30+K1,  3, 43+K1,
	 2, 42+K1,  2, 30+K1,
	 1, 30+K1,  1, 41+K1,
	 0, 35+K1,  0, 32+K1,
	 1, 45+K1,  1, 51+K1,//arm 4
	 2, 59+K1,  2, 45+K1,
	 3, 45+K1,  3, 60+K1,
	 4, 60+K1,  4, 45+K1,
	 5, 45+K1,  5, 61+K1,
	 6, 61+K1,  6, 45+K1,
	 7, 45+K1,  7, 60+K1,
	 8, 60+K1,  8, 45+K1,
	 9, 45+K1,  9, 59+K1,
	10, 51+K1, 10, 45+K1,
	 1, 63+K1, 12, 63+K1,//hand
	13, 64+K1,  0, 64+K1,
	 0, 65+K1, 13, 65+K1,
	12, 66+K1,  1, 66+K1,
	 1, 68+K1,  1, 71+K1,
	 5, 74+K1,  5, 68+K1,
	 8, 68+K1,  8, 74+K1,
	12, 74+K1, 12, 68+K1,
	 1, 132,  1, 124,//leg 1
	 2, 123,  2, 134,
	 2, 146,  2, 154,
	 3, 155,  3, 123,
	 4, 122,  4, 156,
	 5, 157,  5, 122,
	 6, 121,  6, 156,
	 7, 155,  7, 121,
	 8, 121,  8, 154,
	 9, 154,  9, 121,
	10, 121, 10, 153,
	12, 136, 12, 168,//leg 2
	13, 168, 13, 135,
	14, 135, 14, 168,
	15, 168, 15, 134,
	16, 133, 16, 168,
	17, 167, 17, 132,
	18, 133, 18, 167,
	19, 166, 19, 134,
	20, 135, 20, 143,
	20, 155, 20, 166,
	22, 171, 22, 188,//leg 3
	21, 195, 21, 170,
	20, 170, 20, 200,
	19, 201, 19, 171,
	18, 171, 18, 201,
	17, 201, 17, 171,
	16, 172, 16, 201,
	15, 201, 15, 172,
	14, 173, 14, 201,
	13, 199, 13, 173,
	12, 174, 12, 195,
	11, 188, 11, 175
};

#define CLEN2 26 + 11 + 12 + 10 + 12 + 10 + 8 + 11 + 11 + 13
#define SWIDX2 26
#define SWIDX22 89
#define FRMH2 226
#define M2 2
#define K2 66

const char cords2[] PROGMEM = {
	22+M2, 65, 22+M2, 50,//mask
	24+M2, 50, 24+M2, 65,
	25+M2, 64, 40+M2, 61,
	40+M2, 60, 25+M2, 63,
	25+M2, 61, 41+M2, 57,
	42+M2, 55, 25+M2, 59,
	25+M2, 57, 42+M2, 53,
	42+M2, 51, 25+M2, 55,
	25+M2, 53, 42+M2, 49,
	42+M2, 44, 34+M2, 48,
	33+M2, 49, 25+M2, 51,
	22+M2,  6, 22+M2, 42,
	24+M2, 40, 24+M2,  5,
	26+M2,  5, 26+M2, 38,
	28+M2, 36, 28+M2,  6,
	30+M2,  8, 30+M2, 34,
	32+M2, 31, 32+M2, 11,
	31+M2,  0, 33+M2,  5,
	34+M2,  6, 35+M2, 15,
	36+M2, 16, 36+M2, 30,
	36+M2, 31, 35+M2, 38,
	34+M2, 39, 24+M2, 44,
	24+M2, 46, 39+M2, 39,
	40+M2, 30, 40+M2, 15,
	39+M2, 14, 37+M2,  4,
	36+M2,  3, 35+M2,  0,
	22,  6+K2, 22, 38+K2,//chest
	21, 38+K2, 21,  5+K2,
	20,  5+K2, 20, 38+K2,
	19, 38+K2, 19,  4+K2,
	18,  3+K2, 18, 38+K2,
	17, 37+K2, 17,  2+K2,
	16,  3+K2, 16, 37+K2,
	15, 36+K2, 15,  4+K2,
	14,  5+K2, 14, 13+K2,
	14, 25+K2, 14, 36+K2,
	13, 35+K2, 13, 27+K2,
	11,  2+K2, 11,  7+K2,//arm 1
	10,  9+K2, 10,  1+K2,
	 9,  0+K2,  9, 10+K2,
	 8, 12+K2,  8,  0+K2,
	 7,  0+K2,  7, 12+K2,
	 6, 12+K2,  6,  0+K2,
	 5,  0+K2,  5, 12+K2,
	 4, 12+K2,  4,  0+K2,
	 3,  0+K2,  3, 12+K2,
	 2, 10+K2,  2,  0+K2,
	 1,  1+K2,  1,  9+K2,
	 0,  7+K2,  0,  2+K2,
	 1, 22+K2,  1, 15+K2,//arm 2
	 2, 14+K2,  2, 26+K2,
	 3, 27+K2,  3, 14+K2,
	 4, 14+K2,  4, 28+K2,
	 5, 28+K2,  5, 14+K2,
	 6, 14+K2,  6, 28+K2,
	 7, 28+K2,  7, 14+K2,
	 8, 14+K2,  8, 27+K2,
	 9, 26+K2,  9, 14+K2,
	10, 15+K2, 10, 22+K2,
	11, 32+K2, 11, 35+K2,//arm 3
	10, 41+K2, 10, 30+K2,
	 9, 30+K2,  9, 42+K2,
	 8, 43+K2,  8, 30+K2,
	 7, 30+K2,  7, 43+K2,
	 6, 43+K2,  6, 30+K2,
	 5, 30+K2,  5, 43+K2,
	 4, 43+K2,  4, 30+K2,
	 3, 30+K2,  3, 43+K2,
	 2, 42+K2,  2, 30+K2,
	 1, 30+K2,  1, 41+K2,
	 0, 35+K2,  0, 32+K2,
	 1, 45+K2,  1, 51+K2,//arm 4
	 2, 59+K2,  2, 45+K2,
	 3, 45+K2,  3, 60+K2,
	 4, 60+K2,  4, 45+K2,
	 5, 45+K2,  5, 61+K2,
	 6, 61+K2,  6, 45+K2,
	 7, 45+K2,  7, 60+K2,
	 8, 60+K2,  8, 45+K2,
	 9, 45+K2,  9, 59+K2,
	10, 51+K2, 10, 45+K2,
	 1, 63+K2, 12, 63+K2,//hand
	13, 64+K2,  0, 64+K2,
	 0, 65+K2, 13, 65+K2,
	12, 66+K2,  1, 66+K2,
	 1, 68+K2,  1, 71+K2,
	 5, 74+K2,  5, 68+K2,
	 8, 68+K2,  8, 74+K2,
	12, 74+K2, 12, 68+K2,
	 1, 145,  1, 153,//leg 1
	 2, 175,  2, 167,
	 2, 155,  2, 144,
	 3, 144,  3, 176,
	 4, 177,  4, 143,
	 5, 143,  5, 178,
	 6, 177,  6, 142,
	 7, 142,  7, 176,
	 8, 175,  8, 142,
	 9, 142,  9, 175,
	10, 174, 10, 142,
	12, 193, 12, 161,//leg 2
	13, 160, 13, 193,
	14, 193, 14, 160,
	15, 159, 15, 193,
	16, 193, 16, 158,
	17, 157, 17, 192,
	18, 192, 18, 158,
	19, 159, 19, 191,
	20, 191, 20, 180,
	20, 168, 20, 160,
	21, 182, 21, 190,
	22, 197, 22, 210,//leg 3
	21, 224, 21, 196,
	20, 196, 20, 225,
	19, 225, 19, 195,
	18, 195, 18, 225,
	17, 225, 17, 195,
	16, 195, 16, 225,
	15, 225, 15, 195,
	14, 195, 14, 225,
	13, 225, 13, 195,
	12, 196, 12, 225,
	11, 224, 11, 196,
	10, 197, 10, 210
};

#endif
