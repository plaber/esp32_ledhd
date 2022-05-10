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

void setup();
void loop();

#endif
