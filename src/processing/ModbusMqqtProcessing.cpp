
#include "ModbusMqqtProcessing.h"
#include <Arduino.h>
#include "ModbusClientRTU.h"
#include <ArduinoJson.h>
#include "utils/Log.h"

ModbusMqqtProcessing::ModbusMqqtProcessing(IPublisher &publisher, const DeviceConfig &config)
    : _publisher(publisher), _config(config), _mb(config.rs485RtsPin)
{
}

void ModbusMqqtProcessing::initializateModbus(MBOnData onDataHandler, MBOnError onErrorHandler) {

  Serial2.begin(_config.modbusBaud, SERIAL_8N1, GPIO_NUM_17, GPIO_NUM_16);

// Set up ModbusRTU client.
// - provide onData handler function
  _mb.onDataHandler(onDataHandler);
// - provide onError handler function
  _mb.onErrorHandler(onErrorHandler);
// Set message timeout to 2000ms
  _mb.setTimeout(2000);
// Start ModbusRTU background task.
// В eModbus 1.7.x serial передаётся сюда, а не в конструктор.
  _mb.begin(Serial2);
}

String ModbusMqqtProcessing::errorToJson(Error error) {
  JsonDocument doc;
  ModbusError me(error);

  doc[PSTR("code")] = (int)me;
  doc[PSTR("message")] = (const char *)me;
  String output;
  serializeJson(doc, output);
  return output;
}

bool ModbusMqqtProcessing::request(uint32_t token, uint32_t start, uint32_t amount) {
  Error err = _mb.addRequest(token, _config.modbusServerId, READ_INPUT_REGISTER, start, amount);
  if (err != SUCCESS) {
    ModbusError e(err);
    LOG(PSTR("ModbusMqqtProcessing::request Error creating request: %02X - %s"), (unsigned)(int)e, (const char *)e);
    return false;
  }
  return true;
}

void ModbusMqqtProcessing::logResponse(ModbusMessage &response, uint32_t token) {
  LOG_MODBUS(PSTR("ModbusMqqtProcessing::logResponse serverID=%d, FC=%d, Token=%08X, length=%u")
            ,response.getServerID(), response.getFunctionCode(), (unsigned)token, (unsigned)response.size());
}

void ModbusMqqtProcessing::publishError(Error error, uint32_t token) {
  // ModbusError wraps the error code and provides a readable error message for it
  ModbusError me(error);
  LOG(PSTR("ModbusMqqtProcessing::publishError token=%08X %02X - %s"), (unsigned)token, (unsigned)(int)me, (const char *)me);
  _publisher.publish(_config.topicError, errorToJson(error).c_str());
}
