
#include <Arduino.h>

#include "PzemProcessing.h"
#include "configuration.h"
#include "PublishGate.h"
#include "registers/pzem/pzemInputRegisters.h"

#include "ModbusMessage.h"
#include "ModbusClientRTU.h"
#include "utils/MqttContext.h"
#include "utils/Logger.h"
#include <ArduinoJson.h>

#include "ModbusMqqtProcessing.h"

extern "C" {
  #include "freertos/FreeRTOS.h"
  #include "freertos/timers.h"
}

#define timeOfMain pdMS_TO_TICKS(2000)  // 2 sec

extern Logger logger;

const char* PzemProcessing::_mqqtTopic = NULL;
TimerHandle_t PzemProcessing::_timer = NULL;
pzemInputRegister PzemProcessing::_lastPublishedValue;
PublishGate PzemProcessing::_gate(heartbeatMsPzem);

void PzemProcessing::initializate(MqttContext mqttContext
                                ,const char* mqqtTopic
                                ,const char* mqqtTopicError) {

  ModbusMqqtProcessing::initializate(mqttContext, 9600, &handleData, &handleError, mqqtTopicError);
  _mqqtTopic = mqqtTopic;
  _timer = xTimerCreate("get", timeOfMain, pdFALSE, (void*)0, onPollTimer);
  xTimerStart(_timer, 0);
  
}

void PzemProcessing::onPollTimer(TimerHandle_t timer) {
  (void)timer;
  if (!requestOfMessage() && _timer != NULL) {
    // Запрос отвергнут клиентом eModbus: колбэков не будет, перезапускаем
    // таймер сами, иначе опрос останавливается навсегда.
    logger.log(PSTR("PzemProcessing::onPollTimer request rejected, retry scheduled"));
    xTimerStart(_timer, 0);
  }
}

bool PzemProcessing::requestOfMessage() {
  logger.log(PSTR("PzemProcessing::requestOfMessage"));
  return ModbusMqqtProcessing::request(0, 0, 10);
};


void PzemProcessing::processingOfMessage(ModbusMessage response) {
  pzemInputRegister input = pzemInputRegister(response);
  const String payload = input.toJson();
  logger.log(PSTR("%s"), payload.c_str());

  if (_gate.needsForcedPublish() || input.isChanged(_lastPublishedValue)) {
    if (_mqttContext.publishMessage(_mqqtTopic, payload.c_str())) {
      _lastPublishedValue = input;
      _gate.onPublished();
    }
  }

  if (_timer != NULL && xTimerIsTimerActive(_timer) == pdFALSE) {
    xTimerStart(_timer, 0);
  }
};

void PzemProcessing::handleData(ModbusMessage response, uint32_t token) {
  ModbusMqqtProcessing::handleData(response, token);
  PzemProcessing::processingOfMessage(response); 
}

void PzemProcessing::handleError(Error error, uint32_t token) 
{
  ModbusMqqtProcessing::handleError(error, token);
  if (_timer != NULL && xTimerIsTimerActive(_timer) == pdFALSE) {
    xTimerStart(_timer, 0);
  }
}
