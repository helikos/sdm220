
#include <Arduino.h>

#include "PzemProcessing.h"
#include "registers/pzem/pzemInputRegisters.h"

#include "ModbusMessage.h"
#include "ModbusClientRTU.h"
#include "MqttContext.h"
#include <ArduinoJson.h>

#include "ModbusMqqtProcessing.h"

extern "C" {
  #include "freertos/timers.h"
}

#define timeOfMain pdMS_TO_TICKS(2000)  // 2 sec


const char* PzemProcessing::_mqqtTopic = NULL;
TimerHandle_t PzemProcessing::_timer;
pzemInputRegister PzemProcessing::previousValue;

void PzemProcessing::initializate(MqttContext mqttContext
                                ,const char* mqqtTopic
                                ,const char* mqqtTopicError) {

  ModbusMqqtProcessing::initializate(mqttContext, 9600, &handleData, &handleError, mqqtTopicError);
  _mqqtTopic = mqqtTopic;
  _timer = xTimerCreate("get", timeOfMain, pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(request));
  xTimerStart(_timer, 0);
  
}

void PzemProcessing::request() {
  requestOfMessage();
}

void PzemProcessing::requestOfMessage() {
  Serial.printf("PzemProcessing::requestOfMessage\n");
  ModbusMqqtProcessing::request(0, 0, 10);
};


void PzemProcessing::processingOfMessage(ModbusMessage response) {
  pzemInputRegister input = pzemInputRegister(response);
  if (previousValue.lineToNeutralVolts != -1) {
    if (input.isChanged(previousValue)) {
      Serial.println(input.toJson());
      _mqttContext.publishMessage(_mqqtTopic, input.toJson().c_str());
    }  
  }
  previousValue = input;
  if (xTimerIsTimerActive(_timer) == pdFALSE) {
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
  if (xTimerIsTimerActive(_timer) == pdFALSE) {
    xTimerStart(_timer, 0);
  }
}

