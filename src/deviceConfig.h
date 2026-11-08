#pragma once
#ifndef _DEVICE_CONFIG_H
#define _DEVICE_CONFIG_H

#include <stdint.h>
#include <stddef.h>

/* Всё, что отличается между устройствами, собрано в одной структуре.
   Раньше это было размазано #ifdef'ами по трём файлам: топики в main.cpp,
   IP-адрес в WiFiContext.cpp, скорость порта в вызове initializate.

   Какое устройство собирается, решает окружение в platformio.ini
   (-D DEVICE_SDM220 / -D DEVICE_PZEM). */

struct DeviceConfig
{
    const char *name;

    uint8_t ipLastOctet;  // остальная часть адреса - в configuration.h

    unsigned long modbusBaud;
    uint8_t modbusServerId;  // был зашит константой 1 в request()
    int8_t rs485RtsPin;      // -1 = трансивер с авто-направлением

    // NULL = набор регистров у этого устройства отсутствует
    const char *topicPower;
    const char *topicEnergy;
    const char *topicTotal;
    const char *topicError;
    const char *topicAvailability;
};

extern const DeviceConfig deviceConfig;

#endif // _DEVICE_CONFIG_H
