#ifndef _UbiBuilder_H_
#define _UbiBuilder_H_

#include <functional>
#include <map>
#include "UbiConstants.h"
#include "UbiProtocol.h"
#include "UbiTypes.h"

// UbiProtocol *builderTcp();
UbiProtocol* builderHttp();
// UbiProtocol *builderUdp();

typedef std::function<UbiProtocol*()> FunctionType;
typedef std::map<IotProtocol, FunctionType> mapProtocol;

namespace {
const char* _host;
const char* _token;
}  // namespace

class UbiBuilder {
 public:
  explicit UbiBuilder(const char* token, const char* host, IotProtocol iotProtocol);
  UbiProtocol* builder();

 private:
  IotProtocol _iot_protocol;
  mapProtocol command_list;
};

#endif