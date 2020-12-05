#ifndef DIGITUTILS_H
#define DIGITUTILS_H
#include "stdint.h"
class DigitUtilClass {

private:
  unsigned int timeSinceIndicated;
  int currentTag;

public:
  static const int typeVoltage = 1;
  static const int typeCurrent = 2;
  void separate(int *v, int *mv, int *uv, bool *neg, double rawMv);
  void separate2(int *v, int *mv, int *uv, int *nV, bool *neg, double rawMv);
  void renderValue(int x,int y,float val, int size, const int type);
  int blinkColor(int colorHigh, int colorLow, unsigned int period);

  //TODO: Move to separate class
  int indicateColor(int normalColor, int indicatorColor,int period, int tag) ;
  void startIndicator(int tag);

  //TODO: Candidate for separate class
  void print_uint64_t(uint64_t num);

  void displayTime(int millis, int x, int y);

};
extern DigitUtilClass DIGIT_UTIL;

#endif
