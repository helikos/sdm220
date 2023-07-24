#pragma once

#ifndef _SDM220_PROCESSING_H
#define _SDM220_PROCESSING_H

#include "ModbusMessage.h"
#include "ModbusClientRTU.h"
#include "SDM220TypeDefs.h"
#include "utils/MqttContext.h"
#include "ModbusMqqtProcessing.h"


class SDM220Processing : public ModbusMqqtProcessing {

    public:

     static void initializate(MqttContext mqttContext
                             ,const char* mqqtTopicPower
                             ,const char* mqqtTopicEnergy
                             ,const char* mqqtTopicTotal
                             ,const char* mqqtTopicError);
   
    protected:

        static const char* _mqqtTopicPower;
        static const char* _mqqtTopicEnergy;
        static const char* _mqqtTopicTotal;

        static TimerHandle_t _timerPower;
        static TimerHandle_t _timerEnergy;
        static TimerHandle_t _timerTotal;
 
        static void processingOfMessage(ModbusMessage response, RegistersClass registers);
        static void requestOfMessage(RegistersClass registers);
        static void handleData(ModbusMessage response, uint32_t token); 
        static void handleError(Error error, uint32_t token);

        static void requestPower();
        static void requestEnergy();
        static void requestTotal();
  
};

#endif
