#ifndef _UbiHttp_H_
#define _UbiHttp_H_

#include "UbiConstants.h"
#include "UbiProtocol.h"
#include "WiFiNINA.h"

class UbiHttp : public UbiProtocol {
 public:
  UbiHttp(const char* host, const char* token);
  void add(char* variable_label, float value, char* context, unsigned long dot_timestamp_seconds,
           unsigned int dot_timestamp_millis);
  void addContext(char* key_label, char* key_value);
  void getContext(char* context_result);
  bool sendData(const char* device_label, const char* device_name);
  float get(const char* device_label, const char* variable_label);
  void setDebug(bool debug);
  ~UbiHttp();

 private:
  const char* _host;
  const char* _token;
  bool _debug = false;
  bool connect();
  void buildHttpPayload(char* payload);
  bool waitServerAnswer();
  void _floatToChar(char* strValue, float value);
  float parseHttpAnswer(const char* request_type, char* data);
  void readServerAnswer(char* response);
  int _timeout = 5000;
  uint8_t _maxReconnectAttempts = 5;
  uint8_t _current_value = 0;
  uint8_t _current_context = 0;
  Dot* _dots;
  ContextUbi* _context;
  WiFiSSLClient _client_ubi;
};

#endif