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

float SettingsClass::max_current_1A_range() {
  //Serial.println(max_current_1a_range,3);
  return max_current_1a_range;
}
float SettingsClass::max_current_10mA_range() {
  return max_current_10ma_range;
}

int SettingsClass::getMaxTempAllowed() {
  return maxTempAllowed;
}

void SettingsClass::render() {


  
}
SettingsClass SETTINGS;
