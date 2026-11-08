#ifndef _SDM220_INPUT_REGISTER_TOTAL_H
#define _SDM220_INPUT_REGISTER_TOTAL_H

#include <Arduino.h>
#include "ModbusMessage.h"

/*
 30343 Total active energy      kwh        01 56 
 30345 Total reactive energy    kvarh      01 58
*/

class SDM220InputRegisterTotal {
 public:
  float totalActiveEnergy;       
  float totalReactiveEnergy;    


  explicit SDM220InputRegisterTotal();
  explicit SDM220InputRegisterTotal(const ModbusMessage &response);
  bool isChanged(const SDM220InputRegisterTotal &value) const;

  String toJson() const;

};

#endif // _SDM220_INPUT_REGISTER_THIRD_H
