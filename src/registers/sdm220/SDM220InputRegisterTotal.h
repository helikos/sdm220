#ifndef _SDM220_INPUT_REGISTER_TOTAL_H
#define _SDM220_INPUT_REGISTER_TOTAL_H

#include "ModbusMessage.h"
#include "../inputRegisterParameters.h"

/*
 30343 Total active energy      kwh        01 56 
 30345 Total reactive energy    kvarh      01 58
*/

class SDM220InputRegisterTotal : public InputRegisterParameters {
 public:
  float totalActiveEnergy;       
  float totalReactiveEnergy;    


  explicit SDM220InputRegisterTotal();
  explicit SDM220InputRegisterTotal(ModbusMessage response);
  bool isChanged(SDM220InputRegisterTotal value);

  const char *toString();
  String toJson();

};

#endif // _SDM220_INPUT_REGISTER_THIRD_H
