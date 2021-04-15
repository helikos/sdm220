#ifndef _PZEM_PROCESSING_H
#define _PZEM_PROCESSING_H

#include "ModbusMessage.h"
#include "ModbusClientRTU.h"
#include "MqttContext.h"
#include "ModbusMqqtProcessing.h"
#include "registers/pzem/pzemInputRegisters.h"

class PzemProcessing : public ModbusMqqtProcessing{

    public:

        void initializate(MqttContext mqttContext
                         ,const char* mqqtTopic
                         ,const char* mqqtTopicError);
   
    protected:

        static const char* _mqqtTopic;
        static TimerHandle_t _timer;
        static pzemInputRegister previousValue;

        static void request();
        static void requestOfMessage();

        static void processingOfMessage(ModbusMessage response);

        static void handleData(ModbusMessage response, uint32_t token); 
        static void handleError(Error error, uint32_t token);

};

#endif
