class DigitUtilClass {
private:
public:
  static const int typeVoltage = 1;
  static const int typeCurrent = 2;
  void separate(int *v, int *mv, int *uv, bool *neg, float rawMv);
  void renderValue(int x,int y,float val, int size, const int type);

};
extern DigitUtilClass DIGIT_UTIL;

