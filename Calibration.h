
#include "operations.h"
#include "Filters.h"


  
class CalibrationClass {

  union cvt {
    float val;
    unsigned char b[4];
  };

  private:
  
  FiltersClass *FILTERS;
  OPERATION_TYPE operationType;
  unsigned long timeSinceLastChange;

  float set_adc[200]; 
  float meas_adc[200];
  int adc_cal_points=0;

  float set_dac[100];
  float meas_dac[100];
  int dac_cal_points=0;
  
  float dacGainCompPos;
  float dacGainCompNeg;
  float adcGainCompPos;
  float adcGainCompNeg;

  float dacZeroComp;

  int ea_dac_gain_comp_pos;
  int ea_dac_gain_comp_neg;
  int ea_adc_gain_comp_pos;
  int ea_adc_gain_comp_neg;
  int ea_adc_nonlinear_comp_nr;
  int ea_adc_nonlinear_comp_start;

  int ea_dac_zero_comp;

  
  int ea_adc_zero_comp_vol;
  int ea_adc_zero_comp_cur;
  int ea_adc_zero_comp_cur2;

  
  void readAdcCalFromEeprom();
  void writeAdcCalToEeprom(int nrOfPoints);
public:

  bool autoCalInProgress = false;

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

  float getDacZeroComp();
  void adjDacZeroComp(float val);

  // using static to share among instances. TODO: Fix this mess
  static float nullValueVol[2]; 
  static float nullValueCur[2];

  
  float relativeValue[2];
  bool useCalibratedValues = true;
  bool useDacCalibratedValues = true;

  bool autoCalDone = false;


  
  bool toggleCalibratedValues();
  float dac_nonlinear_compensation(float v);
  float adc_nonlinear_compensation(float milliVolt);
  void setNullValueVol(float v, CURRENT_RANGE current_range);
  void setNullValueCur(float v, CURRENT_RANGE current_range);

  void toggleRelativeValue(float v, CURRENT_RANGE current_range);
  bool nullValueIsSet(CURRENT_RANGE current_range);
  bool relativeValueIsSet(CURRENT_RANGE current_range);

  void renderCal(int x, int y, float valM, float setM, bool cur, bool reduceDetails);
  void init(OPERATION_TYPE operation_type);

  void autoCalADCfromDAC();
  void startAutoCal();
};

extern CalibrationClass V_CALIBRATION;
extern CalibrationClass C_CALIBRATION;
