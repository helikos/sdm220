#pragma once

#ifndef _WIFI_CONTEXT_H
#define _WIFI_CONTEXT_H

extern "C" {
  #include "freertos/FreeRTOS.h"
  #include "freertos/timers.h"
}
#include <WiFi.h>
#include "MqttContext.h"

/* Параметры сети одной структурой - так же, как MqttSettings. */
struct WiFiSettings
{
    const char *ssid;
    const char *pass;

    IPAddress ip;
    IPAddress gateway;
    IPAddress subnet;
    IPAddress dns;

    const char *rescueApSsid;
    const char *rescueApPass;  // NULL или короче 8 символов - открытая сеть
};

/* MqttContext хранится по ССЫЛКЕ. Раньше это был статический член-копия,
   который нигде не определялся: линковалось лишь потому, что у класса
   не было полей, и первое же добавленное поле ломало сборку. */
class WiFiContext
{
  public:
    explicit WiFiContext(MqttContext &mqttContext);

    void initializate(const WiFiSettings &settings);
    void connectToWifi();

    /* Вызывается из loop(): ведёт лестницу фоллбэков
       статический IP -> DHCP -> спасательная точка доступа. */
    void supervise();

    /* true - поднят аварийный канал. Пока он поднят, перезагружать плату
       по таймауту MQTT нельзя: это выбьет единственный доступ к /update. */
    bool isRescueApActive() const;

    bool hasIp() const;

  private:
    enum IpMode
    {
        IpModeStatic,
        IpModeDhcp
    };

    MqttContext &_mqttContext;
    TimerHandle_t _wifiReconnectTimer;
    WiFiSettings _settings;

    IpMode _ipMode;
    uint8_t _attemptsInMode;
    bool _hasIp;
    bool _rescueApActive;
    uint32_t _lastIpMs;      // когда последний раз был IP (отсчёт до AP)
    uint32_t _ipAcquiredMs;  // когда получен текущий IP (отсчёт grace брокера)

    void stopTimer();
    void startTimer();
    void onWiFiEvent(WiFiEvent_t event);

    void applyIpConfig();
    void switchIpMode(IpMode mode);
    void startRescueAp();
    void stopRescueAp();

    static const char *describeWiFiEvent(WiFiEvent_t event);
    static const char *describeIpMode(IpMode mode);
    static void reconnectTimerTrampoline(TimerHandle_t timer);
};

#endif // _WIFI_CONTEXT_H
