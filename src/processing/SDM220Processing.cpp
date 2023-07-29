
#include <Arduino.h>

#include "SDM220Processing.h"
#include "SDM220TypeDefs.h"
#include "registers/sdm220/SDM220InputRegisterPower.h"
#include "registers/sdm220/SDM220InputRegisterEnergy.h"
#include "registers/sdm220/SDM220InputRegisterTotal.h"
#include "ModbusMessage.h"
#include "ModbusClientRTU.h"
#include "utils/MqttContext.h"
#include "utils/Logger.h"

extern "C"
{
#include "freertos/timers.h"
}

#define timeOfPower pdMS_TO_TICKS(1000)    // 1 sec
#define timeOfEnergy pdMS_TO_TICKS(120000) // 2 min
#define timeOfTotal pdMS_TO_TICKS(180000)  // 3 minute

extern Logger logger;

const char *SDM220Processing::_mqqtTopicPower = NULL;
const char *SDM220Processing::_mqqtTopicEnergy = NULL;
const char *SDM220Processing::_mqqtTopicTotal = NULL;
TimerHandle_t SDM220Processing::_timerPower;
TimerHandle_t SDM220Processing::_timerEnergy;
TimerHandle_t SDM220Processing::_timerTotal;
SDM220InputRegisterEnergy SDM220Processing::previousEnergyValue;
SDM220InputRegisterPower SDM220Processing::previousPowerValue;
SDM220InputRegisterTotal SDM220Processing::previousTotalValue;

void SDM220Processing::initializate(MqttContext mqttContext, const char *mqqtTopicPower, const char *mqqtTopicEnergy, const char *mqqtTopicTotal, const char *mqqtTopicError)
{

  ModbusMqqtProcessing::initializate(mqttContext, 2400, &handleData, &handleError, mqqtTopicError);

  _mqqtTopicPower = mqqtTopicPower;
  _mqqtTopicEnergy = mqqtTopicEnergy;
  _mqqtTopicTotal = mqqtTopicTotal;

  _timerPower = xTimerCreate(PSTR("getPower"), timeOfPower, pdFALSE, (void *)0, reinterpret_cast<TimerCallbackFunction_t>(requestPower));
  _timerEnergy = xTimerCreate(PSTR("getEnergy"), timeOfEnergy, pdFALSE, (void *)0, reinterpret_cast<TimerCallbackFunction_t>(requestEnergy));
  _timerTotal = xTimerCreate(PSTR("getTotal"), timeOfTotal, pdFALSE, (void *)0, reinterpret_cast<TimerCallbackFunction_t>(requestTotal));

  xTimerStart(_timerPower, 0);
  xTimerStart(_timerEnergy, 0);
  xTimerStart(_timerTotal, 0);
}

void SDM220Processing::requestPower()
{
  requestOfMessage(thePower);
}
void SDM220Processing::requestEnergy()
{
  requestOfMessage(theEnergy);
}
void SDM220Processing::requestTotal()
{
  requestOfMessage(theTotal);
}

void SDM220Processing::processingOfMessage(ModbusMessage response, RegistersClass registers)
{
  logger.log(PSTR("SDM220Processing::processingOfMessage token %08X"), (uint32_t)registers);
  switch (registers) {
    case thePower: {
      SDM220InputRegisterPower power = SDM220InputRegisterPower(response);
      logger.log(power.toJson().c_str());
      if (previousPowerValue.activePower != -1 && power.isChanged(previousPowerValue))
        _mqttContext.publishMessage(_mqqtTopicPower, power.toJson().c_str());
      previousPowerValue = power;
//      if (xTimerIsTimerActive(_timerPower) == pdFALSE)
        xTimerStart(_timerPower, 0);
    }
    break;
    case theEnergy: {
      SDM220InputRegisterEnergy energy = SDM220InputRegisterEnergy(response);
      logger.log(energy.toJson().c_str());
      if (previousEnergyValue.exportActiveEnergy != -1 && energy.isChanged(previousEnergyValue))
        _mqttContext.publishMessage(_mqqtTopicEnergy, energy.toJson().c_str());
      previousEnergyValue = energy;
//      if (xTimerIsTimerActive(_timerEnergy) == pdFALSE)
      xTimerStart(_timerEnergy, 0);
    }
    break;
    case theTotal: {
      SDM220InputRegisterTotal total = SDM220InputRegisterTotal(response);
      logger.log(total.toJson().c_str());
      if (previousTotalValue.totalActiveEnergy != -1 && total.isChanged(previousTotalValue))
        _mqttContext.publishMessage(_mqqtTopicTotal, total.toJson().c_str());
      previousTotalValue = total;
//      if (xTimerIsTimerActive(_timerTotal) == pdFALSE)
        xTimerStart(_timerTotal, 0);
    }
    break;
    default:
      logger.log(PSTR("!!!!! SDM220Processing::processingOfMessage unknown token  %08X"), (uint32_t)registers);
  };
};

void SDM220Processing::requestOfMessage(RegistersClass registers)
{
  //  RegistersClass registers = (RegistersClass)token;
  logger.log(PSTR("SDM220Processing::requestOfMessage %08X"), (uint32_t)registers);
  switch (registers)
  {
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
    logger.log(PSTR("SDM220Processing::requestOfMessage unknown token %08X"), (uint32_t)registers);
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
  logger.log(PSTR("SDM220Processing::handleError token %08X"), (uint32_t)token);

  RegistersClass registers = (RegistersClass)token;
  switch (registers) {
    case thePower: {
      xTimerStart(_timerPower, 0);
    }
    break;
    case theEnergy: {
      xTimerStart(_timerEnergy, 0);
    }
    break;
    case theTotal: {
      xTimerStart(_timerTotal, 0);
    }
    break;
    default:
    logger.log(PSTR("SDM220Processing::handleError unknown token %08X"), (uint32_t)token);
  };
}
