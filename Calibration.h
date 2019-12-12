

class CalibrationClass {

  private:
  unsigned long timeSinceLastChange;

  float set_adc[100] =  {-6000.00, -5000.00, -4000.00, -3000.00, -2000.00, -1000.00, -500.00, -100.00, 0.00, 50.00, 100.00, 500.00, 900.00, 1000.00, 1200.00, 1500.00, 2000.00, 2500.00, 3000.00, 3500.00, 4000.00, 4500.00, 5000.00, 5500.00, 5700.00, 6000.00};
  float meas_adc[100] = {-5999.46, -4999.58, -3999.83, -2999.11, -1998.70, -0999.70, -499.85, -099.87, 0.00, 50.04, 100.09, 500.07, 899.73,  999.45, 1199.46, 1499.80, 1999.82, 2499.85, 3000.60, 3502.70, 4002.50, 4501.50, 4998.00, 5494.60, 5694.50, 5998.30};
  int adc_cal_points = 25;

  float set_dac[100]  = {-3000.00, -2000.00, -1000.00, 0.00, 5.00, 50.00, 100.00, 500.00, 1000.00, 2000.00, 3000.00, 3500.00, 4000.00, 4200.00, 4500.00, 5000.00, 6000.00, 7000.00, 8000.00, 9000.00, 10000.00};
  // actual output
  float meas_dac[100] = {-3000.00, -1999.86, -1000.80, 0.00, 4.95, 49.95,  99.95, 500.12, 1000.32, 2000.70, 3001.30, 3501.50, 4001.70, 4201.85, 4501.90, 5002.10, 6000.00, 7000.00, 8000.00, 9000.00, 10000.00};
  int dac_cal_points = 20;

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
