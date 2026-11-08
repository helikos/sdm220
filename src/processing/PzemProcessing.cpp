
#include <Arduino.h>

#include "PzemProcessing.h"
#include "configuration.h"
#include "utils/Log.h"

extern "C" {
  #include "freertos/FreeRTOS.h"
  #include "freertos/timers.h"
}

#define timeOfMain pdMS_TO_TICKS(2000)  // 2 sec

#define pzemStartRegister 0
#define pzemRegisterCount 10

PzemProcessing::PzemProcessing(IPublisher &publisher, const DeviceConfig &config)
    : ModbusMqqtProcessing(publisher, config)
    , _timer(NULL)
    , _gate(heartbeatMsPzem)
{
}

void PzemProcessing::initializate() {
  LOG_INIT(PSTR("PzemProcessing::initializate started"));

  initializateModbus([this](ModbusMessage response, uint32_t token) { onData(response, token); }
                    ,[this](Error error, uint32_t token) { onError(error, token); });

  _timer = xTimerCreate("get", timeOfMain, pdFALSE, this, pollTimerTrampoline);
  xTimerStart(_timer, 0);

  LOG_INIT(PSTR("PzemProcessing::initializate completed"));
}

void PzemProcessing::pollTimerTrampoline(TimerHandle_t timer) {
  static_cast<PzemProcessing *>(pvTimerGetTimerID(timer))->onPollTimer();
}

void PzemProcessing::restartTimer() {
  if (_timer != NULL && xTimerIsTimerActive(_timer) == pdFALSE) {
    xTimerStart(_timer, 0);
  }
}

void PzemProcessing::onPollTimer() {
  LOG_PROC(PSTR("PzemProcessing::onPollTimer"));

  if (request(0, pzemStartRegister, pzemRegisterCount))
    return;

  // Запрос отвергнут клиентом eModbus: колбэков не будет, перезапускаем
  // таймер сами, иначе опрос останавливается навсегда.
  LOG(PSTR("PzemProcessing::onPollTimer request rejected, retry scheduled"));
  restartTimer();
}

void PzemProcessing::onData(ModbusMessage response, uint32_t token) {
  logResponse(response, token);

  pzemInputRegister current(response);
  const String payload = current.toJson();
  LOG_PROC(PSTR("%s"), payload.c_str());

  const uint32_t epoch = _publisher.connectEpoch();
  if (_gate.needsForcedPublish(epoch) || current.isChanged(_lastPublishedValue)) {
    // Состояние фиксируем только после успешной отправки.
    if (_publisher.publish(_config.topicPower, payload.c_str())) {
      _lastPublishedValue = current;
      _gate.onPublished(epoch);
    }
  }

  restartTimer();
}

void PzemProcessing::onError(Error error, uint32_t token) {
  publishError(error, token);
  restartTimer();
}
