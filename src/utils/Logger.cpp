#include "Logger.h"
#include "configuration.h"
#include <Arduino.h>
#include <cstdarg>

static const char logHeader[] = "Hi! I am sdm220 electricity sensor.\n";

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

void Logger::snapshot(uint32_t &firstPos, uint32_t &count) {
    Logger::lock();
    count = Logger::size;
    firstPos = (Logger::size >= sizeOfLog) ? Logger::position : 0;
    Logger::unlock();
}

size_t Logger::copyLine(uint32_t firstPos, uint32_t lineIndex, char *out, size_t outSize) {
    // Мьютекс держим только на копирование одной строки: остальные таски
    // продолжают писать в тот же кольцевой буфер. Индекс всегда в границах
    // статического массива, так что выйти за пределы невозможно - в худшем
    // случае строка окажется уже перезаписанной более свежей.
    Logger::lock();
    const int n = snprintf(out, outSize, "%s\n", Logger::buffer[(firstPos + lineIndex) % sizeOfLog]);
    Logger::unlock();

    if (n < 0) {
        return 0;
    }
    return ((size_t)n < outSize) ? (size_t)n : (outSize - 1);
}

void Logger::publishLog(AsyncWebServerRequest *request) {
    uint32_t firstPos = 0;
    uint32_t count = 0;
    Logger::snapshot(firstPos, count);

    /* Раньше здесь собиралась одна String на весь буфер (~32 КБ) прямо
       в таске AsyncTCP - скачок кучи на каждый запрос /log. Теперь
       AsyncWebServer вызывает filler столько раз, сколько нужно, и самый
       большой буфер в игре - одна строка лога.

       piece и offset захвачены по значению в mutable-лямбду: ответ хранит
       её копию и дёргает повторно, поэтому состояние обхода сохраняется. */
    uint32_t piece = 0;   // 0 = заголовок, 1..count = строки лога
    uint32_t offset = 0;  // сколько байт текущего куска уже отдано

    request->sendChunked("text/plain",
        [firstPos, count, piece, offset](uint8_t *out, size_t maxLen, size_t index) mutable -> size_t {
            (void)index;

            size_t written = 0;
            char line[msgLength + 2];

            while (piece <= count && written < maxLen) {
                // Заголовок отдаём прямо из статической строки: копировать его
                // в line нельзя - буфер рассчитан под строку лога, а заголовок
                // длиннее, и при малом msgLength это затирало бы стек.
                const char *src;
                size_t len;
                if (piece == 0) {
                    src = logHeader;
                    len = strlen(logHeader);
                } else {
                    len = Logger::copyLine(firstPos, piece - 1, line, sizeof(line));
                    src = line;
                }

                if (offset >= len) {  // кусок пуст или уже отдан целиком
                    piece++;
                    offset = 0;
                    continue;
                }

                size_t chunk = len - offset;
                if (chunk > maxLen - written) {
                    chunk = maxLen - written;
                }
                memcpy(out + written, src + offset, chunk);
                written += chunk;
                offset += chunk;

                if (offset >= len) {
                    piece++;
                    offset = 0;
                }
            }

            // 0 означает терминирующий chunk, поэтому возвращаем его только
            // когда все куски действительно отданы.
            return written;
        });
}
