#include <Arduino.h>
#include <WiFi.h>
extern "C" {
  #include "freertos/FreeRTOS.h"
  #include "freertos/timers.h"
}
#include <AsyncMqttClient.h>
#include <ArduinoJson.h>
#include "configuration.h"
#include "security.h"


// Include the header for the ModbusClient RTU style
#include "utils/MqttContext.h"
#include "utils/wifiContext.h"
#include "utils/Logger.h"
#include "utils/commonFunctions.h"

#include "processing/SDM220Processing.h"
#include "processing/PzemProcessing.h"

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>

#ifdef sdm220
const char* mqqtTopicPower  = "powermetter/sdm200/power";
const char* mqqtTopicEnergy = "powermetter/sdm200/energy";
const char* mqqtTopicTotal  = "powermetter/sdm200/total";
const char* mqqtTopicError  = "powermetter/sdm200/error";
SDM220Processing sdm220Processing;
#endif


#ifdef pzem
const char* mqqtTopicPower = "powermetter/pzem/power";
const char* mqqtTopicError = "powermetter/pzem/error";
PzemProcessing pzemProcessing;
#endif


WiFiContext wifiContext;
MqttContext mqttContext;
Logger logger;

AsyncWebServer server(80);


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);  
  while (!Serial) {}
  Serial.println("__ OK __");
  Serial.println("[APP] Free memory: " + String(esp_get_free_heap_size()) + " bytes");
  logger.logInitialization();
  Serial.println("[APP] Free memory: " + String(esp_get_free_heap_size()) + " bytes");

  mqttContext.initializate(_MQTTSERVER, _MQTTPORT, _MQTTUSER, _MQTTPASS);
  wifiContext.initializate(mqttContext, _SSID, _PASS);

#ifdef sdm220
  sdm220Processing.initializate(mqttContext, mqqtTopicPower, mqqtTopicEnergy, mqqtTopicTotal, mqqtTopicError);
#endif

#ifdef pzem
  pzemProcessing.initializate(mqttContext, mqqtTopicPower, mqqtTopicError);
#endif

  server.on("/log", HTTP_GET, Logger::publishLog);
  server.on("/memory", HTTP_GET, getFreeHeapSize);

  AsyncElegantOTA.begin(&server);    // Start ElegantOTA
  server.begin();

  Serial.println("HTTP server started");

}

void loop(void) {
}
