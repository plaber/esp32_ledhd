#include "main.h"
#include "conf.h"
#ifdef USEBLE
#include "sub_ble.h"
#endif
#include "sub_bmp.h"
#include "sub_btn.h"
#include "sub_enow.h"
#include "sub_http.h"
#include "sub_json.h"
#include "sub_led.h"
#include "sub_udp.h"
#include "sub_wifi.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

int ssid_count = 5;
String ssid[] = {"ledControl", "spiffs", "spiffs", "scp-121",  "salat"};
String pass[] = {"12345678",   "spiffs", "spiffs", "bioshock", "salat191919"};
char exbmp[5] = ".bmp";
char exbma[5] = ".bma";
char exgif[5] = ".gif";
char exjpg[5] = ".jpg";
char extxt[5] = ".txt";

struct config conf = {
	.ver = "v0.19",
	.wpref = "LedHDxx",
	.macs = {0},
	.macson = 0,
	.macslen = 0,
	.wait = 0,
	.brgn = 4,
	.mode = 10,
	.leds = 20,
	.vcc = 5.5,
	.cont = 0,
	.psr = 0, //psram size
	.skwf = false,
	.bt = false, //bluetooth
	.enow = false,
	.enowone = false,
	.pins = {22,25,26,27,0,0},
	.pinb = BTN_PIN,
	.pinp = PWR_PIN,
	.vccch = ADC1_CHANNEL_0 //36
};

struct status state = {
	.go = false,
	.loop = true,
	.next = false,
	.maxbmp = 0,
	.currbmp = 0,
	.currname = "no_bmp",
	.setbmp = 0,
	.whdr = 3,
	.bpm = 4000,
	.uptime = 0, //btt test
	.maxprog = 0,
	.currprog = 0,
	.progname = "no_prog", //curr prog name
	.proglist = {0},
	.maxfold = 0,
	.currfold = 0,
	.foldname = "no_fold", //curr fold name
	.foldlist= {0},
	.calcmax = false
};

int bmpw = 0;
int bmph = 0;
char *p;

File root;
File file;

void setup()
{
	WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
	json_load();
	#ifdef ARDUINO_ESP32_DEV
		if (conf.mode == 10 || conf.mode == 14)
		{
			conf.pinb = 4;
			conf.pinp = 5;
		}
		if (conf.mode == 10) {conf.wpref = "LedHD72"; conf.vccch = ADC1_CHANNEL_5;} //33
		if (conf.mode == 14) {conf.wpref = "LedHD80"; conf.vccch = ADC1_CHANNEL_0;} //36
	#endif
	pinMode(conf.pinb, INPUT_PULLUP); //init mosfet button
	pinMode(conf.pinp, OUTPUT);
	digitalWrite(conf.pinp, LOW);
	
	Serial.begin(115200);
	while (!Serial); // wait for serial attach
	
	#ifdef ARDUINO_ESP32C3_DEV
		conf.wpref = "LedC332";
		conf.leds = 32;
		conf.vcc = 4.1;
		conf.vccch = ADC1_CHANNEL_1; //1
	#endif
	adc1_config_width(ADC_WIDTH_BIT_12);
	adc1_config_channel_atten(conf.vccch, ADC_ATTEN_DB_2_5);
	
	Serial.println();
	Serial.printf("Initializing... %d\n", CORE_DEBUG_LEVEL);
	
	led_init();
	led_calccont(conf.cont);
	led_brgn();
	led_show();
	
	check_up();

	conf.psr = ESP.getPsramSize();
	Serial.printf("Total PSRAM: %d\n", conf.psr);
	if (conf.psr)
	{
		Serial.printf("Allocate ps %d\n", 3800000);
		p = (char*) ps_realloc(p, 3800000);
		if(!p) Serial.println("Allocation error psram");
	}
	else
	{
		Serial.printf("Allocate ram %d\n", 64000);
		p = (char*) realloc(p, 64000);
		if(!p) Serial.println("Allocation error ram");
	}
	
	Serial.printf("fs begin %d\n", FILESYSTEM.begin());
	bmp_init();
	bmp_max();
	if (state.maxprog == 0 && state.whdr == 4) state.whdr == 3;
	if (state.maxfold == 0 && state.whdr == 5) state.whdr == 3;
	root = FILESYSTEM.open("/");
	bmp_next();
	delay(100);
	led_drawvcc();

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
		if (conf.enow)
		{
			int px = enow_getorder();
			for (int i = 0; i < px; i++) led_setpxall(i, green);
			led_show();
			enow_begin();
			delay(200);
			int lp = 0;
			while (state.go == false)
			{
				unsigned long time = millis();
				while(millis() - time < 100)
				{
					enow_poll();
					check_off();
				}
				enow_wakeup();
				lp++;
				if(lp == 100) {led_drawvcc(); lp = 0;}
			}
			enow_end();
		}
		else
		{
			Serial.println("wifi_conn");
			if (conf.skwf == false) wifi_conn();
		}
		Serial.println("http_begin");
		http_begin();
		Serial.println("udp_begin");
		udp_begin();
	}
	
	int lp = 0;
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
		lp++;
		if(lp == 1000) {led_drawvcc(); lp = 0;}
		delay(1);
	}
}


void loop()
{
	if (state.go == true && state.maxbmp > 0)
	{
		if (state.whdr == 3 || state.whdr == 5) //files or fold
		{
			bmp_draw(file.path(), state.bpm);
			if (state.loop)
			{
				bmp_next();
			}
			if (state.next)
			{
				bmp_next();
				state.next = false;
			}
			if (state.setbmp)
			{
				while (state.currbmp != state.setbmp)
				{
					bmp_next();
				}
				state.setbmp = 0;
			}
		}
		if (state.whdr == 4) //prog
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
						Serial.println("prog err: no second arg");
						udp_poll();
						if (prgfile.position() >= prgfile.size()){
							prgfile.close();
							Serial.println("file closed");
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
		#ifdef USEBLE
		if (conf.bt) ble_poll();
		#endif
		delay(1);
	}
}
