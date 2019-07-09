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
  _context = (ContextUbi *) malloc(MAX_VALUES * sizeof(ContextUbi));
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
  bool result = false;
  if (_iotProtocol == UBI_TCP || _iotProtocol == UBI_UDP) {
    result = send(_default_device_label, _default_device_name);
  } else {
    result = send(_default_device_label);
  }
  return result;
}

bool Ubidots::send(const char *device_label) {
  bool result = false;
  char *payload = (char *) malloc(sizeof(char) *MAX_BUFFER_SIZE);
  if (_iotProtocol == UBI_TCP || _iotProtocol == UBI_UDP) {
    buildTcpPayload(payload, device_label, _default_device_name);
    result = sendTCP(payload);
    return result;
  } else {
    buildHttpPayload(payload);
    result = sendHTTP(device_label, payload);
    return result;
  } 
}

bool Ubidots::send(const char *device_label, const char *device_name){
  bool result = false;
  char *payload = (char *) malloc(sizeof(char) *MAX_BUFFER_SIZE);
  if (_iotProtocol == UBI_TCP || _iotProtocol == UBI_UDP) {
    buildTcpPayload(payload, device_label, device_label);
    result = sendTCP(payload);
    return result;
  } else {
    if(_debug) {
      Serial.println(F("HTTP protocol does not allow a parameter for device name, use send(const char *device_label) instead\n"));
    }
  } 
  return result;
}


bool Ubidots::sendTCP(char *payload) {
  bool result = false;
  if(connect()) {
    _client_ubi.print(payload);
  } else {
    if (_debug) {
      Serial.println("Could not connect to the host");
    }
    _client_ubi.stop();
    return false;
  }

  /* Waits for the host's answer */
  if (!waitServerAnswer()) {
    _client_ubi.stop();
    return false;
  }

  /* Parses the host answer, returns true if it is 'Ok' */
  char* response = (char*)malloc(sizeof(char) * 100);

  float value = parseTcpAnswer("POST", response);
  free(response);
  if (value != ERROR_VALUE) {
    _client_ubi.stop();
    return true;
  }

  _client_ubi.stop();
  return false;
}


bool Ubidots::sendHTTP(const char *device_label, char *payload) {
  bool result = false;
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

float Ubidots::get(const char* device_label, const char* variable_label) {
  float result = false;
  if (_iotProtocol == UBI_TCP || _iotProtocol == UBI_UDP) {
    //_default_device_name = device_name;
    //result = sendTCP(device_label, _default_device_name, payload);
    //return = result;
  } else {
    result = getHTTP(device_label, variable_label);
    return result;
  }
}

void Ubidots::addContext(char* key_label, char* key_value) {
  (_context + _current_context)->key_label = key_label;
  (_context + _current_context)->key_value = key_value;
  _current_context++;
  if (_current_context >= MAX_VALUES) {
    Serial.println(
        F("You are adding more than the maximum of consecutive key-values "
          "pairs"));
    _current_context = MAX_VALUES;
  }
}

void Ubidots::getContext(char* context_result) {
  getContext(context_result, _iotProtocol);
}

void Ubidots::getContext(char* context_result, IotProtocol iotProtocol) {
  // TCP context type
  if (iotProtocol == UBI_TCP || iotProtocol == UBI_UDP) {
    sprintf(context_result, "");
    for (uint8_t i = 0; i < _current_context;) {
      sprintf(context_result, "%s%s=%s", context_result,
              (_context + i)->key_label, (_context + i)->key_value);
      i++;
      if (i < _current_context) {
        sprintf(context_result, "%s$", context_result);
      } else {
        sprintf(context_result, "%s", context_result);
        _current_context = 0;
      }
    }
  }

  // HTTP context type
  if (iotProtocol == UBI_HTTP) {
    sprintf(context_result, "");
    for (uint8_t i = 0; i < _current_context;) {
      sprintf(context_result, "%s\"%s\":\"%s\"", context_result,
              (_context + i)->key_label, (_context + i)->key_value);
      i++;
      if (i < _current_context) {
        sprintf(context_result, "%s,", context_result);
      } else {
        sprintf(context_result, "%s", context_result);
        _current_context = 0;
      }
    }
  }
}

float Ubidots::getHTTP(const char* device_label, const char* variable_label) {
  if (connect()) {
    /* Builds the request GET - Please reference this link to know all the
     * request's structures HTTPS://ubidots.com/docs/api/ */
    _client_ubi.print(F("GET /api/v1.6/devices/"));
    _client_ubi.print(device_label);
    _client_ubi.print("/");
    _client_ubi.print(variable_label);
    _client_ubi.print("/lv");
    _client_ubi.print(" HTTP/1.1\r\n");
    _client_ubi.print("Host: ");
    _client_ubi.print(UBIDOTS_HTTP_PORT);
    _client_ubi.print("\r\n");
    _client_ubi.print("User-Agent: ");
    _client_ubi.print(USER_AGENT);
    _client_ubi.print("\r\n");
    _client_ubi.print("X-Auth-Token: ");
    _client_ubi.print(_token);
    _client_ubi.print("\r\n");
    _client_ubi.print("Content-Type: application/json\r\n\r\n");

    if (_debug) {
      Serial.print("GET /api/v1.6/devices/");
      Serial.print(device_label);
      Serial.print("/");
      Serial.print(variable_label);
      Serial.print("/lv");
      Serial.print(" HTTP/1.1\r\n");
      Serial.print("Host: ");
      Serial.print(UBIDOTS_HTTP_PORT);
      Serial.print("\r\n");
      Serial.print("User-Agent: ");
      Serial.print(USER_AGENT);
      Serial.print("\r\n");
      Serial.print("X-Auth-Token: ");
      Serial.print(_token);
      Serial.print("\r\n");
      Serial.print("Content-Type: application/json\r\n\r\n");
    }

    /* Waits for the host's answer */
    if (!waitServerAnswer()) {
      _client_ubi.stop();
      return ERROR_VALUE;
    }

    /* Reads the response from the server */
    char* response = (char *) malloc(sizeof(char) * MAX_BUFFER_SIZE);
    readServerAnswer(response);

    /* Parses the answer */
    float value = parseHttpAnswer("LV", response);
    _client_ubi.stop();
    free(response);
    return value;
  }

  if (_debug) {
    Serial.println("Connection Failed ubidots - Try Again");
  }

  _client_ubi.stop();
  return ERROR_VALUE;
}

void Ubidots::buildHttpPayload(char *payload) {
  /* Builds the payload for HTTP*/
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
      sprintf(payload, "%s,\"context\":{%s}", payload,
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


void Ubidots::buildTcpPayload(char *payload, const char *device_label, const char *device_name) {
  sprintf(payload, "");
  sprintf(payload, "%s|POST|%s|", USER_AGENT, _token);
  sprintf(payload, "%s%s:%s", payload, device_label, device_name);
  sprintf(payload, "%s=>", payload);
  for (uint8_t i = 0; i < _current_value;) {
    char str_value[20];
    _floatToChar(str_value, (_dots + i)->dot_value);
    sprintf(payload, "%s%s:%s", payload, (_dots + i)->variable_label,
            str_value);

    // Adds dot context
    if ((_dots + i)->dot_context != NULL) {
      sprintf(payload, "%s$%s", payload, (_dots + i)->dot_context);
    }

    // Adds timestamp seconds
    if ((_dots + i)->dot_timestamp_seconds != NULL) {
      sprintf(payload, "%s@%lu", payload, (_dots + i)->dot_timestamp_seconds);
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

    i++;

    if (i < _current_value) {
      sprintf(payload, "%s,", payload);
    } else {
      sprintf(payload, "%s|end", payload);
      _current_value = 0;
    }
  }

  if (_debug) {
    Serial.println("----------");
    Serial.println("payload:");
    Serial.println(payload);
    Serial.println("----------");
    Serial.println("");
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

void Ubidots::readServerAnswer(char *response) {
  // Fills with zeros
  for (int i = 0; i <= MAX_BUFFER_SIZE; i++) {
    response[i] = '\0';
  }

  if (_debug) {
    Serial.println("----------");
    Serial.println("Server's response:");
  }

  int j = 0;

  while (_client_ubi.available()) {
    response[j] = (char) _client_ubi.read();
    if (_debug) {
      Serial.write(response[j]);
    }
    j++;
    if (j >= MAX_BUFFER_SIZE - 1) {
      break;
    }
  }

  if (_debug) {
    Serial.println("\n----------");
  }
}

float Ubidots::parseHttpAnswer(const char* request_type, char* data) {
  float result = ERROR_VALUE;
  // LV
  if (request_type == "LV") {
    char *parsed = (char *) malloc(sizeof(char) * 20);
    char *dst = (char *) malloc(sizeof(char) * 20);
    int len = strlen(data);  // Length of the answer char array from the server

    for (int i = 0; i < len - 2; i++) {
      if ((data[i] == '\r') && (data[i + 1] == '\n') && (data[i + 2] == '\r') &&
          (data[i + 3] == '\n')) {
        strncpy(parsed, data + i + 4, 20);  // Copies the result to the parsed
        parsed[20] = '\0';
        break;
      }
    }

    /* Extracts the the value */
    uint8_t index = 0;

    // Creates pointers to split the value
    char* pch = strchr(parsed, '\n');
    if (pch == NULL) {
      return result;
    }

    char* pch2 = strchr(pch + 1, '\n');

    if (pch2 == NULL) {
      return result;
    }

    index = (int)(pch2 - pch - 1);

    sprintf(dst, "%s", pch);
    dst[strlen(dst) - 1] = '\0';

    float result = atof(dst);

    free(dst);
    free(parsed);
    return result;
  }

  return ERROR_VALUE;
}

float Ubidots::parseTcpAnswer(const char* request_type, char* response) {
  int j = 0;

  if (_debug) {
    Serial.println("----------");
    Serial.println("Server's response:");
  }

  while (_client_ubi.available()) {
    char c = _client_ubi.read();
    if (_debug) {
      Serial.write(c);
    }
    response[j] = c;
    j++;
    if (j >= MAX_BUFFER_SIZE - 1) {
      break;
    }
  }

  if (_debug) {
    Serial.println("\n----------");
  }

  response[j] = '\0';
  float result = ERROR_VALUE;

  // POST
  if (request_type == "POST") {
    char *pch = strstr(response, "OK");
    if (pch != NULL) {
      result = 1;
    }
    return result;
  }

  // LV
  char* pch = strchr(response, '|');
  if (pch != NULL) {
    result = atof(pch + 1);
  }

  return result;
}

void Ubidots::setDebug(bool debug) {
  _debug = debug;
}