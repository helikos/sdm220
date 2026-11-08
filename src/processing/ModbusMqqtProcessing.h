#pragma once

#ifndef _MODBUS_MQTT_PROCESSING_H
#define _MODBUS_MQTT_PROCESSING_H

#include "utils/IPublisher.h"
#include "deviceConfig.h"
#include "ModbusClientRTU.h"

/* Общая часть опроса Modbus и отправки в MQTT.

   Раньше _mb и _mqttContext были статическими членами базы, то есть
   SDM220Processing и PzemProcessing делили ОДИН клиент Modbus и один набор
   топиков: "наследование" здесь шарило глобальное состояние, а не
   специализировало поведение. Теперь у каждого экземпляра свои поля. */
class ModbusMqqtProcessing
{
  protected:
    ModbusMqqtProcessing(IPublisher &publisher, const DeviceConfig &config);
    virtual ~ModbusMqqtProcessing() {}

    IPublisher &_publisher;
    const DeviceConfig &_config;
    ModbusClientRTU _mb;

    void initializateModbus(MBOnData onDataHandler, MBOnError onErrorHandler);

    /* false = запрос не принят клиентом eModbus. Ни onData, ни onError
       в этом случае не придут, поэтому таймер опроса обязан
       перезапустить вызывающий. */
    bool request(uint32_t token, uint32_t start, uint32_t amount);

    void logResponse(ModbusMessage &response, uint32_t token);
    void publishError(Error error, uint32_t token);

    static String errorToJson(Error error);
};

#endif
