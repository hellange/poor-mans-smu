#ifndef DIGITUTILS_H
#define DIGITUTILS_H

class DigitUtilClass {

private:
  unsigned int timeSinceIndicated;
  int currentTag;

public:
  static const int typeVoltage = 1;
  static const int typeCurrent = 2;
  void separate(int *v, int *mv, int *uv, bool *neg, float rawMv);
  void separate2(int *v, int *mv, int *uv, int *nV, bool *neg, float rawMv);
  void renderValue(int x,int y,float val, int size, const int type);
  int blinkColor(int colorHigh, int colorLow, unsigned int period);

  //TODO: Move to separate class
  int indicateColor(int normalColor, int indicatorColor,int period, int tag) ;
  void startIndicator(int tag);
};
extern DigitUtilClass DIGIT_UTIL;

#endif
