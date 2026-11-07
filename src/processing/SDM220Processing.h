#pragma once

#ifndef _SDM220_PROCESSING_H
#define _SDM220_PROCESSING_H

#include "ModbusMessage.h"
#include "ModbusClientRTU.h"
#include "SDM220TypeDefs.h"
#include "utils/MqttContext.h"
#include "ModbusMqqtProcessing.h"
#include "PublishGate.h"
#include "registers/sdm220/SDM220InputRegisterEnergy.h"
#include "registers/sdm220/SDM220InputRegisterPower.h"
#include "registers/sdm220/SDM220InputRegisterTotal.h"


class SDM220Processing : public ModbusMqqtProcessing
{

public:
    static void initializate(MqttContext mqttContext, const char *mqqtTopicPower, const char *mqqtTopicEnergy, const char *mqqtTopicTotal, const char *mqqtTopicError);

protected:
    static const char *_mqqtTopicPower;
    static const char *_mqqtTopicEnergy;
    static const char *_mqqtTopicTotal;

    static TimerHandle_t _timerPower;
    static TimerHandle_t _timerEnergy;
    static TimerHandle_t _timerTotal;

    static void processingOfMessage(ModbusMessage response, RegistersClass registers);
    static bool requestOfMessage(RegistersClass registers);
    static void handleData(ModbusMessage response, uint32_t token);
    static void handleError(Error error, uint32_t token);

    /* Один колбэк на все три таймера: какие регистры опрашивать, лежит
       в timer ID. Раньше три void()-функции приводились
       reinterpret_cast'ом к TimerCallbackFunction_t. */
    static void onPollTimer(TimerHandle_t timer);
    static TimerHandle_t timerFor(RegistersClass registers);
    static void requestOrRetry(RegistersClass registers);

private:
    // Сравниваем с последним ОПУБЛИКОВАННЫМ значением, а не с последним
    // прочитанным: иначе медленный дрейф в пределах мёртвой зоны
    // не публикуется никогда.
    static SDM220InputRegisterEnergy _lastPublishedEnergy;
    static SDM220InputRegisterPower _lastPublishedPower;
    static SDM220InputRegisterTotal _lastPublishedTotal;

    static PublishGate _powerGate;
    static PublishGate _energyGate;
    static PublishGate _totalGate;
};

#endif
