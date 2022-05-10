#ifndef CONF_H
#define CONF_H

#define FILESYSTEM FFat
//#define FILESYSTEM SPIFFS
//#define USEBLE

#ifndef ARDUINO_ESP32C3_DEV
#define VCC_PIN 36
#define VCC_CHN ADC1_CHANNEL_0
#define BTN_PIN 12
#define PWR_PIN 14
#else
#define VCC_PIN 1
#define VCC_CHN ADC1_CHANNEL_1
#define BTN_PIN 18
#define PWR_PIN 19
#endif

struct config
{
	String ver;
	String wpref;
	int wait;
	char brgn;
	char mode;
	int leds;
	float vcc;
	int cont;
	int psr; //psram size
	char skwf;
	char bt;
	uint8_t pins[6];
};

struct status
{
	bool go;
	bool loop;
	bool next;
	int maxbmp;
	int currbmp;
	int setbmp;
	int whdr;
	uint16_t bpm;
	unsigned long uptime;
	uint8_t maxprog;
	uint8_t currprog;
	String progname;
	char proglist[15][32];
	bool calcmax;
};

extern struct config conf;
extern struct status state;

extern int bmpw;
extern int bmph;
extern char *p;

extern char exbmp[5];
extern char exbma[5];
extern char exgif[5];
extern char exjpg[5];
extern char extxt[5];

#endif
