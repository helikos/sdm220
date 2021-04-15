#include <Arduino.h>
#include <WiFi.h>
extern "C" {
  #include "freertos/FreeRTOS.h"
  #include "freertos/timers.h"
}
#include <AsyncMqttClient.h>
#include <ArduinoJson.h>

// Include the header for the ModbusClient RTU style
#include "MqttContext.h"
#include "wifiContext.h"
#include "processing/SDM220Processing.h"
#include "processing/PzemProcessing.h"

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>

const char* ssid   = "*";                   // SSID and ...
const char* pass   = "*";                   // password for the WiFi network used

const char* mqttServer = "192.168.1.10";    // mqqt server
const int mqttPort     = 1883;
const char* mqttUser   = "mqtt";
const char* mqttPass   = "mqtt";

//const char* mqqtTopicPower  = "powermetter/sdm200/power";
//const char* mqqtTopicEnergy = "powermetter/sdm200/energy";
//const char* mqqtTopicTotal  = "powermetter/sdm200/total";
//const char* mqqtTopicError  = "powermetter/sdm200/error";
//SDM220Processing sdm220Processing;

const char* mqqtTopicPower = "powermetter/pzem/power";
const char* mqqtTopicError = "powermetter/pzem/error";
PzemProcessing pzemProcessing;

WiFiContext wifiContext;
MqttContext mqttContext;
AsyncWebServer server(80);


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);  
  while (!Serial) {}
  Serial.println("__ OK __");

  mqttContext.initializate(mqttServer, mqttPort, mqttUser, mqttPass);
  wifiContext.initializate(mqttContext, ssid, pass);

//  sdm220Processing.initializate(mqttContext, mqqtTopicPower, mqqtTopicEnergy, mqqtTopicTotal, mqqtTopicError);
  pzemProcessing.initializate(mqttContext, mqqtTopicPower, mqqtTopicError);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "Hi! I am ESP32.");
  });

  AsyncElegantOTA.begin(&server);    // Start ElegantOTA
  server.begin();
  Serial.println("HTTP server started");

}

void loop(void) {
  AsyncElegantOTA.loop();
}
