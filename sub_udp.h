#ifndef SUB_UDP_H
#define SUB_UDP_H

#include <WiFiUdp.h>
#include <Preferences.h>
#include <driver/adc.h>

void udp_begin();
int get_vcc(bool a);
int get_vcc();
int vcc2p(int gvcc);
String get_answ(String san, String sav);
void udp_poll();
void udp_sendip();
void udp_sendmac();
void udp_echo(uint8_t *buf, size_t s);

#endif
