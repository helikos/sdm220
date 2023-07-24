#pragma once
#ifndef _MQTT_CONTEXT_H
#define _MQTT_CONTEXT_H

#include <AsyncMqttClient.h>
extern "C" {
  #include "freertos/timers.h"
}

class MqttContext {

  public:
    static void initializate(const char* mqttServer
                            ,const int mqttPort
                            ,const char* mqttUser
                            ,const char* mqttPass);
    static uint16_t publishMessage(const char* topic, const char* payload);
    static void connectToMqtt();
    static void stopTimer();
    static void startTimer();

  protected:
   static TimerHandle_t _timerReconect;
   static AsyncMqttClient _mqttClient;
 
   static void onMqttConnect(bool sessionPresent);
   static void onMqttDisconnect(AsyncMqttClientDisconnectReason reason);
   static void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total);
   static void onMqttPublish(uint16_t packetId);
   static void onMqttSubscribe(uint16_t packetId, uint8_t qos);
   static void onMqttUnsubscribe(uint16_t packetId);

};

#endif // _MQTT_CONTEXT_H
