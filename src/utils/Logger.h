#pragma once
#ifndef _LOGGER_H
#define _LOGGER_H

#include "configuration.h"
#include <Arduino.h>
#include <ESPAsyncWebServer.h>

class Logger {

public:

    static size_t log(const char *format, ...);
    static void logInitialization();
    static void printBuffer();

    static u_int32_t position;
    static u_int32_t size;
    static void publishLog(AsyncWebServerRequest *request);

private:

    static char* buffer[sizeOfLog];
    static u_int32_t addPosition(u_int32_t pos);
#ifdef isPrint2Serial
    static char* serial_msg;
#endif    

};


#endif