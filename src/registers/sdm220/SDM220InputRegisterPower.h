#ifndef _SDM220_INPUT_REGISTER_POWER_H
#define _SDM220_INPUT_REGISTER_POWER_H

#include <Arduino.h>
#include "ModbusMessage.h"

/*
 30001 Line to neutral volts.   Volts      00 00
 30007 Current.                 Amps       00 06
 30013 Active power.            Watts      00 0C
 30019 Apparent power           VoltAmps   00 12
 30025 Reactive power           VAr        00 18
 30031 Power factor             None       00 1E
 30037 Phase angle.             Degree     00 24
*/

class SDM220InputRegisterPower
{
public:
  float lineToNeutralVolts;
  float current;
  float activePower;
  float apparentPower;
  float reactivePower;
  float powerFactor;
  float phaseAngle;

  explicit SDM220InputRegisterPower();
  explicit SDM220InputRegisterPower(const ModbusMessage &response);
  bool isChanged(const SDM220InputRegisterPower &value) const;

  String toJson() const;
};

#endif // _SDM220_INPUT_REGISTER_FIRST_H
