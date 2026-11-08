#pragma once
#ifndef _LOGGER_H
#define _LOGGER_H

#include "configuration.h"
#include <Arduino.h>
#include <ESPAsyncWebServer.h>

extern "C" {
  #include "freertos/FreeRTOS.h"
  #include "freertos/semphr.h"
}

class Logger {

public:

    /* Вызывается из тасков eModbus, AsyncTCP, службы таймеров и событий WiFi,
       поэтому всё состояние под мьютексом. format(printf) включает проверку
       формат-строк компилятором. */
    static size_t log(const char *format, ...) __attribute__((format(printf, 1, 2)));
    static void logInitialization();
    static void printBuffer();

    static void publishLog(AsyncWebServerRequest *request);

private:

    static char buffer[sizeOfLog][msgLength];
    static uint32_t position;
    static uint32_t size;
    static SemaphoreHandle_t mutex;

    static uint32_t addPosition(uint32_t pos);
    static void lock();
    static void unlock();

    /* Границы для обхода буфера без его копирования. */
    static void snapshot(uint32_t &firstPos, uint32_t &count);

    /* Копирует одну строку снимка в out вместе с '\n'.
       Возвращает число записанных байт. */
    static size_t copyLine(uint32_t firstPos, uint32_t lineIndex, char *out, size_t outSize);

};


#endif
