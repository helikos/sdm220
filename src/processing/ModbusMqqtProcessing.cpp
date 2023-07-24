
#include "ModbusMqqtProcessing.h"
#include <Arduino.h>
#include "ModbusClientRTU.h"
#include <ArduinoJson.h>
#include "utils/Logger.h"

ModbusClientRTU ModbusMqqtProcessing::_mb(Serial2);
const char* ModbusMqqtProcessing::_mqqtTopicError = NULL;
extern Logger logger;


void ModbusMqqtProcessing::initializate(MqttContext mqttContext
                                       ,unsigned long baud
                                       ,MBOnData onDataHandler
                                       ,MBOnError onErrorHandler
                                       ,const char* mqqtTopicError) {

  _mqttContext = mqttContext;
  _mqqtTopicError = mqqtTopicError;

  Serial2.begin(baud, SERIAL_8N1, GPIO_NUM_17, GPIO_NUM_16);

// Set up ModbusRTU client.
// - provide onData handler function
  _mb.onDataHandler(onDataHandler);
// - provide onError handler function
  _mb.onErrorHandler(onErrorHandler);
// Set message timeout to 2000ms
  _mb.setTimeout(2000);
// Start ModbusRTU background task
  _mb.begin();
}

String ModbusMqqtProcessing::errorToJson(Error error) {
  DynamicJsonDocument doc(1024);
  ModbusError me(error);

  doc[PSTR("code")] = (int)me;
  doc[PSTR("message")] = (const char *)me;
  String output;
  serializeJson(doc, output);
  return output;
}

void ModbusMqqtProcessing::request(uint32_t token, uint32_t start, uint32_t amount) {
  Error err = _mb.addRequest(token, 1, READ_INPUT_REGISTER, start, amount);
  if (err!=SUCCESS) {
    ModbusError e(err);
    logger.log(PSTR("ModbusMqqtProcessing::request Error creating request: %02X - %s"), (int)e, (const char *)e);
  }
};

void ModbusMqqtProcessing::handleData(ModbusMessage response, uint32_t token) {

#ifdef debug    
#ifdef modbus_Processing
  logger.log(PSTR("ModbusMqqtProcessing::handleData Response: serverID=%d, FC=%d, Token=%08X, length=%d:"),  response.getServerID(), response.getFunctionCode(), token, response.size());
  for (auto& byte : response) {
    logger.log(PSTR("%02X "), byte);
  }
  logger.log(PSTR(""));
#endif
#endif
}

void ModbusMqqtProcessing::handleError(Error error, uint32_t token) {
  // ModbusError wraps the error code and provides a readable error message for it
  ModbusError me(error);
  logger.log(PSTR("SDM220Processing::handleError Error response: token=%08X %02X - %s"), token, (int)me, (const char *)me);
  _mqttContext.publishMessage(_mqqtTopicError, errorToJson(error).c_str());
}
