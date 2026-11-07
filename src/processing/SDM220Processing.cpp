
#include <Arduino.h>

#include "SDM220Processing.h"
#include "SDM220TypeDefs.h"
#include "configuration.h"
#include "PublishGate.h"
#include "registers/sdm220/SDM220InputRegisterPower.h"
#include "registers/sdm220/SDM220InputRegisterEnergy.h"
#include "registers/sdm220/SDM220InputRegisterTotal.h"
#include "ModbusMessage.h"
#include "ModbusClientRTU.h"
#include "utils/MqttContext.h"
#include "utils/Logger.h"

extern "C"
{
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
}

#define timeOfPower pdMS_TO_TICKS(1000)    // 1 sec
#define timeOfEnergy pdMS_TO_TICKS(120000) // 2 min
#define timeOfTotal pdMS_TO_TICKS(180000)  // 3 minute

extern Logger logger;

const char *SDM220Processing::_mqqtTopicPower = NULL;
const char *SDM220Processing::_mqqtTopicEnergy = NULL;
const char *SDM220Processing::_mqqtTopicTotal = NULL;
TimerHandle_t SDM220Processing::_timerPower = NULL;
TimerHandle_t SDM220Processing::_timerEnergy = NULL;
TimerHandle_t SDM220Processing::_timerTotal = NULL;

SDM220InputRegisterEnergy SDM220Processing::_lastPublishedEnergy;
SDM220InputRegisterPower SDM220Processing::_lastPublishedPower;
SDM220InputRegisterTotal SDM220Processing::_lastPublishedTotal;

PublishGate SDM220Processing::_powerGate(heartbeatMsPower);
PublishGate SDM220Processing::_energyGate(heartbeatMsEnergy);
PublishGate SDM220Processing::_totalGate(heartbeatMsTotal);

void SDM220Processing::initializate(MqttContext mqttContext, const char *mqqtTopicPower, const char *mqqtTopicEnergy, const char *mqqtTopicTotal, const char *mqqtTopicError)
{

  ModbusMqqtProcessing::initializate(mqttContext, 2400, &handleData, &handleError, mqqtTopicError);

  _mqqtTopicPower = mqqtTopicPower;
  _mqqtTopicEnergy = mqqtTopicEnergy;
  _mqqtTopicTotal = mqqtTopicTotal;

  _timerPower = xTimerCreate("getPower", timeOfPower, pdFALSE, (void *)thePower, onPollTimer);
  _timerEnergy = xTimerCreate("getEnergy", timeOfEnergy, pdFALSE, (void *)theEnergy, onPollTimer);
  _timerTotal = xTimerCreate("getTotal", timeOfTotal, pdFALSE, (void *)theTotal, onPollTimer);

  xTimerStart(_timerPower, 0);
  xTimerStart(_timerEnergy, 0);
  xTimerStart(_timerTotal, 0);
}

TimerHandle_t SDM220Processing::timerFor(RegistersClass registers)
{
  switch (registers)
  {
  case thePower:
    return _timerPower;
  case theEnergy:
    return _timerEnergy;
  case theTotal:
    return _timerTotal;
  default:
    return NULL;
  }
}

void SDM220Processing::onPollTimer(TimerHandle_t timer)
{
  requestOrRetry((RegistersClass)(uint32_t)pvTimerGetTimerID(timer));
}

void SDM220Processing::requestOrRetry(RegistersClass registers)
{
  if (requestOfMessage(registers))
    return;

  // Запрос не попал в очередь eModbus, значит ни handleData, ни handleError
  // не вызовутся и перезапустить таймер больше некому: без этого цепочка
  // опроса умирала до перезагрузки.
  TimerHandle_t timer = timerFor(registers);
  if (timer != NULL)
  {
    logger.log(PSTR("SDM220Processing::requestOrRetry request rejected, retry scheduled %08X"), (unsigned)registers);
    xTimerStart(timer, 0);
  }
}

void SDM220Processing::processingOfMessage(ModbusMessage response, RegistersClass registers)
{
  logger.log(PSTR("SDM220Processing::processingOfMessage token %08X"), (unsigned)registers);
  switch (registers) {
    case thePower: {
      SDM220InputRegisterPower power = SDM220InputRegisterPower(response);
      const String payload = power.toJson();
      logger.log(PSTR("%s"), payload.c_str());
      if (_powerGate.needsForcedPublish() || power.isChanged(_lastPublishedPower)) {
        // Состояние фиксируем только после успешной отправки: при недоступном
        // брокере следующий опрос повторит попытку.
        if (_mqttContext.publishMessage(_mqqtTopicPower, payload.c_str())) {
          _lastPublishedPower = power;
          _powerGate.onPublished();
        }
      }
      xTimerStart(_timerPower, 0);
    }
    break;
    case theEnergy: {
      SDM220InputRegisterEnergy energy = SDM220InputRegisterEnergy(response);
      const String payload = energy.toJson();
      logger.log(PSTR("%s"), payload.c_str());
      if (_energyGate.needsForcedPublish() || energy.isChanged(_lastPublishedEnergy)) {
        if (_mqttContext.publishMessage(_mqqtTopicEnergy, payload.c_str())) {
          _lastPublishedEnergy = energy;
          _energyGate.onPublished();
        }
      }
      xTimerStart(_timerEnergy, 0);
    }
    break;
    case theTotal: {
      SDM220InputRegisterTotal total = SDM220InputRegisterTotal(response);
      const String payload = total.toJson();
      logger.log(PSTR("%s"), payload.c_str());
      if (_totalGate.needsForcedPublish() || total.isChanged(_lastPublishedTotal)) {
        if (_mqttContext.publishMessage(_mqqtTopicTotal, payload.c_str())) {
          _lastPublishedTotal = total;
          _totalGate.onPublished();
        }
      }
      xTimerStart(_timerTotal, 0);
    }
    break;
    default:
      logger.log(PSTR("!!!!! SDM220Processing::processingOfMessage unknown token  %08X"), (unsigned)registers);
  };
};

bool SDM220Processing::requestOfMessage(RegistersClass registers)
{
  logger.log(PSTR("SDM220Processing::requestOfMessage %08X"), (unsigned)registers);
  switch (registers)
  {
  case thePower:
    return ModbusMqqtProcessing::request(registers, 0, 42);
  case theEnergy:
    return ModbusMqqtProcessing::request(registers, 70, 12);
  case theTotal:
    return ModbusMqqtProcessing::request(registers, 342, 8);
  default:
    logger.log(PSTR("SDM220Processing::requestOfMessage unknown token %08X"), (unsigned)registers);
    return false;
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
  logger.log(PSTR("SDM220Processing::handleError token %08X"), (unsigned)token);

  TimerHandle_t timer = timerFor((RegistersClass)token);
  if (timer != NULL) {
    xTimerStart(timer, 0);
  } else {
    logger.log(PSTR("SDM220Processing::handleError unknown token %08X"), (unsigned)token);
  }
}
