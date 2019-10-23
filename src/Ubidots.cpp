#include "Ubidots.h"
#include "UbiConstants.h"
#include "UbiProtocol.h"
#include "UbiTypes.h"
#include "WiFiNINA.h"

/**************************************************************************
 * Overloaded constructors
 ***************************************************************************/

Ubidots::Ubidots(const char* token) { Ubidots(token, UBI_INDUSTRIAL, UBI_TCP); }

Ubidots::Ubidots(const char* token, UbiServer server) { Ubidots(token, server, UBI_TCP); }

Ubidots::Ubidots(const char* token, UbiServer server, IotProtocol iotProtocol) {
  _cloudProtocol = new UbiProtocolHandler(token, server, iotProtocol);
}

/**************************************************************************
 * Destructor
 ***************************************************************************/

Ubidots::~Ubidots() { delete _cloudProtocol; }

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

void Ubidots::add(char* variable_label, float value) { add(variable_label, value, NULL, NULL, NULL); }

void Ubidots::add(char* variable_label, float value, char* context) { add(variable_label, value, context, NULL, NULL); }

void Ubidots::add(char* variable_label, float value, char* context, long unsigned dot_timestamp_seconds) {
  add(variable_label, value, context, dot_timestamp_seconds, NULL);
}

void Ubidots::add(char* variable_label, float value, char* context, long unsigned dot_timestamp_seconds,
                  unsigned int dot_timestamp_millis) {
  _cloudProtocol->add(variable_label, value, context, dot_timestamp_seconds, dot_timestamp_millis);
}

/**
 * Sends data to Ubidots
 * @arg device_label [Mandatory] device label where the dot will be stored
 * @arg device_name [optional] Name of the device to be created (supported only
 * for TCP/UDP)
 * @arg flags [Optional] Particle publish flags for webhooks
 */

bool Ubidots::send() { return send(DEFAULT_DEVICE_LABEL, DEFAULT_DEVICE_NAME); }

bool Ubidots::send(const char* device_label) { return send(device_label, DEFAULT_DEVICE_NAME); }

bool Ubidots::send(const char* device_label, const char* device_name) {
  return _cloudProtocol->send(device_label, device_name);
}

float Ubidots::get(const char* device_label, const char* variable_label) {
  return _cloudProtocol->get(device_label, variable_label);
}

void Ubidots::setDebug(bool debug) {
  _debug = debug;
  _cloudProtocol->setDebug(debug);
}

/*
 * Adds to the context structure values to retrieve later it easily by the user
 */

void Ubidots::addContext(char* key_label, char* key_value) { _cloudProtocol->addContext(key_label, key_value); }

/*
 * Retrieves the actual stored context properly formatted
 */
void Ubidots::getContext(char* context_result) { _cloudProtocol->getContext(context_result); }

void Ubidots::init(const char* ssid, const char* password) {
  Serial.println("Attempting to connect to WPA network...");
  Serial.print("SSID: ");
  Serial.println(ssid);
  while (_connect_attempts < 5) {
    _status = WiFi.begin(ssid, password);
    if (_status == WL_CONNECTED) {
      if (_debug) {
        printWiFiStatus();
      }
      _connect_attempts = 0;
      return;
    }
    _connect_attempts++;
    delay(100);
  }
  Serial.print("Could not connect to WiFi network after 5 attempts");
}

void Ubidots::printWiFiStatus() {
  // print the SSID of the network you're connected to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}