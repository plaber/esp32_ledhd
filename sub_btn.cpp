#include "sub_btn.h"
#include "conf.h"
#include "sub_json.h"
#include "sub_led.h"
#include "sub_enow.h"

static int onoff = 0, offp = 0;
static unsigned long offm = 0;

void check_off()
{
	if (digitalRead(conf.pinb) == HIGH) {
		if (onoff == 0)
		{
			onoff = 1;
			Serial.println("released");
		}
		if (offp > 0 && offm != millis())
		{
			offp -= 1;
			offm = millis();
		}
	}
	if (digitalRead(conf.pinb) == LOW && onoff == 1)
	{
		if (offp == 0 && conf.enow)
		{
			char ans[8];
			sprintf(ans, "iamon%02d", enow_getorder());
			enow_send(ans);
		}
		if (offm != millis())
		{
			offp += 3;
			offm = millis();
		}
		//Serial.printf("OFF %d / %d\n", offp, 1000);
		if (offp > 1000)
		{
			digitalWrite(conf.pinp, HIGH);
			pinMode(conf.pinp, INPUT);
			if (conf.brgn == 4) led_brgn(2); else led_brgn(4);
			led_show();
		}
	}
}

void check_up()
{
	int showUp = 0;
	while (digitalRead(conf.pinb) == LOW && showUp < 20)
	{
		delay(50);
		showUp++;
	}
	if (digitalRead(conf.pinb) == LOW)
	{
		int pixBut = -1;
		while (pixBut < 31 && digitalRead(conf.pinb) == LOW)
		{
			pixBut++;
			if (pixBut == 4)
			{
				led_setpxall(10, green);
				led_setpxall(16, red);
				#ifdef USEBLE
				led_setpxall(22, blue);
				#endif
			}
			#ifdef USEBLE
			if (pixBut != 10 && pixBut != 16 && pixBut != 22 && pixBut < 28)
			#else
			if (pixBut != 10 && pixBut != 16 && pixBut < 28)
			#endif
			{
				led_setpxall(pixBut, dwhite);
				Serial.write('.');
			}
			else
			{
				Serial.write('!');
			}
			led_show();
			delay(100);
		}
		if (pixBut > 10 && pixBut <= 16)
		{
			Serial.println("Bt off, skwf off");
			led_setpxall(5, green);
			led_show();
			delay(500);
			conf.bt = false;
			conf.skwf = false;
			conf.enow = false;
			json_save();
		}
		if (pixBut > 16 && pixBut <= 22)
		{
			Serial.println("Bt off, skwf on");
			led_setpxall(5, red);
			led_show();
			delay(500);
			conf.bt = false;
			conf.skwf = true;
			conf.enow = true;
			json_save();
		}
		#ifdef USEBLE
		if (pixBut > 22 && pixBut <= 28)
		{
			Serial.println("Bt on, skwf on");
			led_setpxall(5, blue);
			led_show();
			delay(500);
			conf.bt = true;
			conf.skwf = true;
			conf.enow = false;
			json_save();
		}
		#endif
		led_clear();
	}
}

