#include "UbiBuilder.h"
#include "UbiConstants.h"
#include "UbiHttp.h"
#include "UbiProtocol.h"
//#include "UbiTcp.h"
#include "UbiTypes.h"
//#include "UbiUdp.h"

UbiBuilder::UbiBuilder(const char* token, const char* host, IotProtocol iot_protocol) {
  _iot_protocol = iot_protocol;
  // command_list[UBI_TCP] = &builderTcp;
  command_list[UBI_HTTP] = &builderHttp;
  // command_list[UBI_UDP] = &builderUdp;
  _host = host;
  _token = token;
}

UbiProtocol* UbiBuilder::builder() {
  mapProtocol::iterator i = command_list.find(_iot_protocol);
  UbiProtocol* ubiBuilder = (i->second)();
  return ubiBuilder;
}
/*
UbiProtocol* builderTcp() {
  UbiProtocol* tcpInstance =
      new UbiTCP(_host, UBIDOTS_TCP_PORT, USER_AGENT, _token);
  return tcpInstance;
}
*/

UbiProtocol* builderHttp() {
  UbiProtocol* httpInstance = new UbiHttp(_host, _token);
  return httpInstance;
}

/*
UbiProtocol* builderUdp() {
  UbiProtocol* udpInstance =
      new UbiUDP(_host, UBIDOTS_TCP_PORT, USER_AGENT, _token);
  return udpInstance;
}
*/