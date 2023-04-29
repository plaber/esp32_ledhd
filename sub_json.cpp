#include "sub_json.h"
#include "conf.h"

extern String ssid[];
extern String pass[];

String str_encode(String in)
{
	int len = in.length();
	char ans[(len*2) + 1];
	int i = 0; 
	for (int j = 0; j < len; j++)
	{
		sprintf((char*)(ans + i), "%02X", in.charAt(j));
		i += 2;
	}
	ans[i] = 0;
	return String(ans);
}

String str_decode(String in)
{
	int len = in.length();
	char ans[(len/2) + 1];
	char buf[] = "00";
	for (int j = 0; j < len; j += 2)
	{
		buf[0] = in.charAt(j);
		buf[1] = in.charAt(j + 1);
		ans[j / 2] = strtol(buf, NULL, 16);
	}
	ans[len / 2] = 0;
	return String(ans);
}

void mac_decode(String in, uint8_t *ans)
{
	int len = in.length();
	char buf[] = "00";
	for (int j = 0; j < len; j += 2)
	{
		if (j == 12) break;
		buf[0] = in.charAt(j);
		buf[1] = in.charAt(j + 1);
		ans[j / 2] = strtol(buf, NULL, 16);
	}
}

void json_save()
{
	Preferences preferences;
	preferences.begin("conf", false);
	if (conf.wpref != "LedHDxx" && conf.wpref != "LedHD72" && conf.wpref != "LedHD80")
	{
		preferences.putString("wprf", conf.wpref);
	}
	else
	{
		preferences.remove("wprf");
	}
	preferences.putUChar( "mlen", conf.macslen);
	preferences.putBytes( "mmac", conf.macs, 96);
	preferences.putInt(   "wait", conf.wait);
	preferences.putUChar( "brgn", conf.brgn);
	preferences.putUChar( "mode", conf.mode);
	preferences.putInt(   "leds", conf.leds);
	preferences.putFloat( "vccc", conf.vcc);
	preferences.putInt(   "cont", conf.cont);
	preferences.putBool(  "skwf", conf.skwf);
	preferences.putBool(  "blth", conf.bt);
	preferences.putBool(  "enow", conf.enow);
	preferences.putBool(  "enow1", conf.enowone);
	preferences.putInt(   "whdr", state.whdr);
	preferences.putUShort("bpms", state.bpm);
	if (state.progname != "no_prog")
	{
		preferences.putString("prog", state.progname);
	}
	else
	{
		preferences.remove("prog");
	}
	if (ssid[1] != "spiffs" && pass[1] != "spiffs")
	{
		preferences.putString("ssd1", ssid[1]);
		preferences.putString("pss1", pass[1]);
	}
	else
	{
		preferences.remove("ssd1");
		preferences.remove("pss1");
	}
	if (ssid[2] != "spiffs" && pass[2] != "spiffs")
	{
		preferences.putString("ssd2", ssid[2]);
		preferences.putString("pss2", pass[2]);
	}
	else
	{
		preferences.remove("ssd2");
		preferences.remove("pss2");
	}
	preferences.end();
}

void json_load()
{
	Preferences preferences;
	preferences.begin("conf", true);
	String tmp_wprf = preferences.getString("wprf", ""); 
	if (tmp_wprf != "") conf.wpref = tmp_wprf;
	conf.macslen = preferences.getUChar( "mlen", 0);
	preferences.getBytes( "mmac", conf.macs, 96);
	conf.wait = preferences.getInt(   "wait", 0);
	conf.brgn = preferences.getUChar( "brgn", 4);
	conf.mode = preferences.getUChar( "mode", 10);
	conf.leds = preferences.getInt(   "leds", 20);
	conf.vcc  = preferences.getFloat( "vccc", 2.0);
	conf.cont = preferences.getInt(   "cont", 0);
	conf.skwf = preferences.getBool(  "skwf", false);
	conf.bt   = preferences.getBool(  "blth", false);
	conf.enow = preferences.getBool(  "enow", false);
	conf.enowone = preferences.getBool(  "enow1", false);
	state.whdr = preferences.getInt(  "whdr", 3);
	state.bpm  = preferences.getUShort("bpms", 4000);
	state.progname = preferences.getString("prog", "no_prog");
	ssid[1] = preferences.getString("ssd1", "spiffs");
	pass[1] = preferences.getString("pss1", "spiffs");
	ssid[2] = preferences.getString("ssd2", "spiffs");
	pass[2] = preferences.getString("pss2", "spiffs");
	preferences.end();
	if (conf.enowone)
	{
		preferences.begin("conf", false);
		preferences.putBool(  "skwf", false);
		preferences.putBool(  "enow", false);
		preferences.putBool(  "enow1", false);
		preferences.end();
	}
}

void json_del()
{
	Preferences preferences;
	preferences.begin("conf", false);
	conf.macslen = 0;
	memset(conf.macs, 0, 96);
	state.progname = "no_prog";
	preferences.putUChar( "mlen", conf.macslen);
	preferences.putBytes( "mmac", conf.macs, 96);
	preferences.remove("prog");
	preferences.end();
}

void json_clear()
{
	nvs_flash_erase();
	nvs_flash_init();
}
