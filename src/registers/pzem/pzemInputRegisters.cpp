
#include "pzemInputRegisters.h"
#include <ArduinoJson.h>
#include <stdlib.h>
#include "utils/Logger.h"
using namespace std;

extern Logger logger;

typedef union twoWords
{
  uint16_t _16[2];
  uint32_t _32;
} twoWords;

pzemInputRegister::pzemInputRegister()
{
  lineToNeutralVolts = -1.0;
  current = -1.0;
  activePower = -1.0;
  energyValue = -1.0;
  frequencyValue = -1.0;
  powerFactor = -1.0;
  alarmStatus = 0;
}

pzemInputRegister::pzemInputRegister(ModbusMessage response)
{
  uint16_t _16;
  twoWords _twoWords;

  response.get(3, _16);
  lineToNeutralVolts = _16 * 0.1;

  response.get(5, _twoWords._16[0]);
  response.get(7, _twoWords._16[1]);
  current = _twoWords._32 * 0.001;

  response.get(9, _twoWords._16[0]);
  response.get(11, _twoWords._16[1]);
  activePower = _twoWords._32 * 0.1;

  response.get(13, _twoWords._16[0]);
  response.get(15, _twoWords._16[1]);
  energyValue = _twoWords._32 * 1.0;

  response.get(17, _16);
  frequencyValue = _16 * 0.1;

  response.get(19, _16);
  powerFactor = _16 * 0.01;

  response.get(21, alarmStatus);
}

const char *pzemInputRegister::toString()
{
  logger.log(PSTR("Line to neutral volts %f  V"), lineToNeutralVolts);
  logger.log(PSTR("Current               %f  A"), current);
  logger.log(PSTR("Active power.         %f  W"), activePower);
  logger.log(PSTR("Energy value          %f  Wh"), energyValue);
  logger.log(PSTR("Frequency             %f  Hz"), frequencyValue);
  logger.log(PSTR("Power factor          %f  "), powerFactor);
  logger.log(PSTR("Alarm status          %d  "), alarmStatus);
  return "";
}

String pzemInputRegister::toJson()
{
  DynamicJsonDocument doc(1024);

  doc[PSTR("lineVolts")] = lineToNeutralVolts;
  doc[PSTR("current")] = current;
  doc[PSTR("activePower")] = activePower;
  doc[PSTR("energyValue")] = energyValue;
  doc[PSTR("frequencyValue")] = frequencyValue;
  doc[PSTR("powerFactor")] = powerFactor;
  String output;
  serializeJson(doc, output);
  return output;
}

bool pzemInputRegister::isChanged(pzemInputRegister value) 
{
  return (abs(value.current - current) / value.current) * 100 > 5;
}
