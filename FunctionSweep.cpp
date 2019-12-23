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
void FunctionSweepClass::open(void (*closedFn_)(int type)) {
  closedFn = closedFn_;
  pulseTimer = millis();
}

void FunctionSweepClass::close() {
      closedFn(999); 
}

void FunctionSweepClass::handleButtonAction(int inputTag) {
}

void FunctionSweepClass::render(int x, int y) {

  float max = 2000.0;
  float min = -2000.0;
    // heading
      GD.ColorRGB(COLOR_VOLT);

  GD.ColorA(120);
  GD.cmd_text(x+20, y + 2 ,   29, 0, "SOURCE SWEEP");
  GD.cmd_text(x+20 + 1, y + 2 + 1 ,   29, 0, "SOURCE SWEEP");
  float high = 2000.0;
  float low = -2000.0;
  float step = 50.0;
  float duration = 100;
  GD.__end();
  operateSmu(high, low, step, duration);
  GD.resume();
}


void FunctionSweepClass::operateSmu(float high, float low, float step, int duration)  {
     
   if (pulseTimer+duration/2 < millis()) {
     pulseTimer = millis();
     if (currentSweepDir == 1 && currentSweepValue >= high) {
      currentSweepDir = -1;
     } else if (currentSweepDir == -1 && currentSweepValue <= low) {
      currentSweepDir = +1;
     }
     currentSweepValue += step*currentSweepDir;
     smu.fltSetCommitVoltageSource(currentSweepValue);
   } 
}
