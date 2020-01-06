#include "operations.h"

class CalibrationClass {

  union cvt {
    float val;
    unsigned char b[4];
  };

  private:
  OPERATION_TYPE operationType;
  unsigned long timeSinceLastChange;

  float set_adc[100] =  {-12000.00, -11000.00, -10000.00, -9000.00, -8000.00, -7000.00, -6000.00, -5000.00, -4000.00, -3500-00, -3000.00, -2500.00, -2000.00, -1000.00, -500.00, -100.00, -50.00, 0.00, 50.00, 100.00, 250.00, 400.00, 500.00, 900.00, 1000.00, 2000.00, 2500.00, 3000.00, 4000.00, 5000.00, 6000.00, 7000.00, 8000.00, 9000.00, 10000.00, 11000.00, 12000.00};
  float meas_adc[100] = {-11992.94, -10996.74, -10000.00, -8999.41, -7998.84, -6998.41, -5996.70, -4995.87, -3996.70, -3497.11, -2997.69, -2497.78, -1997.91, -0999.54, -499.96, -100.01, -50.00, 0.00, 50.06, 100.16, 250.34, 400.54, 500.66, 900.95, 1000.95, 2000.68, 2501.31, 3001.89, 4002.48, 5003.02, 6005.36, 7009.10, 8010.84, 9008.52, 10000.00, 10992.71, 12002.12};
  int adc_cal_points = 37;

  float set_dac[100]  = {-11000.00, -10000.00, -5000.00, -4000.00, -3000.00, -2000.00, -1000.00, 0.00, 5.00, 50.00, 100.00, 500.00, 1000.00, 2000.00, 3000.00, 3500.00, 4000.00, 4200.00, 4500.00, 5000.00, 6000.00, 7000.00, 8000.00, 9000.00, 10000.00, 11000.00};
  // actual output
  float meas_dac[100] = {-11000.00, -10000.00, -5000.00, -3999.98, -2999.90, -2000.14, -1000.07, 0.00, 5.00, 49.99, 100.02, 500.00, 0999.98, 1999.97, 3000.00, 3500.20, 4000.08, 4200.06, 4500.05, 5000.04, 6000.01, 7000.17, 8000.12, 9000.10, 10000.00, 11000.00};
  int dac_cal_points = 26;
  
  float dacGainCompPos;
  float dacGainCompNeg;
  float adcGainCompPos;
  float adcGainCompNeg;

  int ea_dac_gain_comp_pos;
  int ea_dac_gain_comp_neg;
  int ea_adc_gain_comp_pos;
  int ea_adc_gain_comp_neg;
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
  void init(OPERATION_TYPE operation_type);
};

extern CalibrationClass V_CALIBRATION;
extern CalibrationClass C_CALIBRATION;
