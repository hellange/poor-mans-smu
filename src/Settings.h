#ifndef Settings_h
#define Settings_h

#include "SMU_HAL_717x.h"


class SettingsClass {

private:
  int maxTempAllowed = 80;
  float max_current_10ma_range = 10.0;
  float max_current_1a_range = 1300.0;
 
public:
 
  float setMilliVoltage = 1234.5;
  float setCurrentLimit = 33.0;

  float setMilliAmpere = 13.0;
  float setVoltageLimit = 5432.1;

  float max_current_1A_range();
  float max_current_10mA_range();


  void init();
  bool lowBandWidth = false;
  void setLowBandWidth(bool t);
  void render();
  int getMaxTempAllowed();
  

 
};

extern SettingsClass SETTINGS;

#endif
