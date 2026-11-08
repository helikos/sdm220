#pragma once
#ifndef _MQTT_CONTEXT_H
#define _MQTT_CONTEXT_H

#include <AsyncMqttClient.h>
#include "IPublisher.h"
extern "C" {
  #include "freertos/FreeRTOS.h"
  #include "freertos/timers.h"
}

/* Параметры подключения одной структурой: раньше initializate принимала
   четыре отдельных аргумента, и каждый новый параметр сессии удлинял
   сигнатуру. availabilityTopic == NULL - LWT не используется. */
struct MqttSettings
{
    const char *server;
    int port;
    const char *user;
    const char *pass;
    const char *clientId;           // NULL = сгенерированный библиотекой
    const char *availabilityTopic;  // NULL = без Last Will
    uint16_t keepAliveSeconds;
};

/* Обычный класс с полями, а не набор статиков: AsyncMqttClient теперь
   принадлежит экземпляру, а колбэки привязаны лямбдами, захватывающими this
   (все колбэки библиотеки - std::function). Статическим остался только
   трамплин таймера FreeRTOS, которому нужен C-указатель на функцию. */
class MqttContext : public IPublisher
{
  public:
    MqttContext();

    void initializate(const MqttSettings &settings);

    bool publish(const char* topic, const char* payload) override;
    uint32_t connectEpoch() const override;

    bool isConnected() const;

    /* Можно звать из любого таска: только запускает таймер. Сам connect()
       выполняет колбэк таймера, чтобы _state клиента трогал один таск. */
    void connectToMqtt();

    /* Вызывается из loop(): перезагружает плату, если брокер недоступен
       дольше mqttMaxOfflineMs. */
    void supervise();

    void stopTimer();
    void startTimer();

  private:
    AsyncMqttClient _mqttClient;
    TimerHandle_t   _timerReconect;

    const char *_availabilityTopic;

    volatile bool     _connected;
    volatile uint32_t _connectEpoch;
    volatile uint32_t _offlineSinceMs;

    void onReconnectTimer();
    void onConnect(bool sessionPresent);
    void onDisconnect(AsyncMqttClientDisconnectReason reason);

    static void reconnectTimerTrampoline(TimerHandle_t timer);
};

#endif // _MQTT_CONTEXT_H
