
#include "SDM220InputRegisterPower.h"
#include <ArduinoJson.h>

//#include "ModbusMessage.h"


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

   SDM220InputRegisterPower::SDM220InputRegisterPower(ModbusMessage response) {
    response.get(3,  lineToNeutralVolts);
    response.get(15, current);
    response.get(27, activePower);
    response.get(39, apparentPower);
    response.get(51, reactivePower);
    response.get(63, powerFactor);
    response.get(75, phaseAngle);
  }

  const char *SDM220InputRegisterPower::toString() {
    Serial.printf("\n !!!!!! POWER !!!!!!!!!!!!!\n");
    Serial.printf("Line to neutral volts %f Volts\n", lineToNeutralVolts);
    Serial.printf("Current               %f Amps\n", current);
    Serial.printf("Active power.         %f Watts\n", activePower);
    Serial.printf("Apparent power        %f VoltAmps\n", apparentPower);
    Serial.printf("Reactive power        %f VAr\n", reactivePower);
    Serial.printf("Power factor          %f None\n", powerFactor);
    Serial.printf("Phase angle.          %f Degree\n", phaseAngle);
    return "";
  }

  String SDM220InputRegisterPower::toJson() {
    DynamicJsonDocument doc(1024);

    doc["lineVolts"] = lineToNeutralVolts;
    doc["current"] = current;
    doc["activePower"] = activePower;
    doc["reactivePower"] = reactivePower;
    doc["powerFactor"] = powerFactor;
    doc["phaseAngle"] = phaseAngle;
    String output;
    serializeJson(doc, output);
    return output;
  }
