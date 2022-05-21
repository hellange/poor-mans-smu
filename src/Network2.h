#ifndef NETWORK2_H
#define NETWORK2_H

#include "QNEthernet.h"
#include "Filters.h"
#include "Vrekrer_scpi_parser.h" // TESTING OUT SCPI LIBRARY, https://github.com/Vrekrer/Vrekrer_scpi_parser
#include "Debug.h"

class Network2Class {


 
public: 

  // Reference to the first ethernet client
  qindesign::network::EthernetClient clientNow;

  int receivedMessages = 0;
  char buffer[100];
  void setup();
  void loop();
  char ipAddressString[18];
  bool linkState = 0;
  bool hasIpx = false;
  char* GetEthMsg();
  bool newMessageReady();
  void clearBuffer();
 };
extern Network2Class ETHERNET2_UTIL;

#endif
