#include "Logger.h"
#include "configuration.h"
#include <Arduino.h>
#include <forward_list> 
#include <cstdarg>

char* Logger::buffer[sizeOfLog];
u_int32_t Logger::position = 0;
u_int32_t Logger::size = 0;

#ifdef isPrint2Serial
    char* Logger::serial_msg = new char[1024];
#endif    


size_t Logger::log(const char *format, ...) {

    va_list arg;
    va_list copy;
    va_start(arg, format);
    va_copy(copy, arg);

    char* msg = Logger::buffer[Logger::position];
    int len = vsnprintf(msg, msgLength, format, copy);
#ifdef isPrint2Serial
    vsnprintf(Logger::serial_msg, 1024, format, copy);
    Serial.println(Logger::serial_msg);
#endif    
    va_end(copy);
    if(len < 0) {
        va_end(arg);
        return 0;
    }
    va_end(arg);
    Logger::position = Logger::addPosition(Logger::position);
    if (Logger::size < sizeOfLog) {
        Logger::size++;
    }
    return len;
}

u_int32_t Logger::addPosition(u_int32_t pos) {
    if (pos < sizeOfLog - 1) {
        return pos + 1;
    } else {
        return 0;
    }
}

void Logger::printBuffer() {
    u_int32_t pos = 0;
    if (Logger::size >= sizeOfLog)
        pos = Logger::position;
    for (u_int32_t i = 0; i < Logger::size; i++) {
        Serial.println(Logger::buffer[pos]);
        pos = Logger::addPosition(pos);
    }
}

    
void Logger::logInitialization() {
    Serial.println("Logger::logInitialization start");
    Logger::position = 0;
    Logger::size = 0;
    for (u_int32_t i = 0; i < sizeOfLog; i++) {
        Logger::buffer[i] = new char[msgLength];
    }
    Serial.println("Logger::logInitialization completed");
}

void Logger::publishLog(AsyncWebServerRequest *request) {
    String status = "Hi! I am sdm220 electricity sensor.\n";
    u_int32_t pos = 0;
    if (Logger::size >= sizeOfLog)
        pos = Logger::position;
    for (u_int32_t i = 0; i < Logger::size; i++) {
        status += String(Logger::buffer[pos])+"\n";
        pos = Logger::addPosition(pos);
    }
    request->send(200, "text/plain", status);

}
