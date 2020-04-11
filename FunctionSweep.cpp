#include "FunctionSweep.h"

#include <SPI.h>
#include "GD2.h"
#include "tags.h"
#include "colors.h"

FunctionSweepClass FUNCTION_SWEEP;


extern ADCClass SMU[];

void FunctionSweepClass::init(/*ADCClass& smu_*/) {
  //smu = smu_;
  currentSweepValue = 0.0;
  currentSweepDir = 1;
}

//TODO: Add open and close mechanism to initialize and cleanup when changing functions


void FunctionSweepClass::open(OPERATION_TYPE operationType_, void (*closedFn_)(OPERATION_TYPE type)) {
  closedFn = closedFn_;
  pulseTimer = millis();
  operationType = operationType_;
}

void FunctionSweepClass::close() {
      closedFn(operationType); 
}




int sinceLastPress2 = millis();
void FunctionSweepClass::handleButtonAction(int inputTag) {

  float buttonStep;

  buttonStep = operationType == SOURCE_CURRENT ? 1.0 : 100.0;

  if (sinceLastPress2 + 100 < millis()) {
    if (inputTag == SWEEP_BUTTON_INC) {
       duration = duration +100;
    } else if (inputTag == SWEEP_BUTTON_DEC) {
       duration = duration -100;
    } else if (inputTag == SWEEP_BUTTON_STEP_INC) {
       step = step + 10;
    } else if (inputTag == SWEEP_BUTTON_STEP_DEC) {
       step = step - 10;
    } else if (inputTag == SWEEP_BUTTON_VOLT_INC) {
       high = high + 100;
       low = low - 100;
    } else if (inputTag == SWEEP_BUTTON_VOLT_DEC) {
       high = high -100;
       low = low + 100;
    } else if (inputTag == SWEEP_BUTTON_CLEAR) {
     currentSweepValue = 0.0;
     currentSweepDir = 1; 
    }

    
    sinceLastPress2 =millis();
  }
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

 
  
  
if (operationType == SOURCE_VOLTAGE) {
  //high = 1000.0;
  //low = -1000.0;
  //step = 100.0;
} else {
  
}
 
  GD.ColorA(255);
  y=y+40;

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


   GD.Tag(SWEEP_BUTTON_VOLT_DEC);
  GD.cmd_button(x+20,y,80,40,28,0,"Dec");
  GD.Tag(0);
  
  GD.Tag(SWEEP_BUTTON_VOLT_INC);
  GD.cmd_button(x+120,y,80,40,28,0,"Inc");
  GD.Tag(0);

  
  y=y+50;

  GD.cmd_number(x+242, y, 31, 4, abs(step));
  GD.cmd_text(x+345, y ,  31, 0, "step");

  GD.Tag(SWEEP_BUTTON_STEP_DEC);
  GD.cmd_button(x+20,y,80,40,28,0,"Dec");
  GD.Tag(0);
  
  GD.Tag(SWEEP_BUTTON_STEP_INC);
  GD.cmd_button(x+120,y,80,40,28,0,"Inc");
  GD.Tag(0);
  
  
  y=y+50;
  GD.cmd_number(x+242, y, 31, 4, abs(duration));
  GD.cmd_text(x+345, y ,  31, 0, "duration");


   GD.ColorRGB(0x000000);

  GD.Tag(SWEEP_BUTTON_DEC);
  GD.cmd_button(x+20,y,80,40,28,0,"Dec");
  GD.Tag(0);
  
  GD.Tag(SWEEP_BUTTON_INC);
  GD.cmd_button(x+120,y,80,40,28,0,"Inc");
  GD.Tag(0);


  GD.Tag(SWEEP_BUTTON_CLEAR);
  GD.cmd_button(x+600,y,80,40,28,0,"Clear");
  GD.Tag(0);


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
     // Note that DAC is calibrated based on dynamic range. If all swipe values
     // are within a DAC softspan range, you can set this to dynamic to get better accuracy (last argument true instead of false)
     // Note that swithing softspan range gives some glitches, so all sweep values must be inside same softspan range to use dynamic range
     bool useBestRange = abs(high)<4500 && abs(low)<4500; // based on 2.5V DAC gives apporx 5V out

     useBestRange = true; // do it anyway. Gives better accuracy. Ok as long as the sweep is slow...
     
     SMU[0].fltSetCommitVoltageSource(currentSweepValue, useBestRange);
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
     SMU[0].fltSetCommitCurrentSource(currentSweepValue);
   } 
}
