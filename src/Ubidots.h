#ifndef _Ubidots_H_
#define _Ubidots_H_

#include "UbiConstants.h"
#include "UbiProtocol.h"
#include "UbiProtocolHandler.h"
#include "UbiTypes.h"
#include "WiFiNINA.h"

class Ubidots {
 public:
  explicit Ubidots(const char *token);
  explicit Ubidots(const char *token, UbiServer server);
  explicit Ubidots(const char *token, UbiServer server, IotProtocol iotProtocol);
  void add(char *variable_label, float value);
  void add(char *variable_label, float value, char *context);
  void add(char *variable_label, float value, char *context, unsigned long dot_timestamp_seconds);
  void add(char *variable_label, float value, char *context, unsigned long dot_timestamp_seconds,
           unsigned int dot_timestamp_millis);
  void addContext(char *key_label, char *key_value);
  void getContext(char *context_result);
  bool send();
  bool send(const char *device_label);
  bool send(const char *device_label, const char *device_name);
  float get(const char *device_label, const char *variable_label);
  void setDebug(bool debug);
  void init(const char *ssid, const char *password);
  ~Ubidots();

 private:
  bool _debug = false;
  void printWiFiStatus();
  int _status = WL_IDLE_STATUS;
  uint8_t _connect_attempts = 0;
  UbiProtocolHandler *_cloudProtocol;
};

#endif