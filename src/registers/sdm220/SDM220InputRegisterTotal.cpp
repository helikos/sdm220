
#include "ModbusMessage.h"
#include "SDM220InputRegisterTotal.h"
#include <ArduinoJson.h>
#include "utils/Logger.h"

extern Logger logger;

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

SDM220InputRegisterTotal::SDM220InputRegisterTotal(ModbusMessage response)
{
  response.get(3, totalActiveEnergy);
  response.get(7, totalReactiveEnergy);
}

const char *SDM220InputRegisterTotal::toString()
{
  logger.log(PSTR("\n !!!!!! TOTAL !!!!!!!!!!!!!"));
  logger.log(PSTR("Total active energy   %f kwh"), totalActiveEnergy);
  logger.log(PSTR("Total reactive energy %f kvarh"), totalReactiveEnergy);
  return "";
}

String SDM220InputRegisterTotal::toJson()
{
  DynamicJsonDocument doc(1024);

  doc[PSTR("totalActiveEnergy")] = totalActiveEnergy;
  doc[PSTR("totalReactiveEnergy")] = totalReactiveEnergy;
  String output;
  serializeJson(doc, output);
  return output;
}

bool SDM220InputRegisterTotal::isChanged(SDM220InputRegisterTotal value)
{
  return (abs(value.totalActiveEnergy - totalActiveEnergy) / value.totalActiveEnergy) * 100 > 5
         || (abs(value.totalReactiveEnergy - totalReactiveEnergy) / value.totalReactiveEnergy) * 100 > 5;
}
