#ifndef SUB_HTTP_H
#define SUB_HTTP_H

#include <WebServer.h>
#if FILESYSTEM == FFat
#include <FFat.h>
#include "HTTPUpdateServerFFat.h"
#endif
#if FILESYSTEM == SPIFFS
#include <SPIFFS.h>
#include <HTTPUpdateServer.h>
#endif
#include <SimpleFTPServer.h>

void http_begin();
void http_poll();
void ftp_poll();

#endif
