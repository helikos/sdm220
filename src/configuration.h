#pragma once
#ifndef _CONFIGURATION_H
#define _CONFIGURATION_H

#include <WiFi.h>


#define debug
#define debug_processing
#define debug_mqtt
#define debug_initialization
// #define modbus_Processing
#define isPrint2Serial

#define sdm220
// #define pzem


// #define configTIMER_QUEUE_LENGTH 20

/* Macros for getting elapsed time */
#define numberOfSeconds(_time_) ((_time_ / 1000) % 60)
#define numberOfMinutes(_time_) (((_time_ / 1000) / 60) % 60)
#define numberOfHours(_time_) (((_time_ / 1000) / 60) / 60)
#define numberOfDays(_time_) ((((_time_ / 1000) / 60) / 60) / 60)


/* ─── Логгер ────────────────────────────────────────────────────────────────
   sizeOfLog * msgLength байт в .bss. msgLength должен вмещать JSON-payload
   целиком, иначе строки лога обрезаются. */
#define sizeOfLog 128
#define msgLength 256


/* ─── Публикация в MQTT ────────────────────────────────────────────────────
   heartbeat: принудительная публикация, даже если показания не изменились.
   0 = выключить (публиковать только по изменению). */
#define heartbeatMs        20000UL
#define heartbeatMsPower   heartbeatMs
#define heartbeatMsEnergy  heartbeatMs
#define heartbeatMsTotal   heartbeatMs
#define heartbeatMsPzem    heartbeatMs

/* Мёртвая зона. Публикуем, если |Δ| >= max(абсолютный порог,
   deadbandRelPercent % от предыдущего опубликованного значения).
   Абсолютный порог глушит шум около нуля и убирает деление на ноль. */
#define deadbandRelPercent  3.0f
#define deadbandVolts       1.0f
#define deadbandAmps        0.02f
#define deadbandWatts       2.0f
#define deadbandVa          2.0f
#define deadbandVar         2.0f
#define deadbandPowerFactor 0.02f
#define deadbandDegrees     1.0f
#define deadbandHz          0.05f
#define deadbandKwh         0.001f
#define deadbandKvarh       0.001f

/* QoS 1 вместо 2: в AsyncMqttClient путь QoS2 наименее обкатан и держит
   состояние на каждый пакет. Для телеметрии с retain хватает QoS 1. */
#define mqttQos    1
#define mqttRetain true

/* Перезагрузка, если брокер недоступен дольше этого времени. 0 = выключить. */
#define mqttMaxOfflineMs 600000UL  // 10 мин

#endif
