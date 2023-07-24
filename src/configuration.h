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



#define sizeOfLog 256
#define msgLength 64

#endif
