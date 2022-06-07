#include "main.h"
#include "conf.h"
#ifdef USEBLE
#include "sub_ble.h"
#endif
#include "sub_bmp.h"
#include "sub_btn.h"
#include "sub_http.h"
#include "sub_json.h"
#include "sub_led.h"
#include "sub_udp.h"
#include "sub_wifi.h"

int ssid_count = 5;
String ssid[] = {"ledControl", "spiffs", "spiffs", "scp-121",  "salat"};
String pass[] = {"12345678",   "spiffs", "spiffs", "bioshock", "salat191919"};
char exbmp[5] = ".bmp";
char exbma[5] = ".bma";
char exgif[5] = ".gif";
char exjpg[5] = ".jpg";
char extxt[5] = ".txt";

struct config conf = {
	"v0.11b",
	"LedHD",
	0, //wait
	4, //brgn
	10, //mode
	20, //leds
	5.5, //vcc
	0, //cont
	0, //psr
	0, //skwf
	0, //bluetooth
	{22,25,26,27,0,0}, //pins
	BTN_PIN,
	PWR_PIN
};

struct status state = {
	false, //go
	true, //loop
	false, //next
	0, //maxbmp
	0, //currbmp
	0, //setbmp
	3, //whdr
	4000, //bpm
	0, //btt test
	0, //maxprog
	0, //curr prog
	"no_prog", //curr prog name
	0, //proglist
	false //calcmax
};

int bmpw = 0;
int bmph = 0;
char *p;

File root;
File file;

void setup()
{
	pinMode(conf.pinb, INPUT_PULLUP); //init mosfet button
	pinMode(conf.pinp, OUTPUT);
	digitalWrite(conf.pinp, LOW);

	Serial.begin(115200);
	while (!Serial); // wait for serial attach

	#ifdef ARDUINO_ESP32C3_DEV
		conf.wpref = "LedC3";
		conf.leds = 32;
		conf.vcc = 4.1;
	#endif
	adc1_config_width(ADC_WIDTH_BIT_12);
	adc1_config_channel_atten(VCC_CHN, ADC_ATTEN_DB_2_5);

	Serial.println();
	Serial.printf("pinb %d pinp &d\n", conf.pinb, conf.pinp);
	Serial.printf("Initializing... %d\n", CORE_DEBUG_LEVEL);

	conf.psr = ESP.getPsramSize();
	Serial.printf("Total PSRAM: %d", conf.psr);
	if (conf.psr)
	{
		Serial.printf(" allocate ps %d bytes\n", 47 * 128 * 3 * 200);
		p = (char*) ps_realloc(p, 47 * 128 * 3 * 200);
		if(!p) Serial.println("Allocation error psram.");
	}
	else
	{
		Serial.printf(" allocate ram %d bytes\n", 80 * 256 * 3);
		p = (char*) realloc(p, 80 * 256 * 3);
		if(!p) Serial.println("Allocation error ram.");
	}
	Serial.flush();
	// this resets all the neopixels to an off state
	Serial.printf("fs begin %d\n", FILESYSTEM.begin());
	json_load();
	led_init();
	led_calccont(conf.cont);
	led_brgn();
	led_show();
	
	check_up();
	
	bmp_init();
	bmp_max();
	delay(500);
	
	Serial.println();
	Serial.println("Running...");

	if (conf.bt)
	{
		#ifdef USEBLE
		ble_init();
		#else
		state.go = true;
		#endif
	}
	else
	{
		Serial.println("wifi_init");
		wifi_init();
		if (conf.skwf == 1)
		{
			state.go = true;
		}
		else
		{
			Serial.println("wifi_conn");
			wifi_conn();
		}
		Serial.println("http_begin");
		http_begin();
		Serial.println("udp_begin");
		udp_begin();
	}
	
	while (state.go == false)
	{
		if (conf.bt)
		{
			#ifdef USEBLE
			ble_poll();
			#endif
		}
		else
		{
			http_poll();
			udp_poll();
			ftp_poll();
		}
		check_off();
		delay(1);
	}
}


void loop()
{
	if (!file)
	{
		root = FILESYSTEM.open("/");
		file = root.openNextFile();
		state.currbmp = 1;
	}
	if (state.go == true && state.maxbmp > 0)
	{
		if (state.whdr == 3)
		{
			bmp_draw(file.name(), state.bpm);
			if (state.loop)
			{
				file = root.openNextFile();
				state.currbmp++;
			}
			if (state.next)
			{
				file = root.openNextFile();
				state.currbmp++;
				state.next = false;
			}
			if (state.setbmp)
			{
				while (state.currbmp != state.setbmp)
				{
					file = root.openNextFile();
					String fname = file.name();
					if (fname.endsWith(extxt)) continue;
					if (!file)
					{
						root = FILESYSTEM.open("/");
						file = root.openNextFile();
						fname = file.name();
						if (state.maxbmp > 0)
						{
							while(!bmp_check(fname))
							{
								file = root.openNextFile();
								fname = file.name();
							}
						}
						state.currbmp = 1;
					}
					else
					{
						state.currbmp++;
					}
				}
				state.setbmp = 0;
			}
		}
		if (state.whdr == 4)
		{
			state.setbmp = 0;
			if (FILESYSTEM.exists("/" + state.progname))
			{
				String prgopen = state.progname;
				File prgfile = FILESYSTEM.open("/" + prgopen, "r");
				String pic;
				while (prgfile.available())
				{
					if (state.whdr != 4 || state.go == false || prgopen != state.progname) break;
					pic = prgfile.readStringUntil('\n');
					//Serial.println("debug pic [" + pic + "] " + String(pic.length(), DEC) + "] fpos " + String(prgfile.position(), DEC) + " flen " + String(prgfile.size()));
					pic.trim();
					if (pic == "stop") break;
					int spidx = pic.indexOf(" ");
					if (spidx == -1 || spidx == pic.length() - 1)
					{
						Serial.println(F("prog err: no second arg"));
						udp_poll();
						if (prgfile.position() >= prgfile.size()){
							prgfile.close();
							Serial.println(F("file closed"));
						}
					}
					else
					{
						String fpic = pic.substring(0, spidx);
						String dspic = pic.substring(spidx + 1);
						dspic.trim();
						int dpic = dspic.toInt();
						bmp_draw(fpic, dpic);
					}
				}
				prgfile.close();
				udp_poll();
			}
			else
			{
				bmp_rainbow();
				bmp_wait(50);
				Serial.println("prog err: no prog");
			}
		}
	}
	while (state.go == false || state.maxbmp == 0)
	{
		http_poll();
		udp_poll();
		ftp_poll();
		check_off();
		bmp_batlog();
		if (conf.bt)
		{
			#ifdef USEBLE
			ble_poll();
			#endif
		}
		delay(1);
	}
}
