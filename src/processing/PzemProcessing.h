#pragma once
#ifndef _PZEM_PROCESSING_H
#define _PZEM_PROCESSING_H

#include "ModbusMessage.h"
#include "ModbusClientRTU.h"
#include "configuration.h"
#include "deviceConfig.h"
#include "utils/IPublisher.h"
#include "ModbusMqqtProcessing.h"
#include "PublishGate.h"
#include "registers/pzem/pzemInputRegisters.h"

class PzemProcessing : public ModbusMqqtProcessing
{

public:
    PzemProcessing(IPublisher &publisher, const DeviceConfig &config);

    void initializate();

private:
    TimerHandle_t _timer;
    PublishGate _gate;
    pzemInputRegister _lastPublishedValue;

    void onData(ModbusMessage response, uint32_t token);
    void onError(Error error, uint32_t token);

    static void pollTimerTrampoline(TimerHandle_t timer);
    void onPollTimer();

    void restartTimer();
};

#endif
