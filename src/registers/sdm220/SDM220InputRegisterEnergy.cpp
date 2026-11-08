
#include "ModbusMessage.h"
#include "SDM220InputRegisterEnergy.h"
#include <ArduinoJson.h>
#include "configuration.h"
#include "utils/commonFunctions.h"


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


SDM220InputRegisterEnergy::SDM220InputRegisterEnergy() {
  frequency = -1.0;
  importActiveEnergy = -1.0;
  exportActiveEnergy = -1.0;
  importReactiveEnergy = -1.0;
  exportReactiveEnergy = -1.0;
}


SDM220InputRegisterEnergy::SDM220InputRegisterEnergy(const ModbusMessage &response) {
  response.get(3, frequency);
  response.get(7, importActiveEnergy);
  response.get(11, exportActiveEnergy);
  response.get(15, importReactiveEnergy);
  response.get(19, exportReactiveEnergy);
}


String SDM220InputRegisterEnergy::toJson() const {
  JsonDocument doc;

  doc[PSTR("frequency")] = frequency;
  doc[PSTR("importActiveEnergy")] = importActiveEnergy;
  doc[PSTR("exportActiveEnergy")] = exportActiveEnergy;
  doc[PSTR("importReactiveEnergy")] = importReactiveEnergy;
  doc[PSTR("exportReactiveEnergy")] = exportReactiveEnergy;
  String output;
  serializeJson(doc, output);
  return output;
}

bool SDM220InputRegisterEnergy::isChanged(const SDM220InputRegisterEnergy &value) const
{
  return isSignificantChange(frequency, value.frequency, deadbandHz, deadbandRelPercent)
         || isSignificantChange(importActiveEnergy, value.importActiveEnergy, deadbandKwh, deadbandRelPercent)
         || isSignificantChange(exportActiveEnergy, value.exportActiveEnergy, deadbandKwh, deadbandRelPercent)
         || isSignificantChange(importReactiveEnergy, value.importReactiveEnergy, deadbandKvarh, deadbandRelPercent)
         || isSignificantChange(exportReactiveEnergy, value.exportReactiveEnergy, deadbandKvarh, deadbandRelPercent);
}
