#ifndef UTILS_H
#define UTILS_H

#include "Wire.h"
class UtilsClass {
public:
  int TC74_getTemperature();
  int LM60_getTemperature(int analogPin);
};
extern UtilsClass UTILS;

#endif
