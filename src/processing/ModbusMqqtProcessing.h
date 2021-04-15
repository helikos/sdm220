
#ifndef _MODBUS_MQTT_PROCESSING_H
#define _MODBUS_MQTT_PROCESSING_H

#include "MqttContext.h"
#include "ModbusClientRTU.h"

class ModbusMqqtProcessing {

  protected:
    static MqttContext  _mqttContext;
    static ModbusClientRTU _mb;
    static const char* _mqqtTopicError;

    static void initializate(MqttContext mqttContext
                            ,unsigned long baud
                            ,MBOnData onDataHandler
                            ,MBOnError onErrorHandler
                            ,const char* mqqtTopicError);
    static String errorToJson(Error error);
    static void request(uint32_t token, uint32_t start, uint32_t amount);
    static void handleData(ModbusMessage response, uint32_t token);
    static void handleError(Error error, uint32_t token);

};

#endif