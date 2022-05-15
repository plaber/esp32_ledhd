#include "sub_wifi.h"
#include "conf.h"
#include "sub_led.h"
#include "sub_udp.h"

extern int ssid_count;
extern String ssid[];
extern String pass[];
int wifid = -1; //wifi founded

void wifi_init()
{
	uint64_t chipid = ESP.getEfuseMac();
	char buf[25] = {0};
	sprintf(buf, "%s_%08X", conf.wpref, (uint32_t) chipid);
	WiFi.mode(WIFI_AP_STA);
	WiFi.softAP(buf, "12345678");
}

void wifi_conn()
{
	int scn = WiFi.scanNetworks();
	for (int scni = 0; scni < scn; ++scni)
	{
		Serial.println(WiFi.SSID(scni) + " [" + String(WiFi.RSSI(scni), DEC) + "] {ch " + WiFi.channel(scni) + "}");
	}
	for (int s3 = 0; s3 < ssid_count; s3++)
	{
		for (int scni = 0; scni < scn; ++scni)
		{
			if (ssid[s3].equals(WiFi.SSID(scni)))
			{
				wifid = s3;
				Serial.println("Founded AP: " + ssid[s3]);
				s3 = ssid_count;
				break;
			}
		}
		if (wifid != -1) break;
	}
	if (wifid != -1)
	{
		char fssid[20], fpass[20];
		ssid[wifid].toCharArray(fssid, 20);
		pass[wifid].toCharArray(fpass, 20);
		WiFi.begin(fssid, fpass);
		int trycon = 0;
		while (WiFi.status() != WL_CONNECTED && trycon < 20)
		{
			Serial.print(".");
			led_blink(255, 0, 0);
			trycon++;
		}
		IPAddress broadCast = WiFi.localIP();
		led_drawip(broadCast[3]);
		udp_sendip();
		Serial.println();
		Serial.println("IP address: " + broadCast.toString());
	}
	else
	{
		Serial.println("No saved AP founded");
		if (state.maxbmp > 0) state.go = true;
	}
}
