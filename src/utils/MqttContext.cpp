
#include <Arduino.h>
#include "MqttContext.h"
#include <AsyncMqttClient.h>
#include <WiFi.h>
#include "Logger.h"
#include "configuration.h"

extern "C" {
  #include "freertos/FreeRTOS.h"
  #include "freertos/timers.h"
}

#define timeOfReconect pdMS_TO_TICKS(2000)

#if defined(debug) && defined(debug_mqtt)
#  define LOG_MQTT(...) logger.log(__VA_ARGS__)
#else
#  define LOG_MQTT(...) ((void)0)
#endif

TimerHandle_t MqttContext::_timerReconect = NULL;
AsyncMqttClient MqttContext::_mqttClient;

volatile bool     MqttContext::_connected = false;
volatile uint32_t MqttContext::_connectEpoch = 0;
volatile uint32_t MqttContext::_offlineSinceMs = 0;

extern Logger  logger;

void MqttContext::initializate(const char* mqttServer
                              ,const int mqttPort
                              ,const char* mqttUser
                              ,const char* mqttPass) {
  LOG_MQTT(PSTR("MqttContext::initializate started"));

  _offlineSinceMs = millis();
  _timerReconect = xTimerCreate("mqttTimer", timeOfReconect, pdFALSE, (void*)0, onReconnectTimer);
  _mqttClient.onConnect(onMqttConnect);
  _mqttClient.onDisconnect(onMqttDisconnect);
  _mqttClient.onSubscribe(onMqttSubscribe);
  _mqttClient.onUnsubscribe(onMqttUnsubscribe);
  _mqttClient.onMessage(onMqttMessage);
  _mqttClient.onPublish(onMqttPublish);
  _mqttClient.setServer(mqttServer, mqttPort);
  _mqttClient.setCredentials(mqttUser, mqttPass);

  LOG_MQTT(PSTR("MqttContext::initializate completed"));
}

bool MqttContext::isConnected() {
  return _connected;
}

uint32_t MqttContext::connectEpoch() {
  return _connectEpoch;
}

/* Только заявка на подключение. Раньше здесь вызывался _mqttClient.connect()
   из таска событий WiFi и из таска таймеров одновременно (гонка по _state),
   а завершался вызов delay(1000), который блокировал службу таймеров
   вместе со всеми опросами счётчика. */
void MqttContext::connectToMqtt() {
  LOG_MQTT(PSTR("MqttContext::connectToMqtt requested"));
  MqttContext::startTimer();
}

void MqttContext::onReconnectTimer(TimerHandle_t timer) {
  (void)timer;

  if (_mqttClient.connected()) {
    return;
  }

  if (!WiFi.isConnected()) {
    LOG_MQTT(PSTR("MqttContext::onReconnectTimer no WiFi, waiting"));
    return;  // таймер перезапустит WiFiContext по событию GOT_IP
  }

  LOG_MQTT(PSTR("MqttContext::onReconnectTimer connecting to MQTT..."));
  _mqttClient.connect();

  // Повторная попытка, пока не придёт onMqttConnect.
  MqttContext::startTimer();
}

void MqttContext::onMqttConnect(bool sessionPresent) {
  _connected = true;
  _connectEpoch++;
  LOG_MQTT(PSTR("MqttContext::onMqttConnect connected to MQTT, session present: %d, epoch: %u")
          ,(int)sessionPresent, (unsigned)_connectEpoch);
  MqttContext::stopTimer();
}


void MqttContext::onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  _connected = false;
  _offlineSinceMs = millis();
  LOG_MQTT(PSTR("MqttContext::onMqttDisconnect disconnected from MQTT, reason: %d"), (int)reason);
  if (WiFi.isConnected()) {
    LOG_MQTT(PSTR("MqttContext::onMqttDisconnect MQTT timer reconnect start"));
    MqttContext::startTimer();
  }
}

void MqttContext::supervise() {
#if mqttMaxOfflineMs > 0
  if (_connected) {
    return;
  }
  const uint32_t offlineFor = millis() - _offlineSinceMs;
  if (offlineFor >= mqttMaxOfflineMs) {
    logger.log(PSTR("MqttContext::supervise MQTT offline for %u ms, restarting"), (unsigned)offlineFor);
    Serial.flush();
    ESP.restart();
  }
#endif
}

void MqttContext::stopTimer() {
  if (_timerReconect == NULL) {
    return;
  }
  if (xTimerIsTimerActive(_timerReconect) == pdTRUE) {
    LOG_MQTT(PSTR("MqttContext::stopTimer _timerReconect"));
    xTimerStop(_timerReconect, 0);
  }
}

void MqttContext::startTimer() {
  if (_timerReconect == NULL) {
    return;
  }
  if (xTimerIsTimerActive(_timerReconect) == pdFALSE) {
    LOG_MQTT(PSTR("MqttContext::startTimer _timerReconect"));
    xTimerStart(_timerReconect, 0);
  }
}


void MqttContext::onMqttSubscribe(uint16_t packetId, uint8_t qos) {
  LOG_MQTT(PSTR("MqttContext::onMqttSubscribe Subscribe acknowledged. packetId: %d qos: %d"), packetId, qos);
}

void MqttContext::onMqttUnsubscribe(uint16_t packetId) {
  LOG_MQTT(PSTR("MqttContext::onMqttUnsubscribe Unsubscribe acknowledged. packetId: %d"), packetId);
}

void MqttContext::onMqttPublish(uint16_t packetId) {
  LOG_MQTT(PSTR("MqttContext::onMqttPublish Publish acknowledged. packetId: %d"), packetId);
}


void MqttContext::onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  (void)payload;
  (void)properties;
  // Подписок нет, так что сюда пока не попадаем. Строка из payload больше
  // не собирается: раньше она строилась дважды и выбрасывалась.
  LOG_MQTT(PSTR("MqttContext::onMqttMessage topic %s len %u, index %u, total %u")
          ,topic, (unsigned)len, (unsigned)index, (unsigned)total);
}

bool MqttContext::publishMessage(const char* topic, const char* payload) {
  if (!_connected) {
    LOG_MQTT(PSTR("MqttContext::publishMessage not connected, dropped: %s"), topic);
    return false;
  }

  const uint16_t ret = _mqttClient.publish(topic, mqttQos, mqttRetain, payload);
  LOG_MQTT(PSTR("MqttContext::publishMessage topic: %s packetId: %u"), topic, (unsigned)ret);
  LOG_MQTT(PSTR("%s"), payload);

  // QoS 0 всегда отдаёт 0; для QoS > 0 ноль означает отказ клиента
  // (нет соединения или мало свободной памяти).
  return (mqttQos == 0) || (ret != 0);
}
