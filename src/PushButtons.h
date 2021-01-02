#include "Arduino.h"
#include <SPI.h>

class PushbuttonsClass {

  void (*callbackFn)(int button, bool quickPress, bool holdAfterLongPress, bool releaseAfterLongPress);


private:
  bool keyHeldLong = false;
  int buttonDetected =0;
  int buttonDetectedTimer = millis();
  int buttonFunction = 0;
  int buttonDepressed = 0;
  int prevButtonFunction = 0;
  int color = 0x0000ff;
  int keydownTimer = 0;
  int analogPin = 0;
  int holdPeriodms = 1000;
public:
  void init(int analogPin, int holdPeriodms);
  void handle();
  void setCallback(void (*callback)(int button, bool quickPress, bool holdAfterLongPress, bool releaseAfterLongPress));

};
