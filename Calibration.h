

class CalibrationClass {

  private:
  unsigned long timeSinceLastChange;

  float set_adc[100] =  {-6000.00, -5000.00, -4000.00, -3000.00, -2500.00, -2000.00, -1000.00, -500.00, -100.00, 0.00, 50.00, 100.00, 250.00, 400.00, 500.00, 900.00, 1000.00, 1200.00, 1500.00, 2000.00, 2500.00, 3000.00, 3500.00, 4000.00, 4500.00, 5000.00, 6000.00, 7000.00};
  float meas_adc[100] = {-5999.46, -5000.00, -4000.00, -3001.18, -2500.00, -2000.01, -0999.70, -499.85, -099.87, 0.00, 50.04, 100.09, 250.20, 400.30, 500.25, 899.83,  999.85, 1199.86, 1500.10, 2000.20, 2499.95, 3000.60, 3502.70, 4002.50, 4501.10, 4997.00, 6000.00, 7000.00};
  int adc_cal_points = 27;

  float set_dac[100]  = {-5000.00, -4000.00, -3000.00, -2000.00, -1000.00, 0.00, 5.00, 50.00, 100.00, 500.00, 1000.00, 2000.00, 3000.00, 3500.00, 4000.00, 4200.00, 4500.00, 5000.00, 6000.00, 7000.00, 8000.00, 9000.00, 10000.00};
  // actual output
  float meas_dac[100] = {-5000.00, -3999.98, -2999.90, -2000.14, -1000.07, 0.00, 4.95, 49.95,  99.95, 500.00,  999.90, 1999.90, 3000.20, 3500.70, 4000.10, 4200.25, 4500.20, 5000.20, 6000.00, 7000.00, 8000.00, 9000.00, 10000.00};
  int dac_cal_points = 22;

public:
  float nullValue[2];
  bool useCalibratedValues = true;
  
  bool toggleCalibratedValues();
  float dac_nonlinear_compensation(float v);
  float adc_nonlinear_compensation(float milliVolt);
  void toggleNullValue(float v, int current_range);
  bool nullValueIsSet(int current_range);
  void renderCal(int x, int y, float valM, float setM, bool cur);
  void init();
};

extern CalibrationClass V_CALIBRATION;
extern CalibrationClass C_CALIBRATION;
