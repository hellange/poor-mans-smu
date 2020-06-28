#include "Settings.h"

extern ADCClass SMU[];

void SettingsClass::init() {
 lowBandWidth = false;
 setLowBandWidth(lowBandWidth);
}

void SettingsClass::setLowBandWidth(bool t) {
  lowBandWidth = t;
  Serial.print("Set GPIP 1 ");
  Serial.println(t);
  SMU[0].setGPIO(1,t); // gpio1:  true = low bandwidth
}

int SettingsClass::getMaxTempAllowed() {
  return maxTempAllowed;
}

void SettingsClass::render() {


  
}
SettingsClass SETTINGS;
