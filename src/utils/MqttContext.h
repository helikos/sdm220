#pragma once
#ifndef _MQTT_CONTEXT_H
#define _MQTT_CONTEXT_H

#include <AsyncMqttClient.h>
extern "C" {
  #include "freertos/FreeRTOS.h"
  #include "freertos/timers.h"
}

class MqttContext {

  public:
    static void initializate(const char* mqttServer
                            ,const int mqttPort
                            ,const char* mqttUser
                            ,const char* mqttPass);

    /* true = сообщение принято клиентом. false = брокер недоступен, данные
       НЕ отправлены; вызывающий не должен фиксировать состояние "опубликовано"
       и повторит попытку на следующем опросе. */
    static bool publishMessage(const char* topic, const char* payload);

    /* Можно звать из любого таска: только выставляет флаг. Сам connect()
       выполняет колбэк таймера, чтобы _state клиента трогал один таск. */
    static void connectToMqtt();

    static bool isConnected();

    /* Растёт на каждом подключении. Нужен, чтобы после реконнекта
       принудительно переопубликовать все показания. */
    static uint32_t connectEpoch();

    /* Вызывается из loop(): перезагружает плату, если брокер недоступен
       дольше mqttMaxOfflineMs. */
    static void supervise();

    static void stopTimer();
    static void startTimer();

  protected:
   static TimerHandle_t _timerReconect;
   static AsyncMqttClient _mqttClient;

   static volatile bool     _connected;
   static volatile uint32_t _connectEpoch;
   static volatile uint32_t _offlineSinceMs;

   static void onReconnectTimer(TimerHandle_t timer);

   static void onMqttConnect(bool sessionPresent);
   static void onMqttDisconnect(AsyncMqttClientDisconnectReason reason);
   static void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total);
   static void onMqttPublish(uint16_t packetId);
   static void onMqttSubscribe(uint16_t packetId, uint8_t qos);
   static void onMqttUnsubscribe(uint16_t packetId);

};

#endif // _MQTT_CONTEXT_H
