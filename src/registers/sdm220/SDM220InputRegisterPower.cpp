
#include "SDM220InputRegisterPower.h"
#include <ArduinoJson.h>
#include "configuration.h"
#include "utils/commonFunctions.h"


// #include "ModbusMessage.h"

/*
 30001 Line to neutral volts.   Volts      00 00
 30007 Current.                 Amps       00 06
 30013 Active power.            Watts      00 0C
 30019 Apparent power           VoltAmps   00 12
 30025 Reactive power           VAr        00 18
 30031 Power factor             None       00 1E
 30037 Phase angle.             Degree     00 24

 30071 Frequency                Hz         00 46
 30073 Import active energy     kwh        00 48
 30075 Export active energy     kwh        00 4A
 30077 Import reactive energy   kvarh      00 4C
 30079 Export reactive energy   kvarh      00 4E

 30343 Total active energy      kwh        01 56
 30345 Total reactive energy    kvarh      01 58
*/
/*

  float lineToNeutralVolts;
  float current;
  float activePower;
  float apparentPower;
  float reactivePower;
  float powerFactor;
  float phaseAngle;

  float frequency;
  float importActiveEnergy;
  float exportActiveEnergy;
  float importReactiveEnergy;
  float exportReactiveEnergy;
  float totalActiveEnergy;
  float totalReactiveEnergy;

*/

SDM220InputRegisterPower::SDM220InputRegisterPower(){
  lineToNeutralVolts = -1.0;
  current = -1.0;
  activePower = -1.0;
  apparentPower = -1.0;
  reactivePower = -1.0;
  powerFactor = -1.0;
  phaseAngle = -1.0;
}


SDM220InputRegisterPower::SDM220InputRegisterPower(const ModbusMessage &response)
{
  response.get(3, lineToNeutralVolts);
  response.get(15, current);
  response.get(27, activePower);
  response.get(39, apparentPower);
  response.get(51, reactivePower);
  response.get(63, powerFactor);
  response.get(75, phaseAngle);
}


String SDM220InputRegisterPower::toJson() const
{
  JsonDocument doc;

  doc[PSTR("lineVolts")] = lineToNeutralVolts;
  doc[PSTR("current")] = current;
  doc[PSTR("activePower")] = activePower;
  doc[PSTR("apparentPower")] = apparentPower;
  doc[PSTR("reactivePower")] = reactivePower;
  doc[PSTR("powerFactor")] = powerFactor;
  doc[PSTR("phaseAngle")] = phaseAngle;
  String output;
  serializeJson(doc, output);
  return output;
}

/* Проверяются только НЕЗАВИСИМЫЕ величины.

   apparentPower, reactivePower и phaseAngle счётчик вычисляет из
   activePower и powerFactor - это подтвердилось на реальных данных:
   sqrt(P^2+Q^2) совпало с S до точности float, P/S - с PF до шестого
   знака, а angle оказался равен 360 - acos(PF). Включать их в детектор
   изменений значит трижды переспрашивать одно и то же, причём в более
   шумном виде: относительный разброс реактивной мощности на наблюдаемой
   нагрузке был 1.4% против 0.9% у активной, так что публикациями
   управлял бы именно её шум.

   В JSON они по-прежнему уходят - речь только о детекторе. */
bool SDM220InputRegisterPower::isChanged(const SDM220InputRegisterPower &value) const
{
  return isSignificantChange(lineToNeutralVolts, value.lineToNeutralVolts, deadbandVolts, deadbandRelPercentFixed)
         || isSignificantChange(current, value.current, deadbandAmps, deadbandRelPercent)
         || isSignificantChange(activePower, value.activePower, deadbandWatts, deadbandRelPercent)
         || isSignificantChange(powerFactor, value.powerFactor, deadbandPowerFactor, deadbandRelPercentFixed);
}
