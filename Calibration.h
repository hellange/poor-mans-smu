

class CalibrationClass {
  private:
  unsigned long timeSinceLastChange;
  float set_adc[100]  =  {0.0, 5.00, 10.0, 50.00, 500.00, 1000.00, 1100.00,  1300.00,  1400.00, 1500.00, 1600.00, 1700.00,  2000.00};
  float meas_adc[100]  = {0.0, 4.94, 9.95, 49.94, 499.98, 1000.10, 1100.130, 1300.140, 1400.10, 1500.10, 1600.05, 1700.030, 1999.93};
  int adc_cal_points = 13;
public:
  float nullValue;
  bool useCalibratedValues = true;

  bool toggleCalibratedValues();
  float adjust(float v);
  void toggleNullValue(float v);
  void renderCal(int x, int y, float valM, float setM, bool cur);
  void init();
};
