
#include <Arduino.h>
#include "MqttContext.h"
#include <AsyncMqttClient.h>
#include <WiFi.h>
#include "Logger.h"
#include "configuration.h"

extern "C" {
  #include "freertos/timers.h"
}

#define timeOfReconect pdMS_TO_TICKS(2000)

TimerHandle_t MqttContext::_timerReconect;
AsyncMqttClient MqttContext::_mqttClient;
extern Logger  logger;

void MqttContext::initializate(const char* mqttServer
                              ,const int mqttPort
                              ,const char* mqttUser
                              ,const char* mqttPass) {
#ifdef debug    
#ifdef debug_mqtt
  logger.log(PSTR("MqttContext::initializate started"));
#endif
#endif

  _timerReconect = xTimerCreate("mqttTimer", timeOfReconect, pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(connectToMqtt));
  _mqttClient.onConnect(onMqttConnect);
  _mqttClient.onDisconnect(onMqttDisconnect);
  _mqttClient.onSubscribe(onMqttSubscribe);
  _mqttClient.onUnsubscribe(onMqttUnsubscribe);
  _mqttClient.onMessage(onMqttMessage);
  _mqttClient.onPublish(onMqttPublish);
  _mqttClient.setServer(mqttServer, mqttPort);
  _mqttClient.setCredentials(mqttUser, mqttPass);

#ifdef debug    
#ifdef debug_mqtt
  logger.log(PSTR("MqttContext::initializate completed"));
#endif
#endif

}

void MqttContext::connectToMqtt() {
#ifdef debug    
#ifdef debug_mqtt
  logger.log(PSTR("MqttContext::connectToMqtt Connecting to MQTT..."));
#endif
#endif
  if (WiFi.status() == WL_CONNECTED) {
#ifdef debug    
#ifdef debug_mqtt
   logger.log(PSTR("MqttContext::connectToMqtt Try to connect to MQTT..."));
#endif
#endif
   _mqttClient.connect();
#ifdef debug    
#ifdef debug_mqtt
  logger.log(PSTR("MqttContext::connectToMqtt completed"));
#endif
#endif
  } 
  delay(1000);
}

void MqttContext::onMqttConnect(bool sessionPresent) {
#ifdef debug    
#ifdef debug_mqtt
  logger.log(PSTR("MqttContext::onMqttConnect Connected to MQTT."));  
  logger.log(PSTR("MqttContext::onMqttConnect Session present: %u"), sessionPresent);     
#endif
#endif
  MqttContext::stopTimer();
}


void MqttContext::MqttContext::onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
#ifdef debug    
#ifdef debug_mqtt
  logger.log(PSTR("MqttContext::onMqttDisconnect Disconnected from MQTT."));
#endif
#endif
  if (WiFi.isConnected()) {
#ifdef debug    
#ifdef debug_mqtt
  logger.log(PSTR("MqttContext::onMqttDisconnect MQTT Timer reconect start."));
#endif
#endif
    MqttContext::startTimer();
  }
}

void MqttContext::stopTimer() {
  if (xTimerIsTimerActive(_timerReconect) == pdTRUE) {
#ifdef debug    
#ifdef debug_mqtt
  logger.log(PSTR("MqttContext::stopTimer _timerReconect"));
#endif
#endif
    xTimerStop(_timerReconect, 0);
  }
}

void MqttContext::startTimer() {
  if (xTimerIsTimerActive(_timerReconect) == pdFALSE) {
#ifdef debug    
#ifdef debug_mqtt
  logger.log(PSTR("MqttContext::stopTimer _timerReconect"));
#endif
#endif
    xTimerStart(_timerReconect, 0);
  }
}


void MqttContext::onMqttSubscribe(uint16_t packetId, uint8_t qos) {
#ifdef debug    
#ifdef debug_mqtt
  logger.log(PSTR("MqttContext::onMqttSubscribe Subscribe acknowledged. packetId: %d qos: %d"), packetId, qos);
#endif
#endif
}

void MqttContext::onMqttUnsubscribe(uint16_t packetId) {
#ifdef debug    
#ifdef debug_mqtt
  logger.log(PSTR("MqttContext::onMqttUnsubscribe Unsubscribe acknowledged. packetId: %d"), packetId);
#endif
#endif
}

void MqttContext::onMqttPublish(uint16_t packetId) {
#ifdef debug    
#ifdef debug_mqtt
  logger.log(PSTR("MqttContext::onMqttPublish Publish acknowledged. packetId: %d"), packetId);
#endif
#endif
}


void MqttContext::onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
#ifdef debug    
#ifdef debug_mqtt
//  if (strcmp(topic, _mqqtTopic) == 0) {
    String messageTemp;
    for (int i = 0; i < len; i++) {
      messageTemp += (char)payload[i];
    }
    logger.log(PSTR("MqttContext::onMqttMessage topic %s"), topic);
    logger.log(PSTR("MqttContext::onMqttMessage len %d, index %d, total %d"), len, index, total);
    logger.log(PSTR("%s"), messageTemp.c_str());
//  }
#endif
#endif
//  if (strcmp(topic, _mqqtTopic) == 0) {
    String message;
    for (int i = 0; i < len; i++) {
      message += (char)payload[i];
    }
//    proceedMqttMessage(message);
//  }
  
}

uint16_t MqttContext::publishMessage(const char* topic, const char* payload) {
#ifdef debug    
#ifdef debug_mqtt
  logger.log(PSTR("MqttContext::publishMessage: topic:%s"), topic);
  logger.log(PSTR("%s"), payload);
#endif
#endif
  if (_mqttClient.connected())
    return _mqttClient.publish(topic, 2, true, payload);
  else 
   return 0;
}
