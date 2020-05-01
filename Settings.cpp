#include "Settings.h"

extern ADCClass SMU[];

void SettingsClass::init() {
 lowBandWidth = true;
 SMU[0].setGPIO(1,true);
}

void SettingsClass::setLowBandWidth(bool t) {
  lowBandWidth = t;
  SMU[0].setGPIO(1,t); // gpio1:  true = low bandwidth
}

void SettingsClass::render() {


  
}
SettingsClass SETTINGS;
