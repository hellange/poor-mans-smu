

#include "EasySMU2.h"

 float nowValueV = 2000.0;
 float nowValueI = 0.056;

 int8_t EasySMU2::fltSetCommitVoltageSource(float fVoltage) {
   nowValueV = fVoltage;
 }
 
 int8_t EasySMU2::fltSetCommitCurrentSource(float fCurrent, int8_t up_down_both) {
  nowValueI = fCurrent;                         
 }
 
 float EasySMU2::MeasureVoltage(){
  int r = random(2);
  if (r == 0) {
    nowValueV = nowValueV + random(100) / 1000000.0;
  } else if (r == 1) {
    nowValueV = nowValueV - random(100) / 1000000.0;
  }
  return nowValueV;
 }
 
 float EasySMU2::MeasureCurrent(){
  float r =  nowValueI +  random(0, 199) / 1000000.0;
  return r;
  //return 0.056123;
 }

    


