
#include <WiFi.h>
#include "WiFiContext.h"

extern "C" {
  #include "freertos/timers.h"
}
#include "MqttContext.h"
#define wifiTimeOfReconect pdMS_TO_TICKS(2000)

TimerHandle_t WiFiContext::_wifiReconnectTimer = NULL;
const char* WiFiContext::_ssid = NULL;
const char* WiFiContext::_pass = NULL;

void WiFiContext::initializate(MqttContext mqttContext, const char* ssid, const char* pass) {
    _wifiReconnectTimer = xTimerCreate("wifiTimer", wifiTimeOfReconect, pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(connectToWifi));
    _mqttContext = mqttContext;
    _ssid = ssid;
    _pass = pass;

    WiFi.onEvent(WiFiEvent);
    connectToWifi();
}

void WiFiContext::connectToWifi() {
  Serial.println("Connecting to Wi-Fi...");
  Serial.printf("ssid=%s, pass= %s \n", _ssid, _pass);
  WiFi.begin(_ssid, _pass);
  delay(1000);
}


void WiFiContext::WiFiEvent(WiFiEvent_t event) {
  Serial.printf("[WiFi-event] event: %d\n", event);
  switch(event) {
    case SYSTEM_EVENT_STA_GOT_IP:
      Serial.println("WiFi connected");  //  "was connnected to WiFi"
      Serial.println("IP address: ");    //  "IP: "
      Serial.println(WiFi.localIP());
      _mqttContext.connectToMqtt();
      break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      Serial.println("WiFi lost connection");
      _mqttContext.stopTimer();
      if (xTimerIsTimerActive(_wifiReconnectTimer) == pdFALSE) {
       Serial.println("WiFi reconect timer start");
       xTimerStart(_wifiReconnectTimer, 0);
      }
      break;
    case SYSTEM_EVENT_WIFI_READY: 
        Serial.println("WiFi interface ready");
        break;
    case SYSTEM_EVENT_SCAN_DONE:
        Serial.println("Completed scan for access points");
        break;
    case SYSTEM_EVENT_STA_START:
        Serial.println("WiFi client started");
        break;
    case SYSTEM_EVENT_STA_STOP:
        Serial.println("WiFi clients stopped");
        break;
    case SYSTEM_EVENT_STA_CONNECTED:
        Serial.println("Connected to access point");
        break;
    case SYSTEM_EVENT_STA_AUTHMODE_CHANGE:
        Serial.println("Authentication mode of access point has changed");
        break;
    case SYSTEM_EVENT_STA_LOST_IP:
        Serial.println("Lost IP address and IP address is reset to 0");
        break;
    case SYSTEM_EVENT_STA_WPS_ER_SUCCESS:
        Serial.println("WiFi Protected Setup (WPS): succeeded in enrollee mode");
        break;
    case SYSTEM_EVENT_STA_WPS_ER_FAILED:
        Serial.println("WiFi Protected Setup (WPS): failed in enrollee mode");
        break;
    case SYSTEM_EVENT_STA_WPS_ER_TIMEOUT:
        Serial.println("WiFi Protected Setup (WPS): timeout in enrollee mode");
        break;
    case SYSTEM_EVENT_STA_WPS_ER_PIN:
        Serial.println("WiFi Protected Setup (WPS): pin code in enrollee mode");
        break;
    case SYSTEM_EVENT_AP_START:
        Serial.println("WiFi access point started");
        break;
    case SYSTEM_EVENT_AP_STOP:
        Serial.println("WiFi access point  stopped");
        break;
    case SYSTEM_EVENT_AP_STACONNECTED:
        Serial.println("Client connected");
        break;
    case SYSTEM_EVENT_AP_STADISCONNECTED:
        Serial.println("Client disconnected");
        break;
    case SYSTEM_EVENT_AP_STAIPASSIGNED:
        Serial.println("Assigned IP address to client");
        break;
    case SYSTEM_EVENT_AP_PROBEREQRECVED:
        Serial.println("Received probe request");
        break;
    case SYSTEM_EVENT_GOT_IP6:
        Serial.println("IPv6 is preferred");
        break;
    case SYSTEM_EVENT_ETH_START:
        Serial.println("Ethernet started");
        break;
    case SYSTEM_EVENT_ETH_STOP:
        Serial.println("Ethernet stopped");
        break;
    case SYSTEM_EVENT_ETH_CONNECTED:
        Serial.println("Ethernet connected");
        break;
    case SYSTEM_EVENT_ETH_DISCONNECTED:
        Serial.println("Ethernet disconnected");
        break;
    case SYSTEM_EVENT_ETH_GOT_IP:
        Serial.println("Obtained IP address");
        break;
    default: break;  
    }
}
