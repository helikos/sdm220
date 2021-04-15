
#include <Arduino.h>

#include "SDM220Processing.h"
#include "SDM220TypeDefs.h"
#include "registers/sdm220/SDM220InputRegisterPower.h"
#include "registers/sdm220/SDM220InputRegisterEnergy.h"
#include "registers/sdm220/SDM220InputRegisterTotal.h"
#include "ModbusMessage.h"
#include "ModbusClientRTU.h"
#include "MqttContext.h"

extern "C" {
  #include "freertos/timers.h"
}

#define timeOfPower pdMS_TO_TICKS(2000)  // 2 sec
#define timeOfEnergy pdMS_TO_TICKS(5000) // 5 sec 
#define timeOfTotal pdMS_TO_TICKS(60000) // 1 minute



const char* SDM220Processing::_mqqtTopicPower = NULL;
const char* SDM220Processing::_mqqtTopicEnergy = NULL;
const char* SDM220Processing::_mqqtTopicTotal = NULL;
TimerHandle_t SDM220Processing::_timerPower;
TimerHandle_t SDM220Processing::_timerEnergy;
TimerHandle_t SDM220Processing::_timerTotal;


void SDM220Processing::initializate(MqttContext mqttContext
                                    ,const char* mqqtTopicPower
                                    ,const char* mqqtTopicEnergy
                                    ,const char* mqqtTopicTotal
                                    ,const char* mqqtTopicError) {

  ModbusMqqtProcessing::initializate(mqttContext, 2400, &handleData, &handleError, mqqtTopicError);

  _mqqtTopicPower = mqqtTopicPower;
  _mqqtTopicEnergy = mqqtTopicEnergy;
  _mqqtTopicTotal = mqqtTopicTotal;

  _timerPower = xTimerCreate("getPower", timeOfPower, pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(requestPower));
  _timerEnergy = xTimerCreate("getEnergy", timeOfEnergy, pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(requestEnergy));
  _timerTotal = xTimerCreate("getTotal", timeOfTotal, pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(requestTotal));
  
  xTimerStart(_timerPower, 0);
  xTimerStart(_timerEnergy, 0);
  xTimerStart(_timerTotal, 0);
}

void SDM220Processing::requestPower() {
  requestOfMessage(thePower);
}
void SDM220Processing::requestEnergy() {
  requestOfMessage(theEnergy);
}
void SDM220Processing::requestTotal() {
  requestOfMessage(theTotal);
}

void SDM220Processing::processingOfMessage(ModbusMessage response, RegistersClass registers) {
  Serial.printf("SDM220Processing::processingOfMessage token %08X\n", (uint32_t)registers);
 
  switch (registers) {
    case thePower: {
      SDM220InputRegisterPower power = SDM220InputRegisterPower(response);
      Serial.println(power.toJson());
      _mqttContext.publishMessage(_mqqtTopicPower, power.toJson().c_str());
      if (xTimerIsTimerActive(_timerPower) == pdFALSE) {
       xTimerStart(_timerPower, 0);
      }
    }
      break;
    case theEnergy: {
      SDM220InputRegisterEnergy energy = SDM220InputRegisterEnergy(response);
      Serial.println(energy.toJson());
      _mqttContext.publishMessage(_mqqtTopicEnergy, energy.toJson().c_str());
      if (xTimerIsTimerActive(_timerEnergy) == pdFALSE) {
       xTimerStart(_timerEnergy, 0);
      }
    }
      break;
    case theTotal: {
      SDM220InputRegisterTotal total = SDM220InputRegisterTotal(response);
      Serial.println(total.toJson());
      _mqttContext.publishMessage(_mqqtTopicTotal, total.toJson().c_str());
      if (xTimerIsTimerActive(_timerTotal) == pdFALSE) {
       xTimerStart(_timerTotal, 0);
      }
    }
      break;
    default:
      Serial.printf("!!!!! SDM220Processing::processingOfMessage unknown token  %08X\n", (uint32_t)registers);
  };
};


void SDM220Processing::requestOfMessage(RegistersClass registers) {
//  RegistersClass registers = (RegistersClass)token;
  Serial.printf("SDM220Processing::requestOfMessage %08X\n", (uint32_t)registers);
  switch (registers) {
    case thePower:
      ModbusMqqtProcessing::request(registers, 0, 42);
      break;
    case theEnergy:
      ModbusMqqtProcessing::request(registers, 70, 12);
      break;
    case theTotal:
      ModbusMqqtProcessing::request(registers, 342, 8);
      break;
    default:
      Serial.printf("SDM220Processing::requestOfMessage unknown token %08X\n", (uint32_t)registers);
  };
};


void SDM220Processing::handleData(ModbusMessage response, uint32_t token) 
{
  ModbusMqqtProcessing::handleData(response, token);
  SDM220Processing::processingOfMessage(response, (RegistersClass)token); 
}

void SDM220Processing::handleError(Error error, uint32_t token) 
{
  ModbusMqqtProcessing::handleError(error, token);

  RegistersClass registers = (RegistersClass)token;
  switch (registers) {
    case thePower: {
      if (xTimerIsTimerActive(_timerPower) == pdFALSE) {
       xTimerStart(_timerPower, 0);
      }
    }
      break;
    case theEnergy: {
      if (xTimerIsTimerActive(_timerEnergy) == pdFALSE) {
       xTimerStart(_timerEnergy, 0);
      }
    }
      break;
    case theTotal: {
      if (xTimerIsTimerActive(_timerTotal) == pdFALSE) {
       xTimerStart(_timerTotal, 0);
      }
    }
      break;
    default:
      Serial.printf("SDM220Processing::handleError unknown token %08X\n", (uint32_t)token);
  };

}
