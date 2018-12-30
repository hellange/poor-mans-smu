

#include "EasySMU2.h"

 float nowValue = 2.0;

 int8_t EasySMU2::fltSetCommitVoltageSource(float fVoltage) {
 }
 
 int8_t EasySMU2::fltSetCommitCurrentSource(float fCurrent, int8_t up_down_both) {                               
 }
 
 float EasySMU2::MeasureVoltage(){
  int r = random(2);
  if (r == 0) {
    nowValue = nowValue + random(100) / 1000000.0;
  } else if (r == 1) {
    nowValue = nowValue - random(100) / 1000000.0;
  }
  return nowValue;
 }
 
 float EasySMU2::MeasureCurrent(){
  float r =  56.0 +  random(0, 199) / 1000.0;
  return r/1000.0;
  //return 0.056123;
 }

    


