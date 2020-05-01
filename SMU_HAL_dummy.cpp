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

 void SMU_HAL_dummy::setCurrentRange(CURRENT_RANGE range, OPERATION_TYPE operationType) {
   Serial.println("WARNING: NOT IMPLEMENTED CURRENT RANGE IN SIMULATOR !");
 }

 void SMU_HAL_dummy::setSamplingRate(int value) {
   samplingDur = 1000/value;      
 }

  
 int SMU_HAL_dummy::getSamplingRate() {
  //TODO: Fix this
  Serial.println("ERROR: getSampling rate has not been implemented correctly !!!!");
  return 999;    
 }
  
 int8_t SMU_HAL_dummy::fltSetCommitCurrentSource(float fVoltage) {
   Serial.println("WARNING: NOT IMPLEMENTED CURRENT SOURCE IN SIMULATOR !");
   return 0;
 }

 bool SMU_HAL_dummy::use100uVSetResolution() {
   Serial.println("NOTE: NOT IMPLEMENTED use100uVSetResolution IN SIMULATOR !");
 } 
 
 int8_t SMU_HAL_dummy::fltSetCommitCurrentLimit(float fCurrent, int8_t up_down_both) {
  return setValueI = fCurrent;                         
 }

 int8_t SMU_HAL_dummy::fltSetCommitVoltageLimit(float fCurrent, int8_t up_down_both) {
  //TODO: Not handling current here ?
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

  int inoise = 5 - random(9); // TODO: Find a better way to get random number from -x to x
  
  float noise = ((float)inoise)/1000.0; // use uV noise 

  // samplingDur gets lower as sampling speed increases.
  // simulate increasing noise for high sampling speeds
  noise = noise * (500.0 / (float)samplingDur);

  if (driftTimer + 10000 > millis()) {
    int r = random(2);
    if (r==0){
      driftDirection = -1;
    } else {
      driftDirection = +1;
    }
    driftTimer = millis();
  }
  //drift = drift + (driftDirection*(float)random(10))/1000.0;
  
  float offset = 2.5; // simulate some mv offset
  
 
  return nowValuemV+ offset + noise + drift;  // return millivolt
 }

 void SMU_HAL_dummy::setGPIO(int nr, bool on) {
   Serial.println("ERROR: Not implemented setGPIO !!!!");
 }

 void SMU_HAL_dummy::disable_ADC_DAC_SPI_units(){
     Serial.println("NOTE: Not implemented disable_ADC_DAC_SPI_units. Not relevant for simulator");

 }



 bool SMU_HAL_dummy::hasCompliance() {
  return compliance;
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
  return setValuemV;// * 1000.0;
 }

 float SMU_HAL_dummy::getLimitValue(){
  return setValueI * 1000.0;
 }

 

    
