
#include <Arduino.h>
#include "MqttContext.h"
#include <AsyncMqttClient.h>
#include <WiFi.h>

extern "C" {
  #include "freertos/timers.h"
}

#define timeOfReconect pdMS_TO_TICKS(2000)

TimerHandle_t MqttContext::_timerReconect;
AsyncMqttClient MqttContext::_mqttClient;

void MqttContext::initializate(const char* mqttServer
                              ,const int mqttPort
                              ,const char* mqttUser
                              ,const char* mqttPass) {

  _timerReconect = xTimerCreate("mqttTimer", timeOfReconect, pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(connectToMqtt));
  _mqttClient.onConnect(onMqttConnect);
  _mqttClient.onDisconnect(onMqttDisconnect);
  _mqttClient.onSubscribe(onMqttSubscribe);
  _mqttClient.onUnsubscribe(onMqttUnsubscribe);
  _mqttClient.onMessage(onMqttMessage);
  _mqttClient.onPublish(onMqttPublish);
  _mqttClient.setServer(mqttServer, mqttPort);
  _mqttClient.setCredentials(mqttUser, mqttPass);
}

void MqttContext::connectToMqtt() {
  Serial.println("Connecting to MQTT...");
  if (WiFi.status() == WL_CONNECTED) {
   _mqttClient.connect();
  } 
   delay(2000);
}

void MqttContext::onMqttConnect(bool sessionPresent) {
  Serial.println("Connected to MQTT.");  //  "Подключились по MQTT."
  Serial.print("Session present: ");     //  "Текущая сессия: "
  Serial.println(sessionPresent);
}


void MqttContext::MqttContext::onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.println("Disconnected from MQTT.");
  if (WiFi.isConnected() &&  xTimerIsTimerActive(_timerReconect) == pdFALSE) {
    Serial.println("MQTT Timer reconect start.");
    xTimerStart(_timerReconect, 0);
  }
}

void MqttContext::stopTimer() {
    xTimerStop(_timerReconect, 0);
}

void MqttContext::onMqttSubscribe(uint16_t packetId, uint8_t qos) {
  Serial.printf("Subscribe acknowledged. packetId: %d qos: %d \n", packetId, qos);
}

void MqttContext::onMqttUnsubscribe(uint16_t packetId) {
  Serial.printf("Unsubscribe acknowledged. packetId: %d\n", packetId);
}

void MqttContext::onMqttPublish(uint16_t packetId) {
  Serial.printf("Publish acknowledged. packetId: %d\n", packetId);
}


void MqttContext::onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  String messageTemp;
  for (int i = 0; i < len; i++) {
    messageTemp += (char)payload[i];
  }

  // проверяем, получено ли MQTT-сообщение в топике «esp32/led»:
  if (strcmp(topic, "esp32/led") == 0) {
    Serial.print("  recived ");
  }
  
}

uint16_t MqttContext::publishMessage(const char* topic, const char* payload) {
  if (_mqttClient.connected())
    return _mqttClient.publish(topic, 2, true, payload);
  else 
   return 0;
}

