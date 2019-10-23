#ifndef _UbiProtocolHandler_H_
#define _UbiProtocolHandler_H_

#include "UbiConstants.h"
#include "UbiProtocol.h"
#include "UbiTypes.h"

class UbiProtocolHandler {
 public:
  explicit UbiProtocolHandler(const char* token, UbiServer server, IotProtocol iot_protocol);
  void add(char* variable_label, float value, char* context, unsigned long dot_timestamp_seconds,
           unsigned int dot_timestamp_millis);
  void addContext(char* key_label, char* key_value);
  void getContext(char* context_result);
  bool send(const char* device_label, const char* device_name);
  float get(const char* device_label, const char* variable_label);
  void setDebug(bool debug);
  ~UbiProtocolHandler();

 private:
  const char* _token;
  bool _debug = false;
  UbiProtocol* _ubiProtocol;
};

#endif