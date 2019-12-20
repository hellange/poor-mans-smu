#include "Arduino.h"


class FanClass {


private:

public:
  void init();
  int getFanWidth();
  static void fanSpeedInterruptHandler();
  uint16_t getPWMFanRPM();
 
};

extern FanClass FAN;
