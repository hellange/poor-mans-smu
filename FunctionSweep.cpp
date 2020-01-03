#include "FunctionSweep.h"

#include <SPI.h>
#include "GD2.h"
#include "tags.h"
#include "colors.h"

FunctionSweepClass FUNCTION_SWEEP;

void FunctionSweepClass::init(ADCClass& smu_) {
  smu = smu_;
  currentSweepValue = 0.0;
  currentSweepDir = 1;
}

//TODO: Add open and close mechanism to initialize and cleanup when changing functions


void FunctionSweepClass::open(OPERATION_TYPE operationType_, void (*closedFn_)(int type)) {
  closedFn = closedFn_;
  pulseTimer = millis();
  operationType = operationType_;
}

void FunctionSweepClass::close() {
      closedFn(999); 
}

void FunctionSweepClass::handleButtonAction(int inputTag) {
}

void FunctionSweepClass::render(int x, int y) {


  // heading
  if (operationType == SOURCE_VOLTAGE) {
    GD.ColorRGB(COLOR_VOLT);
  } else {
    GD.ColorRGB(COLOR_CURRENT);
  }

  GD.ColorA(120);
  GD.cmd_text(x+20, y + 2 ,   29, 0, "SOURCE SWEEP");
  GD.cmd_text(x+20 + 1, y + 2 + 1 ,   29, 0, "SOURCE SWEEP");

 float high = 10.0;
  float low = -10.0;
  float step = 0.1;
  
  float duration = 5000;
  
if (operationType == SOURCE_VOLTAGE) {
  high = 1000.0;
  low = -1000.0;
  step = 100.0;
} else {
  
}
 
  GD.ColorA(255);
  y=y+30;

  if (high < 0) {
    GD.cmd_text(x+220, y ,  31, high, "-");
    x=x+20;
  }
  GD.cmd_number(x+242, y, 31, 4, abs(high));
  
  GD.cmd_text(x+345, y ,  31, 0, "to");
   if (low < 0) { 
    GD.cmd_text(x+385, y ,  31, 0, "-");
    x=x+20;
  }
  GD.cmd_number(x+385, y, 31, 4, abs(low));
  GD.cmd_text(x+490, y ,  31, 0, operationType == SOURCE_VOLTAGE ? "mV": "mA");
  y=y+50;

  GD.cmd_number(x+242, y, 31, 4, abs(step));
  GD.cmd_text(x+345, y ,  31, 0, "step");

  GD.__end();

  if (operationType == SOURCE_VOLTAGE) {
    operateSmuVoltage(high, low, step, duration);
  } else {
    operateSmuCurrent(high, low, step, duration);
  }
  GD.resume();
}


void FunctionSweepClass::operateSmuVoltage(float high, float low, float step, int duration)  {
     
   if (pulseTimer+duration/2 < millis()) {
     pulseTimer = millis();
     if (currentSweepDir == 1 && currentSweepValue >= high) {
      currentSweepDir = -1;
     } else if (currentSweepDir == -1 && currentSweepValue <= low) {
      currentSweepDir = +1;
     }
     currentSweepValue += step*currentSweepDir;
     smu.fltSetCommitVoltageSource(currentSweepValue, false);
   } 
}

void FunctionSweepClass::operateSmuCurrent(float high, float low, float step, int duration)  {
     
   if (pulseTimer+duration/2 < millis()) {
     pulseTimer = millis();
     if (currentSweepDir == 1 && currentSweepValue >= high) {
      currentSweepDir = -1;
     } else if (currentSweepDir == -1 && currentSweepValue <= low) {
      currentSweepDir = +1;
     }
     currentSweepValue += step*currentSweepDir;
     smu.fltSetCommitCurrentSource(currentSweepValue);
   } 
}
