#pragma once
#ifndef _PUBLISH_GATE_H
#define _PUBLISH_GATE_H

#include <Arduino.h>

/* Решает, пора ли публиковать показания.

   Публикация нужна принудительно, без оглядки на изменение значений, когда:
     - это первая успешная публикация (раньше первый отсчёт просто терялся);
     - с прошлой публикации прошло heartbeatMs (на стабильной нагрузке брокер
       иначе не получал вообще ничего, и retained-значение устаревало);
     - сменилась epoch подключения, т.е. был реконнект и брокер нужно
       наполнить заново.

   В остальных случаях решает мёртвая зона конкретного набора регистров.

   onPublished() вызывать ТОЛЬКО после успешной отправки: при недоступном
   брокере состояние не фиксируется и попытка повторится на следующем опросе.

   Epoch передаётся аргументом, а не берётся из MqttContext: класс остаётся
   значением без зависимостей от транспорта. */
class PublishGate
{
public:
    explicit PublishGate(uint32_t heartbeatInterval);

    bool needsForcedPublish(uint32_t connectEpoch) const;
    void onPublished(uint32_t connectEpoch);

private:
    uint32_t _heartbeatMs;
    uint32_t _lastPublishMs;
    uint32_t _publishedEpoch;
    bool _hasPublished;
};

#endif // _PUBLISH_GATE_H
