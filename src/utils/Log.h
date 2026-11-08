#pragma once
#ifndef _LOG_H
#define _LOG_H

#include "configuration.h"
#include "Logger.h"

/* Раньше каждый вызов лога был обёрнут парой вложенных #ifdef, и эта пара
   повторялась около тридцати раз. Теперь область логирования выбирается
   одним макросом, а сами #ifdef живут только здесь.

   Logger вызывается статически: он единственный на прошивку, а глобальный
   объект `logger` с `extern Logger logger;` в восьми файлах только создавал
   видимость экземпляра. */

#ifdef debug
#  define LOG(...) Logger::log(__VA_ARGS__)
#else
#  define LOG(...) ((void)0)
#endif

#if defined(debug) && defined(debug_mqtt)
#  define LOG_MQTT(...) Logger::log(__VA_ARGS__)
#else
#  define LOG_MQTT(...) ((void)0)
#endif

#if defined(debug) && defined(debug_processing)
#  define LOG_PROC(...) Logger::log(__VA_ARGS__)
#else
#  define LOG_PROC(...) ((void)0)
#endif

#if defined(debug) && defined(debug_initialization)
#  define LOG_INIT(...) Logger::log(__VA_ARGS__)
#else
#  define LOG_INIT(...) ((void)0)
#endif

#if defined(debug) && defined(modbus_Processing)
#  define LOG_MODBUS(...) Logger::log(__VA_ARGS__)
#else
#  define LOG_MODBUS(...) ((void)0)
#endif

#endif // _LOG_H
