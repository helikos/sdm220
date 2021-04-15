#ifndef _SDM220_INPUT_REGISTER_ENERGY_H
#define _SDM220_INPUT_REGISTER_ENERGY_H

#include "../inputRegisterParameters.h"
#include "ModbusMessage.h"

/*
 30071 Frequency                Hz         00 46
 30073 Import active energy     kwh        00 48 
 30075 Export active energy     kwh        00 4A
 30077 Import reactive energy   kvarh      00 4C
 30079 Export reactive energy   kvarh      00 4E 
 */

class SDM220InputRegisterEnergy : public InputRegisterParameters {
 public:
  float frequency;
  float importActiveEnergy;
  float exportActiveEnergy;
  float importReactiveEnergy;
  float exportReactiveEnergy;    


  explicit SDM220InputRegisterEnergy(ModbusMessage response);

  const char *toString();
  String toJson();

};

#endif // _SDM220_INPUT_REGISTER_SECOND_H
