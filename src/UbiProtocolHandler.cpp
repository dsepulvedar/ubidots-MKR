#include "UbiProtocolHandler.h"
#include "UbiBuilder.h"
#include "UbiConstants.h"
#include "UbiProtocol.h"
#include "UbiTypes.h"

/**************************************************************************
 * Overloaded constructors
 ***************************************************************************/

UbiProtocolHandler::UbiProtocolHandler(const char* token, UbiServer server, IotProtocol iot_protocol) {
  UbiBuilder builder(token, server, iot_protocol);
  _ubiProtocol = builder.builder();
}

/**************************************************************************
 * Overloaded destructor
 ***************************************************************************/

UbiProtocolHandler::~UbiProtocolHandler() {
  delete _ubiProtocol;
}

/***************************************************************************
FUNCTIONS TO SEND DATA
***************************************************************************/

/**
 * Add a value of variable to save
 * @arg variable_label [Mandatory] variable label where the dot will be stored
 * @arg value [Mandatory] Dot value
 * @arg context [optional] Dot context to store. Default NULL
 * @arg dot_timestamp_seconds [optional] Dot timestamp in seconds, usefull for
 * datalogger. Default NULL
 * @arg dot_timestamp_millis [optional] Dot timestamp in millis to add to
 * dot_timestamp_seconds, usefull for datalogger.
 */

void UbiProtocolHandler::add(char* variable_label, float value, char* context, unsigned long dot_timestamp_seconds,
                             unsigned int dot_timestamp_millis) {
  _ubiProtocol->add(variable_label, value, context, dot_timestamp_seconds, dot_timestamp_millis);
}

void UbiProtocolHandler::addContext(char* key_label, char* key_value) {
  _ubiProtocol->addContext(key_label, key_value);
}

void UbiProtocolHandler::getContext(char* context_result) {
  _ubiProtocol->getContext(context_result);
}

/**
 * Sends data to Ubidots
 * @arg device_label [Mandatory] device label where the dot will be stored
 * @arg device_name [optional] Name of the device to be created (supported only
 * for TCP/UDP)
 * @arg flags [Optional] Particle publish flags for webhooks
 */

bool UbiProtocolHandler::send(const char* device_label, const char* device_name) {
  if (_debug) {
    //Serial.println("Sending data...");
  }
  bool result = _ubiProtocol->sendData(device_label, device_name);
  return result;
}

float UbiProtocolHandler::get(const char* device_label, const char* variable_label) {
  /*
  if (_iot_protocol == UBI_UDP || _iot_protocol == UBI_PARTICLE) {
    Serial.println(
        "ERROR, data retrieval is only supported using TCP or HTTP protocols");
    return ERROR_VALUE;
  }
  */

  float value = ERROR_VALUE;

  value = _ubiProtocol->get(device_label, variable_label);

  return value;
}


/*
  Makes debug messages available
*/
void UbiProtocolHandler::setDebug(bool debug) { 
  _debug = debug;
  _ubiProtocol->setDebug(debug); 
}
