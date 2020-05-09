#ifndef Settings_h
#define Settings_h

#include "SMU_HAL_717x.h"


class SettingsClass {

private:
  int maxTempAllowed = 80;

 
public:

  float setMilliVoltage = 1234.5;
  float setCurrentLimit = 33.0;

  float setMilliAmpere = 13.0;
  float setVoltageLimit = 5432.1;


  float MAX_CURRENT_10mA_RANGE = 8.0; // current values set because the ADC limit is 6 volt now...
  float MAX_CURRENT_1A_RANGE = 1300.0;
  void init();
  bool lowBandWidth = false;
  void setLowBandWidth(bool t);
  void render();
  int getMaxTempAllowed();
  

 
};

extern SettingsClass SETTINGS;

#endif Settings_h
