/*********************************
 * SMU Hardware Abstraction Layer
 * 
 *        dummy for testing
 *********************************/

#include "SMU_HAL_dummy.h"


 
 int8_t SMU_HAL_dummy::fltSetCommitVoltageSource(float fVoltage) {
   setValueV = fVoltage;
   nowValueV = fVoltage;
   return nowValueV;
 }
 
 int8_t SMU_HAL_dummy::fltSetCommitCurrentSource(float fCurrent, int8_t up_down_both) {
  return setValueI = fCurrent;                         
 }

 int SMU_HAL_dummy::init() {
  lastSampleMilli = millis();
   return 0; 
 }
 
 int SMU_HAL_dummy::dataReady() {
  if (lastSampleMilli + 100 < millis()) {
    lastSampleMilli = millis();
    return true;
  }
  return false;
 }
 
 float SMU_HAL_dummy::measureMilliVoltage(){

  int r = random(2);
  if (r == 0) {
    nowValueV = nowValueV + random(20) / 10000000.0;
  } else if (r == 1) {
    nowValueV = nowValueV - random(20) / 10000000.0;
  }
  return nowValueV * 1000-0;  // return millivolt
 }
 
 float SMU_HAL_dummy::measureCurrent(){

  float simulatedLoad = 10.0; //ohm
  nowValueI = nowValueV / simulatedLoad;
  
  nowValueI =  nowValueI + (random(0, 100) / 2000000.0);
  return nowValueI;
 }

 boolean SMU_HAL_dummy::compliance(){
   return abs(setValueI) < abs(nowValueI);
 }

 float SMU_HAL_dummy::getSetValuemV(){
  return setValueV * 1000.0;
 }

 float SMU_HAL_dummy::getSetValuemA(){
  return setValueI * 1000.0;
 }

 

    
