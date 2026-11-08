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

/* Какое устройство собирается, задаёт окружение в platformio.ini
   (-D DEVICE_SDM220 / -D DEVICE_PZEM), а не #define здесь.
   Параметры устройства - в deviceConfig.h/.cpp. */

/* ─── Сеть (общая часть, последний октет - в deviceConfig) ──────────────── */
#define networkPrefix1 192
#define networkPrefix2 168
#define networkPrefix3 1
#define networkGateway4 1
#define networkDns4 1
#define networkSubnet 255, 255, 255, 0

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


/* ─── Фоллбэки сети ───────────────────────────────────────────────────────
   Лестница в пределах одного сеанса: статический IP -> DHCP -> спасательная
   точка доступа. Выбранный режим НЕ сохраняется между перезагрузками -
   для этого нужен NVS, которого в прошивке пока нет. */

/* Неудачных попыток ассоциации в одном режиме IP, прежде чем переключить
   режим. Покрывает и обратный случай: на роутере выключен DHCP-сервер.
   0 = не переключать. */
#define wifiMaxAttemptsPerMode 5

/* Ассоциация есть, но брокер недостижим дольше этого времени - вероятно,
   статический IP прописан в подсети, которой больше нет. Пробуем DHCP.
   0 = не пробовать. */
#define wifiStaticIpGraceMs 120000UL  // 2 мин

/* Связи нет совсем дольше этого времени - поднимаем точку доступа, чтобы
   можно было зайти на /update и прошить сборку с новыми параметрами.
   Настройки точка не принимает: сохранять их некуда без NVS.
   0 = не поднимать. */
#define wifiRescueApAfterMs 300000UL  // 5 мин

/* Пароль спасательной точки. Короче 8 символов или NULL - сеть открытая.
   Лежит в репозитории осознанно: это аварийный канал в домашней сети.
   Нужно иначе - перенесите в security.h и передайте через WiFiSettings. */
#define wifiRescueApPass "sdm220ota"


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

/* Keep-alive сессии. Брокер объявит клиента мёртвым и опубликует LWT
   примерно через keepAlive * 1.5 после пропажи связи. */
#define mqttKeepAliveSeconds 15

/* Availability-топик: для Home Assistant это разница между "датчик
   недоступен" и "датчик показывает старое retained-значение".
   offline публикует сам брокер по Last Will, online - устройство
   при каждом подключении. */
#define availabilityOnline  "online"
#define availabilityOffline "offline"

/* Перезагрузка, если брокер недоступен дольше этого времени. 0 = выключить. */
#define mqttMaxOfflineMs 600000UL  // 10 мин

#endif
