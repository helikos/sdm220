
#include "PublishGate.h"

PublishGate::PublishGate(uint32_t heartbeatInterval)
    : _heartbeatMs(heartbeatInterval), _lastPublishMs(0), _publishedEpoch(0), _hasPublished(false)
{
}

bool PublishGate::needsForcedPublish(uint32_t connectEpoch) const
{
    if (!_hasPublished)
        return true;

    if (connectEpoch != _publishedEpoch)
        return true;

    // Вычитание uint32_t корректно переживает переполнение millis() (~49.7 суток).
    if (_heartbeatMs != 0 && (millis() - _lastPublishMs) >= _heartbeatMs)
        return true;

    return false;
}

void PublishGate::onPublished(uint32_t connectEpoch)
{
    _lastPublishMs = millis();
    _publishedEpoch = connectEpoch;
    _hasPublished = true;
}
