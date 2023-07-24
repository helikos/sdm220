#pragma once
#ifndef _COMMONFUNCTIONS_H
#define _COMMONFUNCTIONS_H

#include <string>
#include <ESPAsyncWebServer.h>

using namespace std;

string getPadded(int num, uint8_t n);

void getFreeHeapSize(AsyncWebServerRequest *request);


#endif
