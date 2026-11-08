#pragma once
#ifndef _IPUBLISHER_H
#define _IPUBLISHER_H

#include <stdint.h>

/* Processing-слой знает только этот интерфейс, а не конкретный MQTT-клиент.
   Раньше MqttContext передавался по значению в WiFiContext и
   ModbusMqqtProcessing, причём статические члены-копии нигде не были
   определены - линковалось только потому, что у класса не было полей. */
class IPublisher
{
public:
    virtual ~IPublisher() {}

    /* true = сообщение принято клиентом. false = транспорт недоступен,
       данные НЕ отправлены; вызывающий не фиксирует состояние
       "опубликовано" и повторит попытку на следующем опросе. */
    virtual bool publish(const char *topic, const char *payload) = 0;

    /* Растёт на каждом подключении: смена epoch означает, что брокер нужно
       наполнить заново. */
    virtual uint32_t connectEpoch() const = 0;
};

#endif // _IPUBLISHER_H
