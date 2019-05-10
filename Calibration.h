

class CalibrationClass {
  private:
  unsigned long timeSinceLastChange;
public:
  float nullValue;

  float adjust(float v);
  void setNullValue(float v);
  void renderCal(int x, int y, float valM, float setM, bool cur);
  void init();
};
