
#include "ModbusMessage.h"
#include "SDM220InputRegisterTotal.h"
#include <ArduinoJson.h>


/*
 30343 Total active energy      kwh        01 56 
 30345 Total reactive energy    kvarh      01 58
*/
/*
  float totalActiveEnergy;       
  float totalReactiveEnergy;    

*/

   SDM220InputRegisterTotal::SDM220InputRegisterTotal(ModbusMessage response) {
    response.get(3, totalActiveEnergy);
    response.get(7, totalReactiveEnergy);
  }

  const char *SDM220InputRegisterTotal::toString() {
    Serial.printf("\n !!!!!! TOTAL !!!!!!!!!!!!!\n");
    Serial.printf("Total active energy   %f kwh\n", totalActiveEnergy);
    Serial.printf("Total reactive energy %f kvarh\n", totalReactiveEnergy);

    return "";
  }

  String SDM220InputRegisterTotal::toJson() {
    DynamicJsonDocument doc(1024);

    doc["totalActiveEnergy"] = totalActiveEnergy;
    doc["totalReactiveEnergy"] = totalReactiveEnergy;
    String output;
    serializeJson(doc, output);
    return output;
  }
