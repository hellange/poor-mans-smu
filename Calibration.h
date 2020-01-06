#include "operations.h"

class CalibrationClass {

  union cvt {
    float val;
    unsigned char b[4];
  };

  private:
  unsigned long timeSinceLastChange;

  float set_adc[100] =  {-10000.00, -6000.00, -5000.00, -4000.00, -3500-00, -3000.00, -2500.00, -2000.00, -1000.00, -500.00, -100.00, 0.00, 50.00, 100.00, 250.00, 400.00, 500.00, 900.00, 1000.00, 2000.00, 2500.00, 3000.00, 4000.00, 5000.00, 6000.00, 7000.00, 8000.00, 9000.00, 10000.00, 11000.00, 12000.00};
  float meas_adc[100] = {-10000.00, -5999.46, -5000.00, -4002.70, -3502.20, -3000.80, -2500.00, -2000.01, -0999.65, -500.10, -099.87, 0.00, 50.06, 100.16, 250.34, 400.54, 500.66, 900.95, 1000.95, 2000.68, 2501.31, 3001.89, 4002.48, 5003.02, 6005.36, 7009.10, 8010.84, 9008.52, 10000.00, 10992.71, 12002.12};
  int adc_cal_points = 31;

  float set_dac[100]  = {-11000.00, -10000.00, -5000.00, -4000.00, -3000.00, -2000.00, -1000.00, 0.00, 5.00, 50.00, 100.00, 500.00, 1000.00, 2000.00, 3000.00, 3500.00, 4000.00, 4200.00, 4500.00, 5000.00, 6000.00, 7000.00, 8000.00, 9000.00, 10000.00, 11000.00};
  // actual output
  float meas_dac[100] = {-11000.00, -10000.00, -5000.00, -3999.98, -2999.90, -2000.14, -1000.07, 0.00, 5.00, 49.99, 100.02, 500.00, 0999.98, 1999.97, 3000.00, 3500.20, 4000.20, 4200.24, 4500.21, 5000.16, 6000.15, 7000.13, 8000.08, 9000.05, 10000.00, 11000.00};
  int dac_cal_points = 26;
  
  float dacGainCompPos;
  float dacGainCompNeg;
  float adcGainCompPos;
  float adcGainCompNeg;
public:
  void floatToEeprom(int address, float f);
  float floatFromEeprom(int address);
  
  float getDacGainCompPos();
  void adjDacGainCompPos(float val);
  float getDacGainCompNeg();
  void adjDacGainCompNeg(float val);

  float getAdcGainCompPos();
  void adjAdcGainCompPos(float val);
  float getAdcGainCompNeg();
  void adjAdcGainCompNeg(float val);
  
  float nullValue[2];
  bool useCalibratedValues = true;
  
  bool toggleCalibratedValues();
  float dac_nonlinear_compensation(float v);
  float adc_nonlinear_compensation(float milliVolt);
  void toggleNullValue(float v, CURRENT_RANGE current_range);
  bool nullValueIsSet(CURRENT_RANGE current_range);
  void renderCal(int x, int y, float valM, float setM, bool cur, bool reduceDetails);
  void init();
};

extern CalibrationClass V_CALIBRATION;
extern CalibrationClass C_CALIBRATION;
