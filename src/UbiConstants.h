#ifndef _UbiConstants_H_
#define _UbiConstants_H_

#include "UbiTypes.h"
#include <stdint.h>

const char* UBIDOTS_SERVER = "industrial.api.ubidots.com";
const char* END_POINT = "/api/v1.6/devices/";
const char* USER_AGENT = "UbidotsArduinoMKR/1010";
const int UBIDOTS_HTTP_PORT = 443;
const int UBIDOTS_TCP_PORT = 9812;
const uint8_t MAX_VALUES = 10;
const uint8_t MAX_VALUE_LENGTH = 20;
const uint8_t MAX_DECIMAL_PLACES = 8;
const float ERROR_VALUE = -3.4028235E+8;
const int MAX_BUFFER_SIZE = 700;
static UbiServer UBI_INDUSTRIAL = "industrial.api.ubidots.com";
static UbiServer UBI_EDUCATIONAL = "things.ubidots.com";
const char* DEFAULT_DEVICE_LABEL = "arduino-mkr1010";
const char* DEFAULT_DEVICE_NAME = "Arduino MKR1010";

#endif