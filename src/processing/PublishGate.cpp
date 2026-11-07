
#include "PublishGate.h"
#include "utils/MqttContext.h"

PublishGate::PublishGate(uint32_t heartbeatInterval)
    : _heartbeatMs(heartbeatInterval), _lastPublishMs(0), _publishedEpoch(0), _hasPublished(false)
{
}

bool PublishGate::needsForcedPublish() const
{
    if (!_hasPublished)
        return true;

    if (MqttContext::connectEpoch() != _publishedEpoch)
        return true;

    // Вычитание uint32_t корректно переживает переполнение millis() (~49.7 суток).
    if (_heartbeatMs != 0 && (millis() - _lastPublishMs) >= _heartbeatMs)
        return true;

    return false;
}

void PublishGate::onPublished()
{
    _lastPublishMs = millis();
    _publishedEpoch = MqttContext::connectEpoch();
    _hasPublished = true;
}
