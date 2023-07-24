
#include "ModbusMessage.h"
#include "SDM220InputRegisterEnergy.h"
#include <ArduinoJson.h>
#include "utils/Logger.h"


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

extern Logger logger;

SDM220InputRegisterEnergy::SDM220InputRegisterEnergy() {
  frequency = -1.0;
  importActiveEnergy = -1.0;
  exportActiveEnergy = -1.0;
  importReactiveEnergy = -1.0;
  exportReactiveEnergy = -1.0;
}


SDM220InputRegisterEnergy::SDM220InputRegisterEnergy(ModbusMessage response) {
  response.get(3, frequency);
  response.get(7, importActiveEnergy);
  response.get(11, exportActiveEnergy);
  response.get(15, importReactiveEnergy);
  response.get(19, exportReactiveEnergy);
}

const char *SDM220InputRegisterEnergy::toString() {
  logger.log(PSTR("\n !!!!!! ENERGY !!!!!!!!!!!!!\n"));
  logger.log(PSTR("Frequency              %f Hz"), frequency);
  logger.log(PSTR("Import active energy   %f kwh"), importActiveEnergy);
  logger.log(PSTR("Export active energy   %f kwh"), exportActiveEnergy);
  logger.log(PSTR("Import reactive energy %f kvarh"), importReactiveEnergy);
  logger.log(PSTR("Export reactive energy %f kvarh"), exportReactiveEnergy);

  return "";
}

String SDM220InputRegisterEnergy::toJson() {
  DynamicJsonDocument doc(1024);

  doc[PSTR("frequency")] = frequency;
  doc[PSTR("importActiveEnergy")] = importActiveEnergy;
  doc[PSTR("exportActiveEnergy")] = exportActiveEnergy;
  doc[PSTR("importReactiveEnergy")] = importReactiveEnergy;
  doc[PSTR("exportReactiveEnergy")] = exportReactiveEnergy;
  String output;
  serializeJson(doc, output);
  return output;
}

bool SDM220InputRegisterEnergy::isChanged(SDM220InputRegisterEnergy value)
{
  return (abs(value.importActiveEnergy - importActiveEnergy) / value.importActiveEnergy) * 100 > 5
         || (abs(value.importReactiveEnergy - importReactiveEnergy) / value.importReactiveEnergy) * 100 > 5;
}
