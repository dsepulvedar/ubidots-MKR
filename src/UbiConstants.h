#ifndef _UbiConstants_H_
#define _UbiConstants_H_

const char* UBIDOTS_SERVER = "industrial.api.ubidots.com";
const char* END_POINT = "/api/v1.6/devices/";
const char* USER_AGENT = "UbidotsArduinoMKR/1010";
const int UBIDOTS_HTTP_PORT = 80;
const int UBIDOTS_HTTPS_PORT = 443;
const int UBIDOTS_TCP_PORT = 9012;
const uint8_t MAX_VALUES = 10;
const uint8_t MAX_VALUE_LENGTH = 20;
const uint8_t MAX_DECIMAL_PLACES = 8;
const float ERROR_VALUE = -3.4028235E+8;
const int MAX_BUFFER_SIZE = 700;

#endif