#ifndef SUB_JSON_H
#define SUB_JSON_H

#if FILESYSTEM == FFat
#include <FFat.h>
#endif
#if FILESYSTEM == SPIFFS
#include <SPIFFS.h>
#endif
#include <Preferences.h>

void json_save();
void json_load();

#endif
