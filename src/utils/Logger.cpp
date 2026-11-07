#include "Logger.h"
#include "configuration.h"
#include <Arduino.h>
#include <cstdarg>

char Logger::buffer[sizeOfLog][msgLength];
uint32_t Logger::position = 0;
uint32_t Logger::size = 0;
SemaphoreHandle_t Logger::mutex = NULL;

void Logger::lock() {
    if (mutex != NULL) {
        xSemaphoreTake(mutex, portMAX_DELAY);
    }
}

void Logger::unlock() {
    if (mutex != NULL) {
        xSemaphoreGive(mutex);
    }
}

size_t Logger::log(const char *format, ...) {

    Logger::lock();

    char *msg = Logger::buffer[Logger::position];

    va_list arg;
    va_start(arg, format);
    int len = vsnprintf(msg, msgLength, format, arg);
    va_end(arg);

    if (len < 0) {
        msg[0] = '\0';
        Logger::unlock();
        return 0;
    }

#ifdef isPrint2Serial
    // Печатаем ту же строку, что легла в кольцевой буфер: отдельный общий
    // буфер под Serial был источником гонки между тасками.
    Serial.println(msg);
#endif

    Logger::position = Logger::addPosition(Logger::position);
    if (Logger::size < sizeOfLog) {
        Logger::size++;
    }

    Logger::unlock();
    return (size_t)len;
}

uint32_t Logger::addPosition(uint32_t pos) {
    if (pos < sizeOfLog - 1) {
        return pos + 1;
    } else {
        return 0;
    }
}

void Logger::printBuffer() {
    Logger::lock();
    uint32_t pos = 0;
    if (Logger::size >= sizeOfLog)
        pos = Logger::position;
    for (uint32_t i = 0; i < Logger::size; i++) {
        Serial.println(Logger::buffer[pos]);
        pos = Logger::addPosition(pos);
    }
    Logger::unlock();
}


void Logger::logInitialization() {
    Serial.println("Logger::logInitialization start");
    // Буфер лежит в .bss, аллокаций нет: ни фрагментации, ни зависимости
    // от порядка инициализации глобальных объектов.
    Logger::mutex = xSemaphoreCreateMutex();
    Logger::position = 0;
    Logger::size = 0;
    Serial.println("Logger::logInitialization completed");
}

void Logger::publishLog(AsyncWebServerRequest *request) {
    String status = "Hi! I am sdm220 electricity sensor.\n";
    Logger::lock();
    uint32_t pos = 0;
    if (Logger::size >= sizeOfLog)
        pos = Logger::position;
    for (uint32_t i = 0; i < Logger::size; i++) {
        status += String(Logger::buffer[pos]) + "\n";
        pos = Logger::addPosition(pos);
    }
    Logger::unlock();
    request->send(200, "text/plain", status);

}
