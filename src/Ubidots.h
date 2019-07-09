#ifndef _Ubidots_H_
#define _Ubidots_H_

#include "WiFiNINA.h"

typedef struct Dot {
  char *variable_label;
  char *dot_context;
  float dot_value;
  unsigned long dot_timestamp_seconds;
  unsigned int dot_timestamp_millis;
} Dot;

typedef enum { 
  UBI_HTTPS, 
  UBI_HTTP,
  UBI_TCP, 
  UBI_UDP
} IotProtocol;

typedef struct ContextUbi {
  char *key_label;
  char *key_value;
} ContextUbi;

class Ubidots {
  private:
    void buildHttpPayload(char *payload);
    void buildTcpPayload(char *payload, const char *device_label, const char *device_name);
    bool waitServerAnswer();
    float parseHttpAnswer(const char *request_type, char* data);
    float parseTcpAnswer(const char *request_type, char* data);
    void readServerAnswer(char *response);

    bool sendHTTP(const char *device_label, char *payload);
    bool sendTCP(char *payload);

    float getHTTP(const char *device_label, const char *variable_label);
    void getContext(char* context_result, IotProtocol iotProtocol);

    void _floatToChar(char * strValue, float value);

    bool connect();
    int _getProtocolPort();

    const char * _token;
    const char * _default_device_label = "arduino-mkr1010";
    const char * _default_device_name = "Arduino-MKR-1010";
    bool _debug = false;
    bool _dirty = false;
    int _timeout = 5000;
    uint8_t _current_value = 0;
    uint8_t _maxReconnectAttempts = 5;
    int8_t _current_context = 0;
    Dot *_dots;
    IotProtocol _iotProtocol;
    ContextUbi *_context;
    WiFiClient _client_ubi;
    WiFiSSLClient _client_ubi_secure;

    
  public:
    explicit Ubidots(const char *token, IotProtocol iotprotocol = UBI_HTTP);

    void add(char *variable_label, float value);
    void add(char *variable_label, float value, char *context);
    void add(char *variable_label, float value, char *context, unsigned long dot_timestamp_seconds);
    void add(char *variable_label, float value, char *context, unsigned long dot_timestamp_seconds, unsigned int dot_timestamp_millis);

    void addContext(char* key_label, char* key_value);
    void getContext(char* context_result);

    bool send();
    bool send(const char *device_label);
    bool send(const char *device_label, const char *device_name);

    float get(const char *device_label, const char *variable_label);

    void setDebug(bool debug);
  
    //~Ubidots();
  
};

#endif
