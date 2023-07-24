#ifndef _INPUT_REGISTER_PARAMETERS_H
#define _INPUT_REGISTER_PARAMETERS_H

#include "ModbusClientRTU.h"
#include "ModbusMessage.h"

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

class InputRegisterParameters
{
public:
  virtual const char *toString();
  virtual String toJson();
};

#endif // _SDM220_INPUT_REGISTER_PARAMETERS_H
