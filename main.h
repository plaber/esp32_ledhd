#ifndef MAIN_H
#define MAIN_H

#include <Arduino.h>
#include <HardwareSerial.h>
#if FILESYSTEM == FFat
#include <FFat.h>
#endif
#if FILESYSTEM == SPIFFS
#include <SPIFFS.h>
#endif
#include <driver/adc.h>
#include <Preferences.h>

void setup();
void loop();

#endif
