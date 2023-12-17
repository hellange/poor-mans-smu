#include "Settings.h"
#include "Debug.h"

//extern ADCClass SMU[];

void SettingsClass::init(SMU_HAL &SMU) {
  SMU1 = &SMU;
 lowBandWidth = false;
 setLowBandWidth(lowBandWidth);
}

void SettingsClass::setLowBandWidth(bool t) {
  lowBandWidth = t;
  DEBUG.print("Set GPIP 1 ");
  DEBUG.println(t);
  SMU1->setGPIO(1,t); // gpio1:  true = low bandwidth
}

float SettingsClass::max_current_1A_range() {
  //DEBUG.print("max_current 1A:");
  //DEBUG.println(max_current_1a_range,3);
  return max_current_1a_range;
}
float SettingsClass::max_current_10mA_range() {
  //DEBUG.print("max_current 10mA:");
  //DEBUG.println(max_current_1a_range,3);
  return max_current_10ma_range;
}

int SettingsClass::getMaxTempAllowed() {
  return maxTempAllowed;
}

void SettingsClass::render() {


  
}
SettingsClass SETTINGS;
