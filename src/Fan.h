#include "Arduino.h"


class FanClass {


private:
  int timeSinceLastRPMReadout = millis();
  uint16_t getPWMFanRPM();
  static void fanSpeedInterruptHandler();
  int storedPWMFanSpeed;
  int speedPercent = 0;
public:
  void setSpeed(int speedPercent);
  int getSpeed();
  void setAutoSpeedBasedOnTemperature(float temp);

  void init();
  int getFanWidth();
  uint16_t getRPMValueFiltered();
};

extern FanClass FAN;
