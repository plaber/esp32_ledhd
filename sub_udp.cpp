#include "sub_udp.h"
#include "conf.h"
#include <driver/adc.h>
#include "sub_bmp.h"
#include "sub_json.h"
#include "sub_led.h"
#include "sub_wifi.h"

WiFiUDP Udp;
char udpBuf[255]; //buffer to hold incoming packet

void udp_begin()
{
	Udp.begin(8888);
}

int get_vcc(bool a)
{
	int ad = 0;
	for (int i = 0; i < 10; i++) ad += adc1_get_raw(VCC_CHN);
	return (a ? 1 : conf.vcc) * ad / 10;
}

int get_vcc()
{
	return get_vcc(false);
}

int vcc2p(int gvcc)
{
	if (isnan(gvcc) || gvcc < 0 || gvcc > 4200) return 101;
	if (gvcc < 3300) return 0;
	const int svcc[12] = {0, 3300, 3400, 3500, 3600, 3700, 3800, 3900, 4000, 4100, 4150, 5000};
	const int pvcc[12] = {0,    1,   12,   23,   34,   45,   56,   67,   78,   89,  100,  101};
	int d = 0;
	for (int i = 0; i < 11; i++)
		if (gvcc >= svcc[i] && gvcc < svcc[i + 1])
		{
			d = i;
			break;
		}
	float ans = pvcc[d] + (gvcc - svcc[d]) / 9.00;
	return ans;
}

String get_answ(String san, String sav)
{
	if (san == "ver")
	{
		uint64_t chipid = ESP.getEfuseMac();
		char buf[45] = {0};
		sprintf(buf, "%s %s_%08X", conf.ver, conf.wpref, (uint32_t) chipid);
		return String(buf);
	}
	if (san == "vcc")
	{
		if (sav == "1")
		{
			int gvcc = get_vcc();
			int pr = vcc2p(gvcc);
			return (String(pr, DEC) + "% (" + String(gvcc, DEC) + "mV) " + WiFi.RSSI());
		}
		else
		{
			conf.vcc = (float)sav.toInt() /  get_vcc(true);
			json_save();
			return String("saved " + String(conf.vcc, DEC));
		}
	}
	if (san == "go")
	{
		if (state.go == false)
		{
			state.go = true;
			led_clear();
			if (state.calcmax)
			{
				bmp_max();
				state.calcmax = false;
			}
		}
		//return ("runned " + String(conf.wait, DEC) + " " + String(conf.brgn, DEC) + " " + String(stat.whdr, DEC) + " " + String(/*whodr1*/0, DEC) + " " + String(/*whodr2*/0, DEC) + " " + String(/*whodr3*/0, DEC) + " " + String(stat.maxbmp, DEC) + " " + String(/*eNow*/0, DEC));
		return ("runned "   + String(conf.wait, DEC) + " " + String(conf.brgn, DEC) + " " + String(state.whdr, DEC) +                     " 0"                             " 0"                             " 0"        " " + String(state.maxbmp, DEC) +       " 0");
	}
	if (san == "stp")
	{
		state.go = false;
		led_clear();
		led_show();
		return "stop";
	}
	if (san == F("info"))
	{
		return (		"w " + String(conf.wait, DEC) +" b "+ String(conf.brgn, DEC) +" m "+ String(state.whdr, DEC) +" p "+ String(state.currprog + 1, DEC) + "/" + String(state.maxprog, DEC) + " b " + String(state.currbmp, DEC) + "/" + String(state.maxbmp, DEC));
	}
	if (san == "btmode")
	{
		if (sav == "one")
		{
			state.loop = false;
			return "mode one";
		}
		if (sav == "loop")
		{
			if (state.loop == false)
			{
				state.loop = true;
				state.next = true;
			}
			return "mode loop";
		}
	}
	if (san == "modeone")
	{
		int savi = sav.toInt();
		if (savi > 0 && savi <= state.maxbmp)
		{
			state.setbmp = savi;
		}
		else
		{
			state.next = true;
		}
		return "next " + sav;
	}
	if (san == "modp")
	{
		if (sav == "m")
		{
			state.loop = false;
			if (state.currbmp > 1) state.setbmp = state.currbmp - 1; else state.setbmp = state.maxbmp;
			return "prev " + String(state.setbmp, DEC);
		}
		if (sav == "p")
		{
			state.loop = false;
			if (state.currbmp < state.maxbmp) state.setbmp = state.currbmp + 1; else state.setbmp = 1;
			return "next " + String(state.setbmp, DEC);
		}
	}
	if (san == "modefile")
	{
		int savi = sav.toInt();
		if (savi > 0 && savi <= state.maxbmp)
		{
			state.setbmp = savi;
		}
		return "file " + sav;
	}
	if (san == "ip")
	{
		IPAddress broadCast = WiFi.localIP();
		return (String(broadCast[0], DEC) + "." + String(broadCast[1], DEC) + "." + String(broadCast[2], DEC) + "." + String(broadCast[3], DEC));
	}
	if (san == "drip")
	{
		IPAddress broadCast = WiFi.localIP();
		led_drawip(broadCast[3]);
		return "draw ip";
	}
	if (san == "brgn" || san == "brg")
	{
		String brgnwarn = "";
		if (sav == "m")
		{
			conf.brgn -= conf.brgn > 8 ? 4 : (conf.brgn > 1 ? 1 : 0);
		}
		if (sav == "p")
		{
			conf.brgn += conf.brgn < 128 ? (conf.brgn < 8 ? 1 : 4) : 0;
			if (conf.brgn == 128) brgnwarn = " max 128";
		}
		int savi = sav.toInt();
		if (savi >= 4 && savi <= 128)
		{
			if (savi == 8 && conf.brgn > 1)
				conf.brgn--;
			else
				conf.brgn = savi;
		}
		if (savi > 128) brgnwarn = " max 128 < " + String(savi);
		led_brgn();
		if (state.go == false) led_show();
		return String(conf.brgn, DEC) + brgnwarn;
	}
	if (san == "wait" || san == "spd")
	{
		if (sav == "m")
		{
			conf.wait -= conf.wait > 0 ? 1 : 0;
		}
		if (sav == "p")
		{
			conf.wait += conf.wait < 200 ? 1 : 0;
		}
		int savi = sav.toInt();
		if (savi > 0 && savi < 200)
		{
			conf.wait = savi;
		}
		return String(conf.wait);
	}
	if (san == "bpm")
	{
		int savi = sav.toInt();
		if (savi > 10 && savi < 65500) state.bpm = savi;
		return String(state.bpm);
	}
	if (san == "cont")
	{
		int savi = sav.toInt();
		savi = savi > 128 ? 128 : savi;
		led_calccont(savi);
		conf.cont = savi;
		return "cont set ok " + sav;
	}
	if (san == "wpref")
	{
		return "name=" + conf.wpref;
	}
	if (san == "heap")
	{
		return String(ESP.getFreeHeap(), DEC);
	}
	if (san == "cmt")
	{
		json_save();
		return "save ok";
	}
	if (san == "blink")
	{
		led_blinkudp();
		return "fill blue";
	}
	if (san == "skwf")
	{
		if (sav == "1")
		{
			conf.skwf = 1;
			return "skip wait wifi on";
		}
		else
		{
			conf.skwf = 0;
			return "skip wait wifi off";
		}
	}
	if (san == "mode")
	{
		if (sav == "1")
		{
			switch (conf.mode)
			{
				case 10: return "poi";
				case 11: return "mask1";
				case 12: return "mask2";
				case 13: return "свой";
				default: return "err";
			}
		}
		else
		{
			int savi = sav.toInt();
			switch (savi)
			{
				case 10: 
					conf.mode = 10;
					#ifndef ARDUINO_ESP32C3_DEV
					conf.leds = 20;
					#else
					conf.leds = 32;
					#endif
					json_save();
					return "poi";
				case 11: conf.mode = 11; conf.leds = 850; json_save(); return "mask1";
				case 12: conf.mode = 12; conf.leds = 850; json_save(); return "mask2";
				case 13: conf.mode = 13; conf.leds = 850; json_save(); return "свой";
				case 3 : state.whdr = 3; json_save(); return "files";
				case 4 : state.whdr = 4; json_save(); return "prog";
				default: return "err mode " + sav;
			}
		}
	}
	if (san == "leds")
	{
		if (sav == "1")
		{
			return String(conf.leds);
		}
		else
		{
			int savi = sav.toInt();
			conf.leds = savi;
			led_init();
			json_save();
			return "saved " + String(conf.leds);
		}
	}
	if (san == "pins")
	{
		char pinb[18];
		sav.toCharArray(pinb,18);
		int pins[6];
		int pinc = sscanf(pinb, "%d%d%d%d%d%d", &pins[0], &pins[1], &pins[2], &pins[3], &pins[4], &pins[5]);
		if (pinc != 6) return "error numbers";
		for (int i = 0; i < 6; i++) if (pins[i] > 34 || pins[i] == 1 || pins[i] == 2 || pins[i] == 3) return "error pin" + String(i)  + " cannot be " + String(pins[i]);
		Preferences preferences;
		preferences.begin("pins", false);
		preferences.putUInt("p0", pins[0]);
		preferences.putUInt("p1", pins[1]);
		preferences.putUInt("p2", pins[2]);
		preferences.putUInt("p3", pins[3]);
		preferences.putUInt("p4", pins[4]);
		preferences.putUInt("p5", pins[5]);
		preferences.end();
		return "saved " + sav;
	}
	if (san == "prog")
	{
		return state.progname;
	}
	if (san == "prg")
	{
		String prgwarn = "";
		if (sav == "m")
		{
			if (state.currprog > 0)
			{
				state.currprog--;
				state.progname = String(state.proglist[state.currprog]);
			}
		}
		if (sav == "p")
		{
			if (state.currprog < state.maxprog - 1)
			{
				state.currprog++;
				state.progname = String(state.proglist[state.currprog]);
			}
		}
		int savi = sav.toInt();
		if (savi > 0 && savi <= state.maxprog)
		{
			state.currprog = savi - 1;
			state.progname = String(state.proglist[state.currprog]);
		}
		return state.progname;
	}
	if (san == "uptime")
	{
		if (sav == "1") state.uptime = 1;
		return String(state.uptime);
	}
	if (san == "restart")
	{
		state.go = false;
		return "restart";
	}
	return "no ans " + san + "=" + sav;
}

void udp_poll()
{
	int packetSize = Udp.parsePacket();
	if (packetSize)
	{
		int len = Udp.read(udpBuf, 255);
		if (len > 0)
		{
			udpBuf[len] = 0;
			String q = String(udpBuf);
			int dei = q.indexOf("=");
			if (dei != -1)
			{
				String sn = q.substring(0, dei);
				String sv = q.substring(dei + 1);
				String ans = get_answ(sn, sv);
				char ansBuf[ans.length() + 1] = {0};
				ans.toCharArray(ansBuf, ans.length() + 1);
				Udp.beginPacket(Udp.remoteIP(), 60201);
				uint8_t an[50] = {0};
				memcpy(an, ansBuf, 49);
				Udp.write(an, ans.length());
				Udp.endPacket();
				if (ans == "restart")
				{
					led_clear();
					delay(500);
					ESP.restart();
				}
			}
		}
	}
}

void udp_sendip()
{
	IPAddress broadCast = WiFi.localIP();
	char ansBuf[21] = {0};
	sprintf(ansBuf, "myip %d.%d.%d.%d", broadCast[0], broadCast[1], broadCast[2], broadCast[3]);
	uint8_t uans[21] = {0};
	memcpy(uans, ansBuf, 21);
	broadCast[3] = 255;
	Udp.beginPacket(broadCast, 60201);
	Udp.write(uans, 21);
	Udp.endPacket();
}