#include "sub_bmp.h"
#include "conf.h"
#ifdef USEBLE
#include "sub_ble.h"
#endif
#include "sub_btn.h"
#include "sub_gif.h"
#include "sub_http.h"
#include "sub_led.h"
#include "sub_udp.h"

extern File root;
extern File file;

static bool isgif = false;

struct bmpheader bmp_load(String path, bool ld)
{
	struct bmpheader ans;
	uint32_t rsiz = 0;
	
	if (path.startsWith("/") == false) path = "/" + path;
	Serial.print("load " + path);
	if (FILESYSTEM.exists(path))
	{
		File file = FILESYSTEM.open(path, "r");
		if (file.size() == 0) {Serial.println("zero size"); goto exitreadbmp;}
		
		file.seek(0, SeekSet);
		file.read((uint8_t*)(&ans), 14);
		file.read((uint8_t*)(&ans.tp), 20);
		bmpw = ans.w;
		bmph = ans.h;
		switch (ans.tp)
		{
			case 12:  strcpy(ans.bminfo, "core");break;
			case 40:  strcpy(ans.bminfo, "v3");  break;
			case 108: strcpy(ans.bminfo, "v4");  break;
			case 124: strcpy(ans.bminfo, "v5");  break;
			default:  strcpy(ans.bminfo, "unkn");
		}
		Serial.printf(" %u %u\n", ans.w, ans.h);
		if (ld == false) goto exitreadbmp;
		
		file.seek(ans.offset , SeekSet);
		rsiz = ans.w * 3;
		if (rsiz % 4 != 0) rsiz += 4 - (rsiz % 4);

		for (int j = 0; j < ans.h; j++) {
			file.seek(ans.offset + rsiz * (ans.h - 1 -j), SeekSet);
			file.readBytes((p + ans.w * 3 * j), ans.w * 3);
		}
		exitreadbmp:
		file.close();
	}
	return ans;
}

void bmp_load(String path)
{
	bmp_load(path, true);
}

bool bmp_check(String path)
{
	return path.endsWith(exbmp) || path.endsWith(exgif);
}


void bmp_draw(String path, unsigned long tm)
{
	isgif = path.endsWith(exgif);
	if (conf.mode == 10 || conf.mode == 14)
	{
		bmp_draw_poi(path, tm);
	}
	else
	{
		bmp_draw_mask(path, tm);
	}
}

void bmp_draw_poi(String path, unsigned long tm)
{
	if (path.startsWith("/") == false) path = "/" + path;
	if (bmp_check(path) && FILESYSTEM.exists(path)) {
		unsigned long stm = millis(); //Serial.println(stm);
		if (path.endsWith(exbmp)) bmp_load(path);
		if (path.endsWith(exgif)) gif_load(path);
		while ((millis() - stm) < tm || state.loop == false)
		{
			if (state.go == false || state.next || (state.loop == false && state.setbmp > 0 && state.currbmp != state.setbmp)) break;
			for (int j = 0; j < bmpw; j++)
			{
				if (state.go == false || state.next) break;
				if ((millis() - stm) > tm && tm != 0 && state.loop == true)
				{
					led_clear();
					led_show();
					return;
				}
				for (int i = 0; i < bmph; i++)
				{
					RgbColor color(0,0,0);
					if (isgif)
					{
						int cidx = *(p + j + bmpw * i + 768) * 3;
						color.B = *(p + cidx + 2);
						color.G = *(p + cidx + 1);
						color.R = *(p + cidx + 0);
					}
					else
					{
						color.B = *(p + (j + bmpw * i) * 3 + 0);
						color.G = *(p + (j + bmpw * i) * 3 + 1);
						color.R = *(p + (j + bmpw * i) * 3 + 2);
					}
					led_setpx(bmph - 1 - i, color);
				}
				led_show();
				check_off();
				if (conf.wait > 0) delayMicroseconds(conf.wait * 100);
			}
			udp_poll();
			http_poll();
		}
		#ifdef USEBLE
		if (conf.bt)
		{
			ble_poll();
		}
		#endif
		led_clear();
		led_show();
	} 
	else
	{
		Serial.println(" no file or its not bmp " + path + " " + path.length());
		http_poll();
		udp_poll();
		check_off();
		delay(1);
	}
}

static char cordslen, swidx, swidx2;
int frmw = 1, frmh = 1;
char *cords;

static int px;
static bool framed = false;
extern bool gct;

void bmp_setpx(int x, int y, int frm, int line)
{
	int ofs = 0;
	if (framed)
	{
		ofs = (bmpw * y + frm * bmpw * frmh + x);
	}
	else
	{
		ofs = (bmpw * y + frm * frmw + x);
	}
	RgbColor color(0,0,0);
	if (isgif)
	{
		if (framed)
		{
			if (gct)
			{
				int cidx = *(p + ofs + 768) * 3;
				color.B = *(p + cidx + 2);
				color.G = *(p + cidx + 1);
				color.R = *(p + cidx + 0);
			}
			else
			{
				int tidx = (768 + bmpw * frmh) * frm;
				int cidx = *(p + ofs + 768 * (frm + 1)) * 3;
				color.B = *(p + tidx + cidx + 2);
				color.G = *(p + tidx + cidx + 1);
				color.R = *(p + tidx + cidx + 0);
			}
		}
		else
		{
			int cidx = *(p + ofs + 768) * 3;
			color.B = *(p + cidx + 2);
			color.G = *(p + cidx + 1);
			color.R = *(p + cidx + 0);
		}
	}
	else
	{
		color.B = *(p + ofs * 3 + 0);
		color.G = *(p + ofs * 3 + 1);
		color.R = *(p + ofs * 3 + 2);
	}
	led_setpx(px, color, line);
	px++;
}

void bmp_drawline(int x1, int y1, int x2, int y2, int frm, int line) {
	const int deltaX = abs(x2 - x1);
	const int deltaY = abs(y2 - y1);
	const int signX = x1 < x2 ? 1 : -1;
	const int signY = y1 < y2 ? 1 : -1;
	int error = deltaX - deltaY;
	while(x1 != x2 || y1 != y2) 
	{
		bmp_setpx(x1, y1, frm, line);
		int error2 = error * 2;
		if(error2 > -deltaY) 
		{
			error -= deltaY;
			x1 += signX;
		}
		if(error2 < deltaX) 
		{
			error += deltaX;
			y1 += signY;
		}
	}
	bmp_setpx(x2, y2, frm, line);
}

void bmp_draw_mask(String path, unsigned long tm)
{
	if (path.startsWith("/") == false) path = "/" + path;
	if (bmp_check(path) && FILESYSTEM.exists(path))
	{
		unsigned long stm = millis();
		int fps = 0;
		framed = false;
		if (path.endsWith(exbmp))
		{
			bmp_load(path);
			fps = bmpw / frmw;
		}
		if (path.endsWith(exgif))
		{
			gifheader g = gif_load(path, true);
			fps = g.fps;
			if (fps > 1)
			{
				framed = true;
			}
			else
			{
				fps = bmpw / frmw;
			}
		}
		Serial.printf("fps %d framed %d %cct %d fps\n\n", fps, framed, isgif ? (gct ? 'G' : 'L') : 'N', fps);
		int line;
		while ((millis() - stm) < tm || state.loop == false)
		{
			if (state.go == false || state.next || (state.loop == false && state.setbmp > 0 && state.currbmp != state.setbmp)) break;
			for (int f = 0; f < fps; f++)
			{
				if (state.go == false || state.next || (state.loop == false && state.setbmp > 0 && state.currbmp != state.setbmp)) break;
				//left side
				px = 0;
				line = 0;
				for (int i = 0; i < cordslen; i++)
				{
					if (swidx && i == swidx) {px = 0; line = 2;}
					if (swidx2 && i == swidx2) {px = 0; line = 4;}
					bmp_drawline(cords[i * 4 + 0], cords[i * 4 + 1], cords[i * 4 + 2], cords[i * 4 + 3], f, line);//led_show();bmpwait(50);
				}
				//right side
				px = 0;
				line = 1;
				for (int i = 0; i < cordslen; i++)
				{
					if (swidx && i == swidx) {px = 0; line = 3;}
					if (swidx2 && i == swidx2) {px = 0; line = 5;}
					if ((conf.mode == 11 || conf.mode ==12) && i >= swidx + 11 && i < swidx + 55)
					{
						bmp_drawline(36 + cords[i * 4 + 0] - 1, cords[i * 4 + 1], 36 + cords[i * 4 + 2] - 1, cords[i * 4 + 3], f, line);//led_show();bmpwait(50);
					}
					else
					{
						bmp_drawline(frmw - cords[i * 4 + 0] - 1, cords[i * 4 + 1], frmw - cords[i * 4 + 2] - 1, cords[i * 4 + 3], f, line);//led_show();bmpwait(50);
					}
				}
				led_show();
				udp_poll();
			}
			udp_poll();
			http_poll();
		}
		if (conf.bt)
		{
			#ifdef USEBLE
			ble_poll();
			#endif
		}
	}
	else
	{
		Serial.println(" no file or its not bmp " + path);
		http_poll();
		udp_poll();
		check_off();
		delay(1);
	}
}

void bmp_draw_last(int fps)
{
	int line;
	framed = true;
	px = 0;
	line = 0;
	for (int i = 0; i < cordslen; i++)
	{
		if (swidx && i == swidx) {px = 0; line = 2;}
		if (swidx2 && i == swidx2) {px = 0; line = 4;}
		bmp_drawline(cords[i * 4 + 0], cords[i * 4 + 1], cords[i * 4 + 2], cords[i * 4 + 3], fps, line);//led_show();bmpwait(50);
	}
	px = 0;
	line = 1;
	for (int i = 0; i < cordslen; i++)
	{
		if (swidx && i == swidx) {px = 0; line = 3;}
		if (swidx2 && i == swidx2) {px = 0; line = 5;}
		if ((conf.mode == 11 || conf.mode ==12) && i >= swidx + 11 && i < swidx + 55)
		{
			bmp_drawline(36 + cords[i * 4 + 0] - 1, cords[i * 4 + 1], 36 + cords[i * 4 + 2] - 1, cords[i * 4 + 3], fps, line);//led_show();bmpwait(50);
		}
		else
		{
			bmp_drawline(frmw - cords[i * 4 + 0] - 1, cords[i * 4 + 1], frmw - cords[i * 4 + 2] - 1, cords[i * 4 + 3], fps, line);//led_show();bmpwait(50);
		}
	}
	led_show();
	udp_poll();
	delay(25);
}

void bmp_init()
{
	if (conf.mode == 11 || (conf.mode == 13 && FILESYSTEM.exists("/costume.txt") == false))
	{
		cordslen = CLEN1;
		Serial.printf("Allocate cords %d\n", cordslen);
		cords = (char*) realloc(cords, cordslen * 4);
		if(!cords) Serial.println("Allocation error cords");
		swidx = SWIDX1;
		swidx2 = SWIDX12;
		frmw = 47;
		frmh = FRMH1;
		memcpy_P(cords, cords1, cordslen * 4);
	}
	if (conf.mode == 12)
	{
		cordslen = CLEN2;
		Serial.printf("Allocate cords %d\n", cordslen);
		cords = (char*) realloc(cords, cordslen * 4);
		if(!cords) Serial.println("Allocation error cords");
		swidx = SWIDX2;
		swidx2 = SWIDX22;
		frmw = 47;
		frmh = FRMH2;
		memcpy_P(cords, cords2, cordslen * 4);
	}
	if (conf.mode == 13 && FILESYSTEM.exists("/costume.txt") == true)
	{
		bmp_loadcost();
	}
}

void bmp_max()
{
	int st = millis();
	File rt = FILESYSTEM.open("/");
	File fl = rt.openNextFile();
	int p = 0, pm = 0;
	char fnd = 0;
	while (fl)
	{
		String f = fl.name();
		if (bmp_check(f)) p++;
		if (f.startsWith("prog") && f.endsWith(extxt))
		{
			if (f == state.progname)
			{
				state.currprog = pm;
				fnd = 1;
			}
			if (pm < 15)
			{
				f.toCharArray(state.proglist[pm], 32);
				pm++;
			}
		}
		fl = rt.openNextFile();
	}
	if (fnd == 0 && pm > 0)
	{
		state.progname = String(state.proglist[0]);
		state.currprog = 0;
	}
	Serial.printf("maxbmp %d prog %d %dms\n", p, pm, millis()-st);
	state.maxprog = pm;
	state.maxbmp = p;
}

String bmp_conf()
{
	String ans = "<script>\n"
		"load({w:'" + String(frmw, DEC) + "',h:'" +  String(frmh, DEC) + "',cl:'" + String(cordslen, DEC) + 
		"',sw:'" + String(swidx, DEC) + "',sw2:'" + String(swidx2, DEC) + "',f1:'";
	char buf[20];
	for (int i = 0; i < cordslen; i++)
	{
		sprintf(buf, i == 0 ? "%3d %3d %3d %3d\0" : "\\n%3d %3d %3d %3d\0", cords[i * 4 + 0], cords[i * 4 + 1], cords[i * 4 + 2], cords[i * 4 + 3]);
		ans += String(buf);
	}
	ans += "'});\n</script>";
	return ans;
}

void bmp_savecost()
{
	File c = FILESYSTEM.open("/costume.txt", "w");
	c.print("cswd=" + String(frmw, DEC) + "\n");
	c.print("csht=" + String(frmh, DEC) + "\n");
	c.print("coln=" + String(cordslen, DEC) + "\n");
	c.print("cos1=" + String(swidx, DEC) + "\n");
	c.print("cos2=" + String(swidx2, DEC) + "\n");
	char buf[20];
	for (int i = 0; i < cordslen; i++)
	{
		sprintf(buf, i == 0 ? "%3d %3d %3d %3d\0" : "\\n%3d %3d %3d %3d\0", cords[i * 4 + 0], cords[i * 4 + 1], cords[i * 4 + 2], cords[i * 4 + 3]);
		c.print(buf);
	}
	c.close();
}

void bmp_loadcost()
{
	File cnffile = FILESYSTEM.open("/costume.txt", "r");
	int i = 0;
	String stcut;
	char sbuf[30];
	int aq, a1, a2, a3, a4;
	while (cnffile.available())
	{
		stcut = cnffile.readStringUntil('\n');
		String sn = stcut.substring(0, 4);
		String sv = stcut.substring(4 + 1);
		if (i == 0)
			frmw = sv.toInt();
		else if (i == 1)
			frmh = sv.toInt();
		else if (i == 2) {
			cordslen = sv.toInt();
			Serial.printf("Allocate cords %d\n", cordslen);
			cords = (char*) realloc(cords, cordslen * 4);
			if(!cords) Serial.println("Allocation error cords");
		} else if (i == 3) {
			swidx = sv.toInt(); Serial.println("swidx " + String(swidx, DEC));
		} else if (i == 4) {
			swidx2 = sv.toInt(); Serial.println("swidx2 " + String(swidx2, DEC));
		} else
		{
			if (i - 5 == cordslen) {break;Serial.println("break");}
			Serial.printf("scan line %d\n", i);
			stcut.toCharArray(sbuf, 30);
			aq = sscanf(sbuf, "%d%d%d%d", &a1, &a2, &a3, &a4);
			if (aq == 4)
			{
				cords[(i - 5) * 4 + 0] = (char)a1;
				cords[(i - 5) * 4 + 1] = (char)a2;
				cords[(i - 5) * 4 + 2] = (char)a3;
				cords[(i - 5) * 4 + 3] = (char)a4;
			}
		}
		i++;
	}
	cnffile.close();
}

void bmp_wait(int ms)
{
	int st = millis();
	while(millis() - st < ms){ udp_poll();http_poll(); }
}

void bmp_rainbow()
{
	static int hue = 0;
	HsbColor hsb(hue / 360.0f, 1, 1);
	hue += 3;
	hue = hue == 360 ? 0 : hue;
	RgbColor rgb(hsb);
	led_clearto(rgb.R, rgb.G, rgb.B);
	led_show();
}

void bmp_batlog()
{
	if (state.uptime)
	{
		unsigned long now = millis();
		if(now > state.uptime + 60000)
		{
			File batlog = FILESYSTEM.open("/batlog_" + String(conf.brgn, DEC) + extxt, "a");
			batlog.printf("%.2f\n", conf.vcc * get_vcc());
			batlog.close();
			state.uptime = millis();
		}
	}
}

void bmp_next()
{
	if (state.maxbmp > 0)
	{
		String fname;
		do
		{
			file = root.openNextFile();
			if (!file)
			{
				root = FILESYSTEM.open("/");
				file = root.openNextFile();
				state.currbmp = 0;
			}
			fname = file.name();
		}
		while(!bmp_check(fname));
		state.currbmp++;
		state.currname = fname;
	}
}
