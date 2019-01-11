/*********************************
 * SMU Hardware Abstraction Layer
 * 
 *        dummy for testing
 *********************************/

#include "SMU_HAL_dummy.h"

 float nowValueV = 0.0;
 float nowValueI = 0.0;
 float setValueI = 0.0;

 int8_t SMU_HAL_dummy::fltSetCommitVoltageSource(float fVoltage) {
   return nowValueV = fVoltage;
 }
 
 int8_t SMU_HAL_dummy::fltSetCommitCurrentSource(float fCurrent, int8_t up_down_both) {
  return setValueI = fCurrent;                         
 }
 
 float SMU_HAL_dummy::MeasureVoltage(){
  int r = random(2);
  if (r == 0) {
    nowValueV = nowValueV + random(50) / 1000000.0;
  } else if (r == 1) {
    nowValueV = nowValueV - random(50) / 1000000.0;
  }
  return nowValueV;
 }
 
 float SMU_HAL_dummy::MeasureCurrent(){

  float simulatedLoad = 10.0; //ohm
  nowValueI = nowValueV / simulatedLoad;
  
  nowValueI =  nowValueI +  nowValueI * (random(0, 10) / 1000.0); // 0.0% - 0.1% error
  return nowValueI;
 }

 boolean SMU_HAL_dummy::Overflow(){
   return setValueI < nowValueI;
 }
 

    


