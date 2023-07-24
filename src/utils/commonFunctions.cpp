
#include <Arduino.h>
#include "commonFunctions.h"

#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <ESPAsyncWebServer.h>

extern "C"
{
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
}

using namespace std;

string getPadded(int num, uint8_t n){
    string str = std::to_string(num);
	ostringstream ss;
	ss << std::setw(n) << std::setfill('0') << str;
	return  ss.str();
}

void getFreeHeapSize(AsyncWebServerRequest *request) {
    String status = "Hi! I am smd220.\n";
	status += "Free heap memory size is ";
	status += String(esp_get_free_heap_size());
	status += " bytes \n";
    request->send(200, "text/plain", status);
}