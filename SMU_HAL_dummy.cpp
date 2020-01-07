/*********************************
 * SMU Hardware Abstraction Layer
 * 
 *        dummy for testing
 *********************************/

#include "SMU_HAL_dummy.h"


 
 int8_t SMU_HAL_dummy::fltSetCommitVoltageSource(float mv, bool dynamicRange) {
  Serial.println("Set voltage");

   setValuemV = mv;
   nowValuemV = mv;
       Serial.println(nowValuemV, 5);

   return nowValuemV;
 }

 void SMU_HAL_dummy::setCurrentRange(CURRENT_RANGE range) {
   Serial.println("WARNING: NOT IMPLEMENTED CURRENT RANGE IN SIMULATOR !");
 }
 
 void SMU_HAL_dummy::setSamplingRate(int value) {
   samplingDur = 1000/value;      
 }
  
 int8_t SMU_HAL_dummy::fltSetCommitCurrentSource(float fVoltage) {
   Serial.println("WARNING: NOT IMPLEMENTED CURRENT SOURCE IN SIMULATOR !");
   return 0;
 }
 
 
 int8_t SMU_HAL_dummy::fltSetCommitLimit(float fCurrent, int8_t up_down_both) {
  return setValueI = fCurrent;                         
 }

 int SMU_HAL_dummy::init() {
  lastSampleMilli = millis();
   return 0; 
 }

 int SMU_HAL_dummy::dataReady() {
  
  if (lastSampleMilli + (int)samplingDur > millis()){
    return -1;
  }
  lastSampleMilli = millis();
  
  if (volt_current == 0) {
    volt_current = 1;
  } else {
    volt_current = 0;
  }
  return volt_current;
 }
 
 float SMU_HAL_dummy::measureMilliVoltage(){

  int r = random(2);
  if (r == 0) {
    nowValuemV = nowValuemV + random(20) / 10000.0;
  } else if (r == 1) {
    nowValuemV = nowValuemV - random(20) / 10000.0;
  }
  float offset = 2.5; // simulate some mv offset
  
 
  return nowValuemV+ offset;  // return millivolt
 }
 
 float SMU_HAL_dummy::measureCurrent(int range){

  float simulatedLoad = 10.0; //ohm
  nowValueI = (nowValuemV) / simulatedLoad;

  // simulate noise
  nowValueI =  nowValueI + (random(0, 100) / 2000000.0);
  
  compliance = abs(setValueI) < abs(nowValueI/1000.0);
  return nowValueI;
 }

 float SMU_HAL_dummy::getSetValuemV(){
  return setValuemV * 1000.0;
 }

 float SMU_HAL_dummy::getLimitValue(){
  return setValueI * 1000.0;
 }

 

    
