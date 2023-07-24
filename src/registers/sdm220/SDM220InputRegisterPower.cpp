
#include "SDM220InputRegisterPower.h"
#include <ArduinoJson.h>
#include "utils/Logger.h"

extern Logger logger;

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


SDM220InputRegisterPower::SDM220InputRegisterPower(ModbusMessage response)
{
  response.get(3, lineToNeutralVolts);
  response.get(15, current);
  response.get(27, activePower);
  response.get(39, apparentPower);
  response.get(51, reactivePower);
  response.get(63, powerFactor);
  response.get(75, phaseAngle);
}

const char *SDM220InputRegisterPower::toString()
{
  logger.log(PSTR("\n !!!!!! POWER !!!!!!!!!!!!!"));
  logger.log(PSTR("Line to neutral volts %f Volts"), lineToNeutralVolts);
  logger.log(PSTR("Current               %f Amps"), current);
  logger.log(PSTR("Active power.         %f Watts"), activePower);
  logger.log(PSTR("Apparent power        %f VoltAmps"), apparentPower);
  logger.log(PSTR("Reactive power        %f VAr"), reactivePower);
  logger.log(PSTR("Power factor          %f None"), powerFactor);
  logger.log(PSTR("Phase angle.          %f Degree"), phaseAngle);
  return "";
}

String SDM220InputRegisterPower::toJson()
{
  DynamicJsonDocument doc(1024);

  doc[PSTR("lineVolts")] = lineToNeutralVolts;
  doc[PSTR("current")] = current;
  doc[PSTR("activePower")] = activePower;
  doc[PSTR("reactivePower")] = reactivePower;
  doc[PSTR("powerFactor")] = powerFactor;
  doc[PSTR("phaseAngle")] = phaseAngle;
  String output;
  serializeJson(doc, output);
  return output;
}

bool SDM220InputRegisterPower::isChanged(SDM220InputRegisterPower value)
{
  return (abs(value.lineToNeutralVolts - lineToNeutralVolts) / value.lineToNeutralVolts) * 100 > 5
         || (abs(value.current - current) / value.current) * 100 > 5
         || (abs(value.activePower - activePower) / value.activePower) * 100 > 5;
}
