#include "sub_jpg.h"
#include "conf.h"

bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap)
{
	//Serial.printf("x %03d y %03d w %03d h %03d\n", x, y, w, h);
	for (int i = 0; i < h; i++)
	{
		for (int j = 0; j < w; j++)
		{
			int idx = i * w + j;
			uint16_t px = bitmap[idx];
			uint16_t red = (px >> 11);
			uint16_t grn = ((px >> 6) & 0b11111);
			uint16_t blu = (px & 0b11111);
			p[(x + j + bmpw * (y + i)) * 3 + 0] = (char)(blu << 3);
			p[(x + j + bmpw * (y + i)) * 3 + 1] = (char)(grn << 3);
			p[(x + j + bmpw * (y + i)) * 3 + 2] = (char)(red << 3);
		}
	}
	// Return 1 to decode next block
	return 1;
}

struct jpgheader jpg_header(String path, bool rundec)
{
	struct jpgheader ans = {0, 0, 1};
	if (path.startsWith("/") == false) path = "/" + path;
	Serial.println("load " + path);
	
	File file = FILESYSTEM.open(path, "r");
	if (file.size() == 0){
		file.close();
		return ans;
	}
	ans.size = file.size();
	TJpgDec.getFsJpgSize(&ans.w, &ans.h, path, FILESYSTEM);
	bmph = ans.h;
	bmpw = ans.w;
	
	if (rundec)
	{
		TJpgDec.setJpgScale(1);
		TJpgDec.setCallback(tft_output);
		TJpgDec.drawFsJpg(0, 0, path, FILESYSTEM);
	}
	file.close();
	return ans;
}

void jpg_load(String path)
{
	jpg_header(path, true);
}

