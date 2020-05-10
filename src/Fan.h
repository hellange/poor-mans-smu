#include "Arduino.h"


class FanClass {


private:
  int timeSinceLastRPMReadout = millis();
  uint16_t getPWMFanRPM();
  static void fanSpeedInterruptHandler();
  int storedPWMFanSpeed;
public:
  void init();
  int getFanWidth();
  uint16_t getRPMValueFiltered();
};

extern FanClass FAN;
