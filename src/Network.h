#ifndef ETHERNET_H
#define ETHERNET_H
class EtnernetClass {


public:
 
  void setup();
  bool loop();
  void httpRequest();
  void teensyMAC(uint8_t *mac);
  int status;
};
extern EtnernetClass ETHERNET_UTIL;

#endif
