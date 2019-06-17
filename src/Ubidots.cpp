#include "Ubidots.h"
#include "UbiConstants.h"
#include "string.h"
#include "avr/dtostrf.h"

/**************************************************************************
 * constructors
 ***************************************************************************/

Ubidots::Ubidots(const char *token, IotProtocol iotprotocol) {
  _token = token;
  _iotProtocol = iotprotocol;
  _dots = (Dot *) malloc(sizeof(Dot) * MAX_VALUES);
}

void Ubidots::add(char *variable_label, float value) {
  add(variable_label, value, NULL, NULL, NULL);
}

void Ubidots::add(char *variable_label, float value, char *context) {
  add(variable_label, value, context, NULL, NULL);
}

void Ubidots::add(char *variable_label, float value, char *context, long unsigned dot_timestamp_seconds) {
  add(variable_label, value, context, dot_timestamp_seconds, NULL);
}

void Ubidots::add(char* variable_label, float value, char *context, long unsigned dot_timestamp_seconds, unsigned int dot_timestamp_millis) {
  (_dots + _current_value)->variable_label = variable_label;
  (_dots + _current_value)->dot_value = value;
  (_dots + _current_value)->dot_context = context;
  (_dots + _current_value)->dot_timestamp_seconds = dot_timestamp_seconds;
  (_dots + _current_value)->dot_timestamp_millis = dot_timestamp_millis;

  _current_value++;
  if (_current_value > MAX_VALUES) {
    if (_debug) {
      Serial.println(F("You are sending more than the maximum of consecutive variables"));
    }
    _current_value = MAX_VALUES;
  }
}

bool Ubidots::send() {
  return send(_default_device_label);
}

bool Ubidots::send(const char *device_label) {
  // Builds the payload
  bool result = false;
  char *payload = (char *) malloc(sizeof(char) *MAX_BUFFER_SIZE);
  if (_iotProtocol == UBI_TCP || _iotProtocol == UBI_UDP) {
    //buildTcpPayload(payload, device_label, device_name);
  } else {
    buildHttpPayload(payload);
  }
  
  if (connect()) {
    /* Builds the request POST - Please reference this link to know all the
     * request's structures https://ubidots.com/docs/api/ */
    _client_ubi.print(F("POST /api/v1.6/devices/"));
    _client_ubi.print(device_label);
    _client_ubi.print(F(" HTTP/1.1\r\n"));
    _client_ubi.print(F("Host: "));
    _client_ubi.print(UBIDOTS_SERVER);
    _client_ubi.print(F("\r\n"));
    _client_ubi.print(F("User-Agent: "));
    _client_ubi.print(USER_AGENT);
    _client_ubi.print(F("\r\n"));
    _client_ubi.print(F("X-Auth-Token: "));
    _client_ubi.print(_token);
    _client_ubi.print(F("\r\n"));
    _client_ubi.print(F("Connection: close\r\n"));
    _client_ubi.print(F("Content-Type: application/json\r\n"));
    int content_length = strlen(payload);
    _client_ubi.print(F("Content-Length: "));
    _client_ubi.print(content_length);
    _client_ubi.print(F("\r\n\r\n"));
    _client_ubi.print(payload);
    _client_ubi.print(F("\r\n"));
    _client_ubi.flush();

    if (_debug) {
      Serial.println(F("Making request to Ubidots:\n"));
      Serial.print("POST /api/v1.6/devices/");
      Serial.print(device_label);
      Serial.print(" HTTP/1.1\r\n");
      Serial.print("Host: ");
      Serial.print(UBIDOTS_SERVER);
      Serial.print("\r\n");
      Serial.print("User-Agent: ");
      Serial.print(USER_AGENT);
      Serial.print("\r\n");
      Serial.print("X-Auth-Token: ");
      Serial.print(_token);
      Serial.print("\r\n");
      Serial.print("Connection: close\r\n");
      Serial.print("Content-Type: application/json\r\n");
      Serial.print("Content-Length: ");
      Serial.print(content_length);
      Serial.print("\r\n\r\n");
      Serial.print(payload);
      Serial.print("\r\n");

      Serial.println("waiting for server answer ...");
    }

    /* Reads the response from the server */
    waitServerAnswer();

    if (_debug) {
      Serial.println("\nUbidots' Server response:\n");
      while (_client_ubi.available()) {
        char c = _client_ubi.read();
        Serial.print(c);
      }
    }
    result = true;
  } else {
    if (_debug) {
      Serial.println("Could not send data to ubidots using HTTP");
    }
  }

  _client_ubi.stop();
  _current_value = 0;
  return result;
}


void Ubidots::buildHttpPayload(char *payload) {
  /* Builds the payload */
  sprintf(payload, "{");

  for (uint8_t i = 0; i < _current_value;) {
    char strValue[MAX_VALUE_LENGTH];
    _floatToChar(strValue, (_dots + i)->dot_value);
    sprintf(payload, "%s\"%s\":{\"value\":%s", payload, (_dots + i)->variable_label, strValue);

    // Adds timestamp seconds
    if ((_dots + i)->dot_timestamp_seconds != NULL) {
      sprintf(payload, "%s,\"timestamp\":%lu", payload,
              (_dots + i)->dot_timestamp_seconds);
      // Adds timestamp milliseconds
      if ((_dots + i)->dot_timestamp_millis != NULL) {
        char milliseconds[3];
        int timestamp_millis = (_dots + i)->dot_timestamp_millis;
        uint8_t units = timestamp_millis % 10;
        uint8_t dec = (timestamp_millis / 10) % 10;
        uint8_t hund = (timestamp_millis / 100) % 10;
        sprintf(milliseconds, "%d%d%d", hund, dec, units);
        sprintf(payload, "%s%s", payload, milliseconds);
      } else {
        sprintf(payload, "%s000", payload);
      }
    }

    // Adds dot context
    if ((_dots + i)->dot_context != NULL) {
      sprintf(payload, "%s,\"context\": {%s}", payload,
              (_dots + i)->dot_context);
    }

    sprintf(payload, "%s}", payload);
    i++;

    if (i < _current_value) {
      sprintf(payload, "%s,", payload);
    } else {
      sprintf(payload, "%s}", payload);
      _current_value = 0;
    }
  }
}

bool Ubidots::waitServerAnswer() {
  int timeout = 0;
  while (!_client_ubi.available() && timeout < _timeout) {
    timeout++;
    delay(1);
    if (timeout > _timeout - 1) {
      if (_debug) {
        Serial.println("timeout, could not read any response from the host");
      }
      return false;
    }
  }
  return true;
}

bool Ubidots::connect() {
  int port = _getProtocolPort();
  uint8_t attempts = 0;
  bool connect = false;
  while (!_client_ubi.connected() && attempts < _maxReconnectAttempts) {
    if (_debug) {
      Serial.print("Trying to connect to ");
      Serial.print(UBIDOTS_SERVER);
      Serial.print(" , attempt number: ");
      Serial.println(attempts);
    }
    connect = _client_ubi.connect(UBIDOTS_SERVER, port);
    if (_debug) {
      Serial.println("Attempt finished");
    }
    attempts += 1;
    delay(1000);
  }
  return connect;
}

int Ubidots::_getProtocolPort() {
  int port;
  if (_iotProtocol == UBI_TCP || _iotProtocol == UBI_UDP) {
    port = UBIDOTS_TCP_PORT;
  } else if (_iotProtocol == UBI_HTTPS) {
    port = UBIDOTS_HTTPS_PORT;
  } else {
    port = UBIDOTS_HTTP_PORT;
  }
  return port;
}

void Ubidots::_floatToChar(char * strValue, float value) {
  char tempValue[MAX_VALUE_LENGTH];
  dtostrf(value, MAX_VALUE_LENGTH, MAX_DECIMAL_PLACES, tempValue);
  uint8_t k = 0;
  for(uint8_t j = 0; j < MAX_VALUE_LENGTH;) {
    if(tempValue[j] != ' ') {
      strValue[k] = tempValue[j];
      k++;
    }
    j++;
  }
  strValue[k] = '\0';
}

void Ubidots::setDebug(bool debug) {
  _debug = debug;
}