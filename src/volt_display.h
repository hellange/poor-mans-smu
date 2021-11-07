#ifndef VOLTDISPLAY_H
#define VOLTDISPLAY_H

#include <stdint.h>

class VoltDisplayClass {
public:
  void renderMeasured(int x, int y, float rawMv, bool compliance);
  void renderSet(int x, int y, int64_t raw_uV);
  void boldText(int x, int y, const char *text);
  void boldNumber(int x, int y, int digits, int number);
 // void separate(int *v, int *mv, int *uv, bool *neg, float raw);

private:
  char sign[2] = "+";
};

extern VoltDisplayClass VOLT_DISPLAY;
#endif