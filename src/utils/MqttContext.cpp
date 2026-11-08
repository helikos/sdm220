
#include <Arduino.h>
#include "MqttContext.h"
#include <AsyncMqttClient.h>
#include <WiFi.h>
#include "Log.h"
#include "configuration.h"

extern "C" {
  #include "freertos/FreeRTOS.h"
  #include "freertos/timers.h"
}

#define timeOfReconect pdMS_TO_TICKS(2000)

MqttContext::MqttContext()
    : _timerReconect(NULL)
    , _availabilityTopic(NULL)
    , _connected(false)
    , _connectEpoch(0)
    , _offlineSinceMs(0)
{
}

void MqttContext::initializate(const MqttSettings &settings) {
  LOG_INIT(PSTR("MqttContext::initializate started"));

  _availabilityTopic = settings.availabilityTopic;
  _offlineSinceMs = millis();
  _timerReconect = xTimerCreate("mqttTimer", timeOfReconect, pdFALSE, this, reconnectTimerTrampoline);

  _mqttClient.onConnect([this](bool sessionPresent) { onConnect(sessionPresent); });
  _mqttClient.onDisconnect([this](AsyncMqttClientDisconnectReason reason) { onDisconnect(reason); });
  _mqttClient.onSubscribe([](uint16_t packetId, uint8_t qos) {
    LOG_MQTT(PSTR("MqttContext Subscribe acknowledged. packetId: %d qos: %d"), packetId, qos);
  });
  _mqttClient.onUnsubscribe([](uint16_t packetId) {
    LOG_MQTT(PSTR("MqttContext Unsubscribe acknowledged. packetId: %d"), packetId);
  });
  _mqttClient.onPublish([](uint16_t packetId) {
    LOG_MQTT(PSTR("MqttContext Publish acknowledged. packetId: %d"), packetId);
  });
  _mqttClient.onMessage([](char* topic, char* payload, AsyncMqttClientMessageProperties properties
                          ,size_t len, size_t index, size_t total) {
    (void)payload;
    (void)properties;
    // Подписок нет, так что сюда пока не попадаем.
    LOG_MQTT(PSTR("MqttContext::onMqttMessage topic %s len %u, index %u, total %u")
            ,topic, (unsigned)len, (unsigned)index, (unsigned)total);
  });

  _mqttClient.setServer(settings.server, settings.port);
  _mqttClient.setCredentials(settings.user, settings.pass);
  _mqttClient.setKeepAlive(settings.keepAliveSeconds);

  if (settings.clientId != NULL) {
    _mqttClient.setClientId(settings.clientId);
  }

  if (_availabilityTopic != NULL) {
    // Last Will: offline публикует БРОКЕР, когда клиент пропал, не прислав
    // DISCONNECT - в том числе при обрыве WiFi и при ESP.restart().
    // retain обязателен, иначе подписавшийся позже не увидит состояние.
    _mqttClient.setWill(_availabilityTopic, mqttQos, true, availabilityOffline);
    LOG_INIT(PSTR("MqttContext::initializate will topic: %s"), _availabilityTopic);
  }

  LOG_INIT(PSTR("MqttContext::initializate completed"));
}

bool MqttContext::isConnected() const {
  return _connected;
}

uint32_t MqttContext::connectEpoch() const {
  return _connectEpoch;
}

void MqttContext::connectToMqtt() {
  LOG_MQTT(PSTR("MqttContext::connectToMqtt requested"));
  startTimer();
}

void MqttContext::reconnectTimerTrampoline(TimerHandle_t timer) {
  static_cast<MqttContext*>(pvTimerGetTimerID(timer))->onReconnectTimer();
}

void MqttContext::onReconnectTimer() {
  if (_mqttClient.connected()) {
    return;
  }

  if (!WiFi.isConnected()) {
    LOG_MQTT(PSTR("MqttContext::onReconnectTimer no WiFi, waiting"));
    return;  // таймер перезапустит WiFiContext по событию GOT_IP
  }

  LOG_MQTT(PSTR("MqttContext::onReconnectTimer connecting to MQTT..."));
  _mqttClient.connect();

  // Повторная попытка, пока не придёт onConnect.
  startTimer();
}

void MqttContext::onConnect(bool sessionPresent) {
  _connected = true;
  _connectEpoch++;
  LOG_MQTT(PSTR("MqttContext::onConnect connected to MQTT, session present: %d, epoch: %u")
          ,(int)sessionPresent, (unsigned)_connectEpoch);

  if (_availabilityTopic != NULL) {
    publish(_availabilityTopic, availabilityOnline);
  }

  stopTimer();
}

void MqttContext::onDisconnect(AsyncMqttClientDisconnectReason reason) {
  _connected = false;
  _offlineSinceMs = millis();
  LOG_MQTT(PSTR("MqttContext::onDisconnect disconnected from MQTT, reason: %d"), (int)reason);
  if (WiFi.isConnected()) {
    LOG_MQTT(PSTR("MqttContext::onDisconnect MQTT timer reconnect start"));
    startTimer();
  }
}

void MqttContext::supervise() {
#if mqttMaxOfflineMs > 0
  if (_connected) {
    return;
  }
  const uint32_t offlineFor = millis() - _offlineSinceMs;
  if (offlineFor >= mqttMaxOfflineMs) {
    LOG(PSTR("MqttContext::supervise MQTT offline for %u ms, restarting"), (unsigned)offlineFor);
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

bool MqttContext::publish(const char* topic, const char* payload) {
  if (!_connected) {
    LOG_MQTT(PSTR("MqttContext::publish not connected, dropped: %s"), topic);
    return false;
  }

  const uint16_t ret = _mqttClient.publish(topic, mqttQos, mqttRetain, payload);
  LOG_MQTT(PSTR("MqttContext::publish topic: %s packetId: %u"), topic, (unsigned)ret);
  LOG_MQTT(PSTR("%s"), payload);

  // QoS 0 всегда отдаёт 0; для QoS > 0 ноль означает отказ клиента
  // (нет соединения или мало свободной памяти).
  return (mqttQos == 0) || (ret != 0);
}
