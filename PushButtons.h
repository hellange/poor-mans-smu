#include "Arduino.h"
#include <SPI.h>


class PushbuttonsClass {

private:
  int buttonDetected =0;
  int buttonDetectedTimer = millis();
  int buttonFunction = 0;
  int buttonDepressed = 0;
  int prevButtonFunction = 0;
  int color = 0x0000ff;
  int keydownTimer = 0;
public:
 
  void init();
  void handle();
 
};

extern PushbuttonsClass PUSHBUTTONS;
