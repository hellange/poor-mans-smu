#ifndef CURRENT_DISPLAY_H
#define CURRENT_DISPLAY_H

#include "operations.h"
#include <stdint.h>

class CurrentDisplayClass {
public:
  void renderMeasured(int x, int y, float rawMa, bool compliance, bool show_nA, CURRENT_RANGE current_range, bool reduceDetails);
  void renderOverflowSW(int x, int y);
  void renderSet(int x, int y, int64_t raw_uA, CURRENT_RANGE current_range);
  void boldText(int x, int y, const char *text);
  void boldNumber(int x, int y, int digits, int number);
private:
  char sign[2] = "+";
};

extern CurrentDisplayClass CURRENT_DISPLAY;
#endif