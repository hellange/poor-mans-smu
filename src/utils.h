#ifndef UTILS_H
#define UTILS_H

#include "Wire.h"
class UtilsClass {
private:
  float meanValueLM60;
  unsigned long sampleTimer = millis();
public:
  int TC74_getTemperature();
  float LM60_getTemperature(int analogPin);
};
extern UtilsClass UTILS;

#endif
