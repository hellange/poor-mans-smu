#ifndef ETHERNET_H
#define ETHERNET_H

#include <NativeEthernet.h>

class EtnernetClass {


public:
 
  void setup();
  bool loop();
  void teensyMAC(uint8_t *mac);
  String localIp();
  int status = 42;
};
extern EtnernetClass ETHERNET_UTIL;

#endif
