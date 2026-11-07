#pragma once
#ifndef _PUBLISH_GATE_H
#define _PUBLISH_GATE_H

#include <Arduino.h>

/* Решает, пора ли публиковать показания.

   Публикация нужна принудительно, без оглядки на изменение значений, когда:
     - это первая успешная публикация (раньше первый отсчёт просто терялся);
     - с прошлой публикации прошло heartbeatMs (на стабильной нагрузке брокер
       иначе не получал вообще ничего, и retained-значение устаревало);
     - сменилась epoch подключения к MQTT, т.е. был реконнект и брокер нужно
       наполнить заново.

   В остальных случаях решает мёртвая зона конкретного набора регистров.

   onPublished() вызывать ТОЛЬКО после успешной отправки: при недоступном
   брокере состояние не фиксируется и попытка повторится на следующем опросе. */
class PublishGate
{
public:
    explicit PublishGate(uint32_t heartbeatInterval);

    bool needsForcedPublish() const;
    void onPublished();

private:
    uint32_t _heartbeatMs;
    uint32_t _lastPublishMs;
    uint32_t _publishedEpoch;
    bool _hasPublished;
};

#endif // _PUBLISH_GATE_H
