#ifndef _UbiProtocol_H_
#define _UbiProtocol_H_

#include "UbiTypes.h"

class UbiProtocol {
 public:
  virtual void add(char* variable_label, float value, char* context, unsigned long dot_timestamp_seconds,
                   unsigned int dot_timestamp_millis);
  void addContext(char* key_label, char* key_value);
  void getContext(char* context_result);
  virtual bool sendData(const char* device_label, const char* device_name) = 0;
  virtual float get(const char* device_label, const char* variable_label) = 0;
  virtual void setDebug(bool debug) = 0;
};

#endif