
#include "ModbusMessage.h"
#include "SDM220InputRegisterTotal.h"
#include <ArduinoJson.h>
#include "configuration.h"
#include "utils/commonFunctions.h"


/*
 30343 Total active energy      kwh        01 56
 30345 Total reactive energy    kvarh      01 58
*/
/*
  float totalActiveEnergy;
  float totalReactiveEnergy;

*/

SDM220InputRegisterTotal::SDM220InputRegisterTotal()
{
  totalActiveEnergy = -1.0;       
  totalReactiveEnergy = -1.0;    
}

SDM220InputRegisterTotal::SDM220InputRegisterTotal(const ModbusMessage &response)
{
  response.get(3, totalActiveEnergy);
  response.get(7, totalReactiveEnergy);
}


String SDM220InputRegisterTotal::toJson() const
{
  JsonDocument doc;

  doc[PSTR("totalActiveEnergy")] = totalActiveEnergy;
  doc[PSTR("totalReactiveEnergy")] = totalReactiveEnergy;
  String output;
  serializeJson(doc, output);
  return output;
}

// Накопительные счётчики: только абсолютный порог, см. комментарий
// к deadbandRelPercentFixed в configuration.h.
bool SDM220InputRegisterTotal::isChanged(const SDM220InputRegisterTotal &value) const
{
  return isSignificantChange(totalActiveEnergy, value.totalActiveEnergy, deadbandKwh, deadbandRelPercentFixed)
         || isSignificantChange(totalReactiveEnergy, value.totalReactiveEnergy, deadbandKvarh, deadbandRelPercentFixed);
}
