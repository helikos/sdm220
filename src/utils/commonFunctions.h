#pragma once
#ifndef _COMMONFUNCTIONS_H
#define _COMMONFUNCTIONS_H

#include <string>
#include <ESPAsyncWebServer.h>

using namespace std;

string getPadded(int num, uint8_t n);

void getFreeHeapSize(AsyncWebServerRequest *request);

/* Мёртвая зона без деления: значение считается изменившимся, если
   |current - previous| >= max(absTolerance, relPercent% от |previous|).
   Абсолютный порог задаёт пол чувствительности около нуля - раньше здесь
   было деление на previous, дававшее inf/NaN при нулевой нагрузке. */
bool isSignificantChange(float current, float previous, float absTolerance, float relPercent);


#endif
