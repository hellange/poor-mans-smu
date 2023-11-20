/*********************************
 * SMU Hardware Abstraction Layer
 * 
 *        dummy for testing
 *********************************/

#include "SMU_HAL_dummy.h"
#include "Debug.h"

//TODO: Use int64 instead of float for settings here (same as changed to in main SMU_HAL)
 
 int64_t SMU_HAL_dummy::fltSetCommitVoltageSource(int64_t voltage_uV, bool dynamicRange) {
  DEBUG.println("Set simulated voltage");
    operationType = SOURCE_VOLTAGE;

   setValue_micro = voltage_uV;
   //setValuemV = mv;
   //nowValuemV = mv;
   //DEBUG.println(nowValuemV, 5);

   return setValue_micro;
 }


CURRENT_RANGE SMU_HAL_dummy::getCurrentRange() {
  return current_range;
}

 void SMU_HAL_dummy::setCurrentRange(CURRENT_RANGE range, OPERATION_TYPE operationType) {
     current_range = range;

   DEBUG.println("WARNING: NOT IMPLEMENTED CURRENT RANGE IN SIMULATOR !");
 }

 void SMU_HAL_dummy::setSamplingRate(int value) {
   samplingDur = 1000/value;      
 }

  
 int SMU_HAL_dummy::getSamplingRate() {
  //TODO: Fix this
  DEBUG.println("ERROR: getSampling rate has not been implemented correctly !!!!");
  return 999;    
 }
  
 int64_t SMU_HAL_dummy::fltSetCommitCurrentSource(int64_t current_uA) {
   setValue_micro = current_uA;
     operationType = SOURCE_CURRENT;

   //DEBUG.println("WARNING: NOT IMPLEMENTED CURRENT SOURCE IN SIMULATOR !");
   return setValue_micro;
 }

 bool SMU_HAL_dummy::use100uVSetResolution() {
   if (DAC_RANGE_LOW == -10.0) {
     return false;
   } else {
     return true;
   }
 } 
 
 int64_t SMU_HAL_dummy::fltSetCommitCurrentLimit(int64_t current_uA) {
   setLimit_micro = current_uA;
   return setLimit_micro;
 }

 int64_t SMU_HAL_dummy::fltSetCommitVoltageLimit(int64_t voltage_uV) {
   setLimit_micro = voltage_uV;
   return setLimit_micro;
 }

 int SMU_HAL_dummy::init() {
   lastSampleMilli = millis();
   return 0; 
 }

 int SMU_HAL_dummy::dataReady() {
  
  // if (lastSampleMilli + (int)samplingDur > (int) millis()){
  //   return -1;
  // }
  // lastSampleMilli = millis();

  if (lastSampleMilli + 10000 > micros()) {
   return -1;
  }
  lastSampleMilli = micros();

  if (volt_current == 0) {
    volt_current = 1;
  } else {
    volt_current = 0;
  }
  return volt_current;
 }
 
 float SMU_HAL_dummy::measureMilliVoltage(){

  int inoise = 5 - random(9); // TODO: Find a better way to get random number from -x to x
  float noise = ((float)inoise)/1000.0; // use uV noise 

  // samplingDur gets lower as sampling speed increases.
  // simulate increasing noise for high sampling speeds
  noise = noise * (50.0 / (float)samplingDur);

  if (driftTimer + 10000 > (int)millis()) {
    int r = random(2);
    if (r==0){
      driftDirection = -1;
    } else {
      driftDirection = +1;
    }
    driftTimer = millis();
  }
  //drift = drift + (driftDirection*(float)random(10))/1000.0;
  
   
  float nowValuemV = 0.0;
  if (operationType == SOURCE_CURRENT) {
    nowValuemV = simulatedLoad * (setValue_micro / 1000.0);
    compliance = abs(setLimit_micro) < abs(nowValuemV*1000.0);
  } else {
    nowValuemV = setValue_micro / 1000.0;
  }

  // adjust for compliance that can have been set both by voltage measurement or current measurement
  if (compliance && operationType == SOURCE_VOLTAGE) {
    return simulatedLoad * setLimit_micro / 1000.0 + noise + simulatedOffset + drift;
  } else if (compliance && operationType == SOURCE_CURRENT) {
    return setLimit_micro/1000.0 + noise + simulatedOffset + drift;
  } else {
    return nowValuemV + noise + simulatedOffset + drift;
  }


 }

 void SMU_HAL_dummy::setGPIO(int nr, bool on) {
   DEBUG.println("ERROR: Not implemented setGPIO !!!!");
 }

 void SMU_HAL_dummy::disable_ADC_DAC_SPI_units(){
     //DEBUG.println("NOTE: Not implemented disable_ADC_DAC_SPI_units. Not relevant for simulator");

 }



 bool SMU_HAL_dummy::hasCompliance() {
  return compliance;
 }
 
 float SMU_HAL_dummy::measureCurrent(int range){

    float noise = random(1,10)/ 1000.0; // 10uA
    if (current_range == MILLIAMP10) {
      noise = noise / 500.0;
    }
    
    float nowValueI = 0;

    if (operationType == SOURCE_VOLTAGE) {
      nowValueI = (setValue_micro / 1000.0) / simulatedLoad;
      compliance = abs(setLimit_micro) < abs(nowValueI * 1000.0);
    } else {
      nowValueI = setValue_micro / 1000.0;
    }

    // adjust for compliance that can have been set both by voltage measurement or current measurement
    if (compliance && operationType == SOURCE_VOLTAGE) {
      return setLimit_micro / 1000.0 + noise;
    } else if (compliance && operationType == SOURCE_CURRENT) {
      return (setLimit_micro / 1000.0) / simulatedLoad + noise;
    } else {
      return nowValueI + noise;
    }
    
 }


 float SMU_HAL_dummy::getLimitValue(){
  return setValueI * 1000.0;
 }

  int64_t SMU_HAL_dummy::getSetValue_micro(){
  return setValue_micro;
 }

 int64_t SMU_HAL_dummy::getLimitValue_micro(){
   return setLimit_micro;
 }

 

    
