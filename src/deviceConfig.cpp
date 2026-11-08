
#include "deviceConfig.h"

#if defined(DEVICE_SDM220) == defined(DEVICE_PZEM)
#error "Выберите ровно одно устройство: pio run -e sdm220 либо -e pzem"
#endif

#ifdef DEVICE_SDM220
const DeviceConfig deviceConfig = {
    /* name            */ "sdm220",
    /* ipLastOctet     */ 183,
    /* modbusBaud      */ 2400,
    /* modbusServerId  */ 1,
    /* rs485RtsPin     */ -1,
    /* topicPower      */ "powermetter/sdm200/power",
    /* topicEnergy     */ "powermetter/sdm200/energy",
    /* topicTotal      */ "powermetter/sdm200/total",
    /* topicError      */ "powermetter/sdm200/error",
    /* topicAvailability*/ "powermetter/sdm200/availability",
};
#endif

#ifdef DEVICE_PZEM
const DeviceConfig deviceConfig = {
    /* name            */ "pzem",
    /* ipLastOctet     */ 182,
    /* modbusBaud      */ 9600,
    /* modbusServerId  */ 1,
    /* rs485RtsPin     */ -1,
    /* topicPower      */ "powermetter/pzem/power",
    /* topicEnergy     */ NULL,
    /* topicTotal      */ NULL,
    /* topicError      */ "powermetter/pzem/error",
    /* topicAvailability*/ "powermetter/pzem/availability",
};
#endif
