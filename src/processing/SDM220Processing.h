#pragma once

#ifndef _SDM220_PROCESSING_H
#define _SDM220_PROCESSING_H

#include "ModbusMessage.h"
#include "ModbusClientRTU.h"
#include "SDM220TypeDefs.h"
#include "configuration.h"
#include "deviceConfig.h"
#include "utils/IPublisher.h"
#include "utils/Log.h"
#include "ModbusMqqtProcessing.h"
#include "PublishGate.h"
#include "registers/sdm220/SDM220InputRegisterEnergy.h"
#include "registers/sdm220/SDM220InputRegisterPower.h"
#include "registers/sdm220/SDM220InputRegisterTotal.h"


class SDM220Processing : public ModbusMqqtProcessing
{

public:
    SDM220Processing(IPublisher &publisher, const DeviceConfig &config);

    void initializate();

private:
    /* Три набора регистров отличаются только таблицей параметров, поэтому
       живут в массиве, а не в трёх почти одинаковых блоках switch по 20 строк.
       Добавить новую группу регистров = добавить строку в _channels. */
    struct Channel
    {
        SDM220Processing *owner;
        RegistersClass registers;
        const char *topic;
        uint32_t startRegister;
        uint32_t amount;
        TimerHandle_t timer;
        PublishGate gate;
    };

    Channel _channels[3];

    SDM220InputRegisterPower _lastPublishedPower;
    SDM220InputRegisterEnergy _lastPublishedEnergy;
    SDM220InputRegisterTotal _lastPublishedTotal;

    Channel *channelFor(RegistersClass registers);

    void onData(ModbusMessage response, uint32_t token);
    void onError(Error error, uint32_t token);

    /* Один колбэк на все таймеры: какие регистры опрашивать, лежит
       в timer ID. Раньше три void()-функции приводились
       reinterpret_cast'ом к TimerCallbackFunction_t. */
    static void pollTimerTrampoline(TimerHandle_t timer);
    void onPollTimer(Channel &channel);

    // Сравниваем с последним ОПУБЛИКОВАННЫМ значением, а не с последним
    // прочитанным: иначе медленный дрейф в пределах мёртвой зоны
    // не публикуется никогда.
    template <typename TRegister>
    void publishIfNeeded(Channel &channel, TRegister &lastPublished, const ModbusMessage &response)
    {
        TRegister current(response);
        const String payload = current.toJson();
        LOG_PROC(PSTR("%s"), payload.c_str());

        const uint32_t epoch = _publisher.connectEpoch();
        if (channel.gate.needsForcedPublish(epoch) || current.isChanged(lastPublished))
        {
            // Состояние фиксируем только после успешной отправки: при
            // недоступном брокере следующий опрос повторит попытку.
            if (_publisher.publish(channel.topic, payload.c_str()))
            {
                lastPublished = current;
                channel.gate.onPublished(epoch);
            }
        }
    }
};

#endif
