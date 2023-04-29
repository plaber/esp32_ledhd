#ifndef SUB_JSON_H
#define SUB_JSON_H

#if FILESYSTEM == FFat
#include <FFat.h>
#endif
#if FILESYSTEM == SPIFFS
#include <SPIFFS.h>
#endif
#include <Preferences.h>
#include <nvs_flash.h>

String str_encode(String in);
void mac_decode(String in, uint8_t *ans);
void json_save();
void json_load();
void json_del();

#endif
