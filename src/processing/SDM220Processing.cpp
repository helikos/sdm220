
#include <Arduino.h>

#include "SDM220Processing.h"
#include "SDM220TypeDefs.h"
#include "configuration.h"
#include "utils/Log.h"

extern "C"
{
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
}

#define timeOfPower pdMS_TO_TICKS(1000)    // 1 sec
#define timeOfEnergy pdMS_TO_TICKS(120000) // 2 min
#define timeOfTotal pdMS_TO_TICKS(180000)  // 3 minute

SDM220Processing::SDM220Processing(IPublisher &publisher, const DeviceConfig &config)
    : ModbusMqqtProcessing(publisher, config)
    , _channels{
          {this, thePower,  config.topicPower,  0,   42, NULL, PublishGate(heartbeatMsPower)},
          {this, theEnergy, config.topicEnergy, 70,  12, NULL, PublishGate(heartbeatMsEnergy)},
          {this, theTotal,  config.topicTotal,  342, 8,  NULL, PublishGate(heartbeatMsTotal)},
      }
{
}

void SDM220Processing::initializate()
{
    LOG_INIT(PSTR("SDM220Processing::initializate started"));

    initializateModbus([this](ModbusMessage response, uint32_t token) { onData(response, token); }
                      ,[this](Error error, uint32_t token) { onError(error, token); });

    static const char *const timerNames[3] = {"getPower", "getEnergy", "getTotal"};
    static const TickType_t timerPeriods[3] = {timeOfPower, timeOfEnergy, timeOfTotal};

    for (uint8_t i = 0; i < 3; i++)
    {
        _channels[i].timer = xTimerCreate(timerNames[i], timerPeriods[i], pdFALSE
                                         ,&_channels[i], pollTimerTrampoline);
        xTimerStart(_channels[i].timer, 0);
    }

    LOG_INIT(PSTR("SDM220Processing::initializate completed"));
}

SDM220Processing::Channel *SDM220Processing::channelFor(RegistersClass registers)
{
    for (uint8_t i = 0; i < 3; i++)
    {
        if (_channels[i].registers == registers)
            return &_channels[i];
    }
    return NULL;
}

void SDM220Processing::pollTimerTrampoline(TimerHandle_t timer)
{
    // В timer ID лежит канал, а владельца канал хранит сам: так из
    // C-колбэка FreeRTOS попадаем в метод нужного экземпляра.
    Channel *channel = static_cast<Channel *>(pvTimerGetTimerID(timer));
    channel->owner->onPollTimer(*channel);
}

void SDM220Processing::onPollTimer(Channel &channel)
{
    LOG_MODBUS(PSTR("SDM220Processing::onPollTimer %08X"), (unsigned)channel.registers);

    if (request(channel.registers, channel.startRegister, channel.amount))
        return;

    // Запрос не попал в очередь eModbus, значит ни onData, ни onError
    // не вызовутся и перезапустить таймер больше некому: без этого цепочка
    // опроса умирала до перезагрузки.
    LOG(PSTR("SDM220Processing::onPollTimer request rejected, retry scheduled %08X"), (unsigned)channel.registers);
    xTimerStart(channel.timer, 0);
}

void SDM220Processing::onData(ModbusMessage response, uint32_t token)
{
    logResponse(response, token);

    Channel *channel = channelFor((RegistersClass)token);
    if (channel == NULL)
    {
        LOG(PSTR("SDM220Processing::onData unknown token %08X"), (unsigned)token);
        return;
    }

    switch (channel->registers)
    {
    case thePower:
        publishIfNeeded(*channel, _lastPublishedPower, response);
        break;
    case theEnergy:
        publishIfNeeded(*channel, _lastPublishedEnergy, response);
        break;
    case theTotal:
        publishIfNeeded(*channel, _lastPublishedTotal, response);
        break;
    }

    xTimerStart(channel->timer, 0);
}

void SDM220Processing::onError(Error error, uint32_t token)
{
    publishError(error, token);

    Channel *channel = channelFor((RegistersClass)token);
    if (channel == NULL)
    {
        LOG(PSTR("SDM220Processing::onError unknown token %08X"), (unsigned)token);
        return;
    }
    xTimerStart(channel->timer, 0);
}
