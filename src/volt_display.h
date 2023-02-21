#ifndef VOLTDISPLAY_H
#define VOLTDISPLAY_H

#include <stdint.h>

class VoltDisplayClass {
public:
  void renderMeasured(int x, int y, float rawMv, bool compliance, bool reduceDetails);
  void renderMeasuredResistance(int x, int y, float rawMv, float rawMa, bool compliance);

  void renderSet(int x, int y, int64_t raw_uV);
  void boldText(int x, int y, const char *text);
  void boldNumber(int x, int y, int digits, int number);

private:
};

extern VoltDisplayClass VOLT_DISPLAY;
#endif