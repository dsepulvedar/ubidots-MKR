#ifndef _UbiTypes_H_
#define _UbiTypes_H_

typedef struct Dot {
  char *variable_label;
  char *dot_context;
  float dot_value;
  unsigned long dot_timestamp_seconds;
  unsigned int dot_timestamp_millis;
} Dot;

typedef struct ContextUbi {
  char *key_label;
  char *key_value;
} ContextUbi;

typedef const char *UbiServer;

typedef enum { UBI_HTTP, UBI_TCP, UBI_UDP } IotProtocol;

#endif