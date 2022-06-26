#include "sub_gif.h"
#include "conf.h"
#include "sub_bmp.h"

extern int frmw, frmh;

static int getBit(uint8_t *data, int pos)
{
	int posByte = pos / 8;
	int posBit = pos % 8;
	uint8_t valByte = data[posByte];
	int valInt = valByte >> ((posBit)) & 0x0001;
	return valInt;
}

const int pow2[] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096};

static uint8_t colors[256*3];
static uint16_t colorslen;
static uint16_t dict[4096 * 2], dictlen;
static int pofs;
static uint8_t block[256];
static uint16_t bt, bi, lzb, lz, last;
static uint32_t mil;
bool gct = false;

static void freeall()
{
	colorslen = dictlen = pofs = bt = bi = lzb = lz = last = 0;
}

static bool indict(int el)
{
	return el < (colorslen + 2 + (dictlen / 2));
}

static uint16_t first(uint16_t el)
{
	if (el < colorslen)
	{
		return el;
	}
	else
	{
		if (!indict(el))
		{
			Serial.printf("--- %d not in dict %d %d---\n", el, colorslen, dictlen);
			return 0;
		}
		uint16_t deep = el;
		while((deep = *(dict + (deep - colorslen - 2) * 2)) > colorslen);
		return deep;
	}
}

static void dict_push(uint16_t el1, uint16_t el2)
{
	if (dictlen == 4096 * 2) return;
	dict[dictlen++] = el1;
	dict[dictlen++] = el2;
}

static void add(uint16_t c)
{
	uint16_t stack[512];
	int i = 0;
	stack[i++] = c;

	while (i > 0)
	{
		uint16_t el = stack[--i];
		if (el < colorslen)
		{
			p[pofs++] = (char)el;
		}
		else
		{
			uint16_t c1 = *(dict + (el - colorslen - 2) * 2);
			uint16_t c2 = *(dict + (el - colorslen - 2) * 2 + 1);
			stack[i++] = c2;
			stack[i++] = c1;
		}
	}
}

static void decode(uint16_t idx, bool start)
{
	//if (idx == colorslen || idx == colorslen + 1) return;
	if (start) //decode first pixel
	{
		add(idx);
		last = idx;
		return;
	}
	if (indict(idx))
	{
		dict_push(last, first(idx));
		add(idx);
	}
	else
	{
		dict_push(last, first(last));
		add(colorslen + 2 + (dictlen / 2) -1);
	}
	last = idx;
}

static void extract(uint8_t len)
{
	//Serial.printf("\nextr %03d ", len);
	if (len == 255) Serial.write('.'); else Serial.print(len);
	static bool reinit = true;
	for(int i = 0; i < len * 8; i++)
	{
		int bit = getBit(block, i);
		bt |= (bit << bi);
		bi++;
		if (bi == lz)
		{
			if (bt == colorslen) //clear code
			{
				dictlen = 0;
				lz = lzb;
				bi = 0;
				bt = 0;
				last = 0;
				reinit = true;
				//Serial.write('C');
				continue;
			}
			if (bt == colorslen + 1) //stop code
			{
				bi = 0;
				bt = 0;
				//Serial.write('S');
				break;
			}
			decode(bt, reinit);
			bi = 0;
			bt = 0;
			reinit = false;
			if((colorslen + 2 + (dictlen / 2)) == pow2[lz] && lz < 12) lz++;
		}
	}
	//uint32_t dif = millis() - mil;
	//Serial.println(dif);
}

struct gifheader gif_load(String path, bool ld)
{
	struct gifheader ans;

	if (path.startsWith("/") == false) path = "/" + path;
	Serial.println("load " + path);
	if (FILESYSTEM.exists(path))
	{
		long stm = millis();
		File file = FILESYSTEM.open(path, "r");
		if (file.size() == 0) {Serial.println("zero size"); goto exitreadgif;}
		file.seek(0, SeekSet);
		file.read((uint8_t*)(&ans), 13);
		bmph = ans.h;
		bmpw = ans.w;
		ans.fsz = 0;
		ans.fps = 0;
		if (ans.flag & 0b10000000) //global color table
		{
			ans.cdp = ans.flag & 0b111;
			gct = true;
		}
		else
		{
			ans.cdp = 0;
			gct = false;
		}
		if (ld == false) goto exitreadgif;
		freeall();
		if (ans.cdp)
		{
			int ctsize = pow2[ans.cdp + 1] * 3;
			file.read((uint8_t*)(colors), ctsize);
			memcpy(p + pofs, colors, 768);
			pofs += 768;
			colorslen = ctsize / 3;
			while (file.available())
			{
				ans.fb = file.read();
				if (ans.fb == 0x3B) {Serial.write('\n');goto exitreadgif;}
				if (ans.fb == 0x21)
				{
					file.read(); //scip code
					ans.sz = file.read();
					do
					{
						file.seek(ans.sz, SeekCur); //jump block
					}
					while ((ans.sz = file.read()) != 0);
				}
				if (ans.fb == 0x2C)
				{
					file.read((uint8_t*)(&ans.row), 11);
					lzb = ans.lzw + 1;
					lz = ans.lzw + 1;
					mil = millis();
					do
					{
						file.read((uint8_t*)(block), ans.sz);
						extract(ans.sz);
						ans.fsz += ans.sz;
					} while ((ans.sz = file.read()) != 0);
					if (state.go && conf.mode != 10 && bmpw == frmw) bmp_draw_last(ans.fps);
					ans.fps++;
				}
			}
		}
		else
		{
			while (file.available())
			{
				ans.fb = file.read();
				if (ans.fb == 0x3B) {Serial.write('\n');goto exitreadgif;}
				if (ans.fb == 0x21)
				{
					file.read(); //scip code
					ans.sz = file.read();
					do
					{
						file.seek(ans.sz, SeekCur); //jump block
					}
					while ((ans.sz = file.read()) != 0);
				}
				if (ans.fb == 0x2C)
				{
					file.read((uint8_t*)(&ans.row), 9);
					if (ans.flagb & 0b10000000) //local color table
					{
						int cdp = ans.flagb & 0b111;
						int ctsize = pow2[cdp + 1] * 3;
						file.read((uint8_t*)(colors), ctsize);
						memcpy(p + pofs, colors, 768);
						pofs += 768;
						colorslen = ctsize / 3;
						//Serial.printf("color tab %d %d ", ans.cdp, ctsize);
					}
					ans.lzw = file.read();
					//Serial.printf("lzw %d\n", ans.lzw);
					ans.sz = file.read();
					dictlen = 0; bt = 0; bi = 0;
					lzb = ans.lzw + 1;
					lz = ans.lzw + 1;
					do
					{
						file.read((uint8_t*)(block), ans.sz);
						extract(ans.sz);
						ans.fsz += ans.sz;
					}
					while ((ans.sz = file.read()) != 0);
					if (state.go && conf.mode != 10 && bmpw == frmw) bmp_draw_last(ans.fps);
					ans.fps++;
				}
			}
			ans.fb = 0;
		}
		exitreadgif:
		Serial.printf("dictlen: %d pofs: %d\ntime %d\n", dictlen, pofs, millis() - stm);
		file.close();
		freeall();
	}
	return ans;
}

void gif_load(String path)
{
	gif_load(path, true);
}

