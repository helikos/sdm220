#pragma once

#ifndef _CONFIGURATION_H
#define _CONFIGURATION_H

#include <WiFi.h>


#define debug
#define debug_processing
#define debug_mqtt
#define debug_initialization
#define isPrint2Serial


// #define configTIMER_QUEUE_LENGTH 20

/* Macros for getting elapsed time */
#define numberOfSeconds(_time_) ((_time_ / 1000) % 60)
#define numberOfMinutes(_time_) (((_time_ / 1000) / 60) % 60)
#define numberOfHours(_time_) (((_time_ / 1000) / 60) / 60)
#define numberOfDays(_time_) ((((_time_ / 1000) / 60) / 60) / 60)


#define _SSID       "t_k"                   // SSID and ...
#define _PASS       "tk12-45rfg-98ijd"      // password for the WiFi network used
#define _MQTTSERVER "192.168.1.10"          // mqqt server
#define _MQTTPORT   1883
#define _MQTTUSER   "mqtt"
#define _MQTTPASS   "mqtt"

#define sizeOfLog 256
#define msgLength 64

#endif
