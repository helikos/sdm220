#ifndef _PZEM_INPUT_REGISTERS_H
#define _PZEM_INPUT_REGISTERS_H

#include "ModbusMessage.h"
#include "../inputRegisterParameters.h"

/*
 30001 Line to neutral volts.   Volts      00 00
 30007 Current.                 Amps       00 06
 30013 Active power.            Watts      00 0C
 30019 Apparent power           VoltAmps   00 12
 30025 Reactive power           VAr        00 18
 30031 Power factor             None       00 1E
 30037 Phase angle.             Degree     00 24
*/

class pzemInputRegister : public InputRegisterParameters
{
public:
  float lineToNeutralVolts;
  float current;
  float activePower;
  float energyValue;
  float frequencyValue;
  float powerFactor;
  uint32_t alarmStatus;

  explicit pzemInputRegister();
  explicit pzemInputRegister(ModbusMessage response);
  bool isChanged(pzemInputRegister value);

  const char *toString();
  String toJson();
};

#endif // _SDM220_INPUT_REGISTER_FIRST_H
