#include "FunctionSweep.h"

#include <SPI.h>
#include "GD2.h"
#include "tags.h"
#include "colors.h"
#include "digit_util.h"


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

  previousSweepValues[0]=0;
  previousSweepValues[1]=0;
}

void FunctionSweepClass::close() {
      closedFn(operationType); 
}




int sinceLastPress2 = millis();
int cnt = 0;
int lastButtonTag;
int pressDuration = 0;
int pauseBetweenChanges = 500;
void FunctionSweepClass::handleButtonAction(int inputTag) {

  float buttonStep;

 
  buttonStep = operationType == SOURCE_CURRENT ? 1.0 : 100.0;


  if (inputTag == 0) {
    pressDuration = 0;
  }
  
  if (lastButtonTag == inputTag) {
    if (sinceLastPress2 + pauseBetweenChanges > millis()) {
      return;
    }
  }

  pressDuration++;
  if (pressDuration >= 15) {
    pauseBetweenChanges = 50;
  }  else if (pressDuration >= 5) {
    pauseBetweenChanges = 125;
  } else {
    pauseBetweenChanges = 250;
  }
  
  sinceLastPress2 =millis();
  lastButtonTag = inputTag;

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
    
}

void FunctionSweepClass::render(int x, int y) {


  // heading
  GD.ColorRGB(operationType == SOURCE_VOLTAGE?COLOR_VOLT:COLOR_CURRENT);
  GD.ColorA(120);
  GD.cmd_text(x+20, y + 2 ,   29, 0, "SOURCE SWEEP");
  GD.cmd_text(x+20 + 1, y + 2 + 1 ,   29, 0, "SOURCE SWEEP");

  GD.ColorA(255);
  y=y+40;

  if (high < 0) {
    GD.cmd_text(x+220, y ,  31, 0/*high*/, "-");
    x=x+20;
  }
  GD.cmd_number(x+242, y+10, 30, 5, abs(high));
  
  GD.cmd_text(x+345, y ,  31, 0, "to");
   if (low < 0) { 
    GD.cmd_text(x+390, y+10 ,  30, 0, "-");
    x=x+20;
  }

  GD.ColorRGB(operationType == SOURCE_VOLTAGE?COLOR_VOLT:COLOR_CURRENT);

  GD.cmd_number(x+385, y+10, 30, 5, abs(low));
  GD.cmd_text(x+490, y ,  31, 0, operationType == SOURCE_VOLTAGE ? "mV": "mA");


  GD.ColorRGB(0x000000);
  GD.Tag(SWEEP_BUTTON_VOLT_DEC);
  GD.cmd_button(x+20,y,80,40,28,0,"Dec");
  GD.Tag(0);
  
  GD.Tag(SWEEP_BUTTON_VOLT_INC);
  GD.cmd_button(x+120,y,80,40,28,0,"Inc");
  GD.Tag(0);

  
  y=y+50;

  GD.ColorRGB(operationType == SOURCE_VOLTAGE?COLOR_VOLT:COLOR_CURRENT);

  GD.cmd_number(x+242, y, 31, 4, abs(step));
  GD.cmd_text(x+345, y ,  31, 0, "step");

  GD.ColorRGB(0x000000);

  GD.Tag(SWEEP_BUTTON_STEP_DEC);
  GD.cmd_button(x+20,y,80,40,28,0,"Dec");
  GD.Tag(0);
  
  GD.Tag(SWEEP_BUTTON_STEP_INC);
  GD.cmd_button(x+120,y,80,40,28,0,"Inc");
  GD.Tag(0);
  
  
  y=y+50;
  GD.ColorRGB(operationType == SOURCE_VOLTAGE?COLOR_VOLT:COLOR_CURRENT);

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

    GD.ColorRGB(COLOR_VOLT);

 
 

  float t = millis() - pulseTimer;
  t=t/(duration/2.0);  

  GD.ScissorXY(560,y-50-30-30);
  GD.ScissorSize(300,130);
  GD.Begin(RECTS);  

  /*
  GD.ColorRGB(COLOR_VOLT);
  GD.LineWidth(10);
  GD.Vertex2ii(x+560, y-50-30);
  GD.Vertex2ii(x+560+200 ,y-20);
  */
 
  GD.ColorRGB(COLOR_VOLT);
  DIGIT_UTIL.renderValue(x + 560,  y-50-t*30 , currentSweepValue, 3, -1); 
  GD.ColorRGB(0x666666);
  DIGIT_UTIL.renderValue(x + 560,  y-80-t*30 , previousSweepValues[0], 3, -1); 
  GD.ColorRGB(0x333333);
  DIGIT_UTIL.renderValue(x + 560,  y-110-t*30 , previousSweepValues[1], 3, -1); 

  
  GD.ScissorXY(0,0);
  GD.ScissorSize(800-1,480-1);

  

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

     previousSweepValues[1] = previousSweepValues[0];
     previousSweepValues[0] = currentSweepValue;
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
