#include <Arduino.h>
#include <WiFi.h>
extern "C" {
  #include "freertos/FreeRTOS.h"
  #include "freertos/timers.h"
}
#include <AsyncMqttClient.h>
#include <ArduinoJson.h>
#include "configuration.h"
#include "deviceConfig.h"
#include "security.h"

#include "utils/MqttContext.h"
#include "utils/WiFiContext.h"
#include "utils/Logger.h"
#include "utils/Log.h"
#include "utils/commonFunctions.h"

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>

/* Какой счётчик собирается, решает окружение в platformio.ini.
   Топики, IP и скорость порта лежат в deviceConfig, поэтому здесь
   остался единственный #ifdef - на тип класса. */
#ifdef DEVICE_SDM220
#  include "processing/SDM220Processing.h"
typedef SDM220Processing MeterProcessing;
#else
#  include "processing/PzemProcessing.h"
typedef PzemProcessing MeterProcessing;
#endif

/* Порядок объявления = порядок конструирования: mqttContext готов раньше,
   чем на него берут ссылку остальные. Зависимости передаются по ссылке,
   а не копией класса-без-полей, как было раньше. */
MqttContext mqttContext;
WiFiContext wifiContext(mqttContext);
MeterProcessing meterProcessing(mqttContext, deviceConfig);

AsyncWebServer server(80);

/* Стабилен для платы и уникален между платами. Используется как MQTT
   client id (без него AsyncMqttClient генерирует свой, и две платы с
   одинаковым ID выбивают друг друга из брокера) и как SSID спасательной
   точки доступа. */
static char deviceId[32];


void setup() {
  Serial.begin(115200);
  Serial.println("__ OK __");
  Serial.printf("[APP] device: %s\n", deviceConfig.name);
  Serial.println("[APP] Free memory: " + String(esp_get_free_heap_size()) + " bytes");
  Logger::logInitialization();

  snprintf(deviceId, sizeof(deviceId), "%s-%06X", deviceConfig.name
          ,(unsigned)((ESP.getEfuseMac() >> 24) & 0xFFFFFF));
  Serial.printf("[APP] device id: %s\n", deviceId);

  const MqttSettings mqttSettings = {
      /* server            */ _MQTTSERVER,
      /* port              */ _MQTTPORT,
      /* user              */ _MQTTUSER,
      /* pass              */ _MQTTPASS,
      /* clientId          */ deviceId,
      /* availabilityTopic */ deviceConfig.topicAvailability,
      /* keepAliveSeconds  */ mqttKeepAliveSeconds,
  };
  mqttContext.initializate(mqttSettings);

  const WiFiSettings wifiSettings = {
      /* ssid          */ _SSID,
      /* pass          */ _PASS,
      /* ip            */ IPAddress(networkPrefix1, networkPrefix2, networkPrefix3, deviceConfig.ipLastOctet),
      /* gateway       */ IPAddress(networkPrefix1, networkPrefix2, networkPrefix3, networkGateway4),
      /* subnet        */ IPAddress(networkSubnet),
      /* dns           */ IPAddress(networkPrefix1, networkPrefix2, networkPrefix3, networkDns4),
      /* rescueApSsid  */ deviceId,
      /* rescueApPass  */ wifiRescueApPass,
  };
  wifiContext.initializate(wifiSettings);

  meterProcessing.initializate();

  server.on("/log", HTTP_GET, Logger::publishLog);
  server.on("/memory", HTTP_GET, getFreeHeapSize);

  AsyncElegantOTA.begin(&server);    // Start ElegantOTA
  server.begin();

  Serial.println("HTTP server started");
  Serial.println("[APP] Free memory: " + String(esp_get_free_heap_size()) + " bytes");
}

void loop(void) {
  // Таск loop() больше не крутится вхолостую на приоритете 1.

  // Сначала сеть: ведёт лестницу статический IP -> DHCP -> точка доступа.
  wifiContext.supervise();

  // Перезагрузка по таймауту MQTT - последнее средство, и она несовместима
  // со спасательной точкой доступа: ребут выбил бы единственный путь
  // к /update. Порядок вызовов здесь важен.
  if (!wifiContext.isRescueApActive()) {
    mqttContext.supervise();
  }

  delay(1000);
}
