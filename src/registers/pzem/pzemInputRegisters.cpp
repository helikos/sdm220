
#include "pzemInputRegisters.h"
#include <ArduinoJson.h>
#include <stdlib.h>

typedef union twoWords
{
  uint16_t _16[2];         
  uint32_t _32;
} twoWords;

   pzemInputRegister::pzemInputRegister() {
    lineToNeutralVolts = -1.0;
    current = -1.0;
    activePower = -1.0;
    energyValue = -1.0;
    frequencyValue = -1.0;
    powerFactor = -1.0;
    alarmStatus = 0;
   }


   pzemInputRegister::pzemInputRegister(ModbusMessage response) {
    uint16_t _16;
    twoWords _twoWords;

    response.get(3, _16);
    lineToNeutralVolts = _16*0.1;

    response.get(5, _twoWords._16[0]);
    response.get(7, _twoWords._16[1]);
    current = _twoWords._32*0.001;

    response.get(9, _twoWords._16[0]);
    response.get(11, _twoWords._16[1]);
    activePower = _twoWords._32*0.1;

    response.get(13, _twoWords._16[0]);
    response.get(15, _twoWords._16[1]);
    energyValue = _twoWords._32*1.0;

    response.get(17, _16);
    frequencyValue = _16*0.1;

    response.get(19, _16);
    powerFactor = _16*0.01;

    response.get(21, alarmStatus);

  }

  const char *pzemInputRegister::toString() {
    Serial.printf("Line to neutral volts %f  V\n", lineToNeutralVolts);
    Serial.printf("Current               %f  A\n", current);
    Serial.printf("Active power.         %f  W\n", activePower);
    Serial.printf("Energy value          %f  Wh\n", energyValue);
    Serial.printf("Frequency             %f  Hz\n", frequencyValue);
    Serial.printf("Power factor          %f  \n", powerFactor);
    Serial.printf("Alarm status          %d  \n", alarmStatus);
    return "";
  }

  String pzemInputRegister::toJson() {
    DynamicJsonDocument doc(1024);

    doc["lineVolts"] = lineToNeutralVolts;
    doc["current"] = current;
    doc["activePower"] = activePower;
    doc["energyValue"] = energyValue;
    doc["frequencyValue"] = frequencyValue;
    doc["powerFactor"] = powerFactor;
    String output;
    serializeJson(doc, output);
    return output;

  }

  bool pzemInputRegister::isChanged(pzemInputRegister value) {
    return (abs(value.current - current)/value.current)*100 > 5;
  }