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

void json_save()
{
	String jsonString = "";
	jsonString += "wait=" + String(conf.wait, DEC) + "\n";
	jsonString += "brgn=" + String(conf.brgn, DEC) + "\n";
	jsonString += "mode=" + String(conf.mode, DEC) + "\n";
	jsonString += "leds=" + String(conf.leds, DEC) + "\n";
	jsonString += "vccc=" + String(conf.vcc, 3) + "\n";
	jsonString += "cont=" + String(conf.cont, DEC) + "\n";
	jsonString += "skwf=" + String(conf.skwf, DEC) + "\n";
	jsonString += "blth=" + String(conf.bt, DEC) + "\n";
	jsonString += "bpms=" + String(state.bpm, DEC) + "\n";
	if (ssid[1] != "spiffs" && pass[1] != "spiffs")
	{
		jsonString += "ssd1=" + ssid[1] + "\n";
		jsonString += "pss1=" + str_encode(pass[1]) + "\n";
	}
	if (ssid[2] != "spiffs" && pass[2] != "spiffs")
	{
		jsonString += "ssd2=" + ssid[2] + "\n";
		jsonString += "pss2=" + str_encode(pass[2]) + "\n";
	}
	if (conf.wpref != "LedHD")
	{
		jsonString += "wprf=" + conf.wpref + "\n";
	}
	jsonString += "whdr=" + String(state.whdr, DEC) + "\n";
	jsonString += "prog=" + state.progname;
	File c = FILESYSTEM.open("/config.txt", "w");
	c.print(jsonString);
	c.close();
}

void json_load()
{
	if (FILESYSTEM.exists("/config.txt"))
	{
		File cnffile = FILESYSTEM.open("/config.txt", "r");
		while (cnffile.available())
		{
			String confs = cnffile.readStringUntil('\n');
			String sn = confs.substring(0, 4);
			String sv = confs.substring(4 + 1);
			if(sn == "wait") conf.wait = sv.toInt();
			if(sn == "brgn") conf.brgn = sv.toInt();
			if(sn == "mode") conf.mode = sv.toInt();
			if(sn == "leds") conf.leds = sv.toInt();
			if(sn == "vccc") conf.vcc = sv.toFloat();
			if(sn == "cont") conf.cont = sv.toInt();
			if(sn == "skwf") conf.skwf = sv.toInt();
			if(sn == "blth") conf.bt = sv.toInt();
			if(sn == "bpms") state.bpm = sv.toInt();
			if(sn == "ssd1") ssid[1] = sv;
			if(sn == "pss1") pass[1] = str_decode(sv);
			if(sn == "ssd2") ssid[2] = sv;
			if(sn == "pss2") pass[2] = str_decode(sv);
			if(sn == "wprf") conf.wpref = sv;
			if(sn == "whdr") state.whdr = sv.toInt();
			if(sn == "prog") state.progname = sv;
		}
		cnffile.close();
	}
}
