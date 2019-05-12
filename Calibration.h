

class CalibrationClass {
  private:
  unsigned long timeSinceLastChange;
  float set_adc[100]  = {0.0, 1.0, 50.00, 500.00, 1000.00, 1100.00, 1300.00, 1400.00, 1500.00, 1600.00, 1700.00, 2000.00};
  float meas_adc[100]  = {0.0, 0.960, 49.90, 499.990, 1000.10, 1100.145, 1300.135, 1400.120, 1500.125, 1600.04, 1700.03, 1999.90};

 
public:
  float nullValue;

  float adjust(float v);
  void setNullValue(float v);
  void renderCal(int x, int y, float valM, float setM, bool cur);
  void init();
};
