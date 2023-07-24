#pragma once

#ifndef _WIFI_CONTEXT_H
#define _WIFI_CONTEXT_H

extern "C" {
  #include "freertos/timers.h"
}
#include <WiFi.h>
#include "MqttContext.h"


class WiFiContext {

  private:
    static TimerHandle_t _wifiReconnectTimer;
    static MqttContext  _mqttContext;
    static const char* _ssid;
    static const char* _pass;
    static void stopTimer();
    static void startTimer();
    static const char* describeWiFiEvent(WiFiEvent_t event);

  public:
    static void connectToWifi();
    static void WiFiEvent(WiFiEvent_t event);
    static void initializate(MqttContext mqttContext, const char* ssid, const char* pass);

};

#endif // _WIFI_CONTEXT_H
