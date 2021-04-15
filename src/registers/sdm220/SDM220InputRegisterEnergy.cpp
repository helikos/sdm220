
#include "ModbusMessage.h"
#include "SDM220InputRegisterEnergy.h"
#include <ArduinoJson.h>


/*
 30071 Frequency                Hz         00 46
 30073 Import active energy     kwh        00 48 
 30075 Export active energy     kwh        00 4A
 30077 Import reactive energy   kvarh      00 4C
 30079 Export reactive energy   kvarh      00 4E 
 */
/*
  float frequency;
  float importActiveEnergy;
  float exportActiveEnergy;
  float importReactiveEnergy;
  float exportReactiveEnergy;    
*/

   SDM220InputRegisterEnergy::SDM220InputRegisterEnergy(ModbusMessage response) {
    response.get(3, frequency);
    response.get(7, importActiveEnergy);
    response.get(11, exportActiveEnergy);
    response.get(15, importReactiveEnergy);
    response.get(19, exportReactiveEnergy);
  }

  const char *SDM220InputRegisterEnergy::toString() {
    Serial.printf("\n !!!!!! ENERGY !!!!!!!!!!!!!\n");
    Serial.printf("Frequency              %f Hz\n", frequency);
    Serial.printf("Import active energy   %f kwh\n", importActiveEnergy);
    Serial.printf("Export active energy   %f kwh\n", exportActiveEnergy);
    Serial.printf("Import reactive energy %f kvarh\n", importReactiveEnergy);
    Serial.printf("Export reactive energy %f kvarh\n", exportReactiveEnergy);

    return "";
  }

  String SDM220InputRegisterEnergy::toJson() {
    DynamicJsonDocument doc(1024);

    doc["frequency"] = frequency;
    doc["importActiveEnergy"] = importActiveEnergy;
    doc["exportActiveEnergy"] = exportActiveEnergy;
    doc["importReactiveEnergy"] = importReactiveEnergy;
    doc["exportReactiveEnergy"] = exportReactiveEnergy;
    String output;
    serializeJson(doc, output);
    return output;
  }
