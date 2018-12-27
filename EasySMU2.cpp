

#include "EasySMU2.h"


 int8_t EasySMU2::fltSetCommitVoltageSource(float fVoltage) {
 }
 
 int8_t EasySMU2::fltSetCommitCurrentSource(float fCurrent, int8_t up_down_both) {                               
 }
 
 float EasySMU2::MeasureVoltage(){
  return 2 + random(0, 199) / 1000000.0;
 }
 
 float EasySMU2::MeasureCurrent(){
  float r =  56.0 +  random(0, 199) / 1000.0;
  return r/1000.0;
  //return 0.056123;
 }

    


