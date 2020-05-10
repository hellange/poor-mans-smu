#include "FunctionPulse.h"

#include <SPI.h>
#include "GD2.h"
#include "tags.h"
#include "colors.h"
#include "digit_util.h"
#include "Stats.h"

FunctionPulseClass FUNCTION_PULSE;


//TODO: Clean up the static mess...
float FunctionPulseClass::max = 2000.0;
float FunctionPulseClass::min = -2000.0;
//int IntervalTimer::myPulseTimer;
int FunctionPulseClass::pulseTimer = millis();

int FunctionPulseClass::pulseHigh = false;

float FunctionPulseClass::measuredHigh = 0.0;
float FunctionPulseClass::measuredLow = 0.0;
int FunctionPulseClass::hz = 20;
float FunctionPulseClass::duration = 1000;
OPERATION_TYPE FunctionPulseClass::operationType = SOURCE_VOLTAGE;
    
extern ADCClass SMU[];

void FunctionPulseClass::init(/*ADCClass& smu_*/) {
  
//  smu = smu_;
  min = -2000.0;
  max = 2000.0;
  pulseTimer = millis();

}
void FunctionPulseClass::open(OPERATION_TYPE operationType_, void (*closedFn_)(OPERATION_TYPE type)) {
  closedFn = closedFn_;
  
  operationType = operationType_;

  GD.__end();
 
  SMU[0].setCurrentRange(AMP1, operationType);
     
    SMU[0].disable_ADC_DAC_SPI_units();
    GD.resume();

    
  if (operationType == SOURCE_CURRENT) {
    max = 20.0;
    min = -20.0;
  } 

  //myPulseTimer.begin(sourcePulse, 10000); // in microseconds
  updateSamplingPeriod(hz);
  SPI.usingInterrupt(myPulseTimer);
  
}

void FunctionPulseClass::close() {
      closedFn(operationType); // do we need this ?
      myPulseTimer.end();
}

void FunctionPulseClass::updateSamplingPeriod(int hz) {
   myPulseTimer.end();
   float period = 1.0/(float)hz;
   float ms = period * 1000.0;
   float us = ms * 1000;
   Serial.print("Updating pulse sampling rate, us=");
   Serial.println(us);
   Serial.flush();
   myPulseTimer.begin(sourcePulse, us);
   myPulseTimer.priority(0); // highest pri
   SPI.usingInterrupt(myPulseTimer);
}

int sinceLastPress = millis();
void FunctionPulseClass::handleButtonAction(int inputTag) {

  float buttonStep;

  buttonStep = operationType == SOURCE_CURRENT ? 1.0 : 100.0;

  if (sinceLastPress + 100 < millis()) {
    if (inputTag == PULSE_BUTTON_INC) {
      hz = hz + (hz<10 ? 1 : 10);
      sinceLastPress = millis();
            updateSamplingPeriod(hz);

    } else if (inputTag == PULSE_BUTTON_DEC) {
      hz = hz - (hz<=10 ? 1 : 10);
      if (hz < 1) {
        hz = 1;
      }
      sinceLastPress = millis();
      updateSamplingPeriod(hz);
    } else if (inputTag == PULSE_BUTTON_INC2) {
      
      if (operationType == SOURCE_CURRENT && (min >= -1.0 and min <1.0) ) {
        buttonStep = 0.1;
      } else if (operationType == SOURCE_VOLTAGE && (min >= -100.0 and min <100.0) ) {
        buttonStep = 10.0;
      }
  
      min = min + buttonStep;
      sinceLastPress = millis();
    }  else if (inputTag == PULSE_BUTTON_DEC2) {
        if (operationType == SOURCE_CURRENT && (min > -1.0 and min <=1.0) ) {
        buttonStep = 0.1;
      } else if (operationType == SOURCE_VOLTAGE && (min > -100.0 and min <=100.0) ) {
        buttonStep = 10.0;
      }
      min = min - buttonStep;
      sinceLastPress = millis();
    }  else if (inputTag == PULSE_BUTTON_INC3) {
      if (operationType == SOURCE_CURRENT && (max >= -1.0 and max <1.0) ) {
        buttonStep = 0.1;
      } else if (operationType == SOURCE_VOLTAGE && (max >= -100.0 and max <100.0) ) {
        buttonStep = 10.0;
      }
      
      max = max + buttonStep;
      sinceLastPress = millis();
    }  else if (inputTag == PULSE_BUTTON_DEC3) {
        if (operationType == SOURCE_CURRENT && (max > -1.0 and max <=1.0) ) {
        buttonStep = 0.1;
      } else if (operationType == SOURCE_VOLTAGE && (max > -100.0 and max <=100.0) ) {
        buttonStep = 10.0;
      }
      max = max - buttonStep;
      sinceLastPress = millis();
    }
  }
}


void FunctionPulseClass::render(int x, int y) {
  


  duration = 1000.0/hz;
  
  // heading
 

  GD.ColorRGB(0x000000);

  GD.Tag(PULSE_BUTTON_DEC);
  GD.cmd_button(x+20,y+50,80,40,28,0,"Dec");
  GD.Tag(0);

  
  GD.Tag(PULSE_BUTTON_INC);
  GD.cmd_button(x+130,y+50,80,40,28,0,"Inc");
  GD.Tag(0);

  y=y+50;

  GD.Tag(PULSE_BUTTON_DEC2);
  GD.cmd_button(x+20,y+60,80,40,28,0,"Dec");
  GD.Tag(0);
  
  GD.Tag(PULSE_BUTTON_INC2);
  GD.cmd_button(x+130,y+60,80,40,28,0,"Inc");
  GD.Tag(0);
 
  y=y+50;
  
  GD.Tag(PULSE_BUTTON_DEC3);
  GD.cmd_button(x+20,y+55,80,40,28,0,"Dec");
  GD.Tag(0);
  
  GD.Tag(PULSE_BUTTON_INC3);
  GD.cmd_button(x+130,y+55,80,40,28,0,"Inc");
  GD.Tag(0);

  y=y-100;

  if (operationType == SOURCE_VOLTAGE) {
    GD.ColorRGB(COLOR_VOLT);
  } else {
    GD.ColorRGB(COLOR_CURRENT);
  }
  
  GD.ColorA(120);
  if (operationType == SOURCE_VOLTAGE) {
    GD.cmd_text(x+20, y + 2 ,   29, 0, "SOURCE VOLTAGE PULSE");
    GD.cmd_text(x+20 + 1, y + 2 + 1 ,   29, 0, "SOURCE VOLTAGE PULSE");
  } else {
    GD.cmd_text(x+20, y + 2 ,   29, 0, "SOURCE CURRENT PULSE");
    GD.cmd_text(x+20 + 1, y + 2 + 1 ,   29, 0, "SOURCE CURRENT PULSE");
  }
  

GD.ColorRGB(0xaaaaaa);
  GD.ColorA(255);
  y=y+10;
  GD.cmd_number(x+242, y+5, 1, 3, hz);
  GD.cmd_text(x+410, y+5 ,  1, 0, "Hz");
  
  y=y+95;
//  if (min < 0) {
//    GD.cmd_text(x+220, y ,  31, hz, "-");
//  }
//  GD.cmd_number(x+242, y, 31, 4, abs(min));

  
  
  DIGIT_UTIL.renderValue(x + 220,  y, min, 4, operationType == SOURCE_VOLTAGE ? DigitUtilClass::typeVoltage : DigitUtilClass::typeCurrent); 

if (operationType == SOURCE_VOLTAGE) {
    GD.ColorRGB(COLOR_VOLT);
  } else {
    GD.ColorRGB(COLOR_CURRENT);
  }
  
  //GD.cmd_text(x+365, y ,  31, 0, operationType == SOURCE_VOLTAGE ? "mV" : "mA");
  DIGIT_UTIL.renderValue(x + 500,  y, measuredLow, 4, operationType == SOURCE_VOLTAGE ? DigitUtilClass::typeVoltage : DigitUtilClass::typeCurrent); 

  GD.ColorRGB(0xaaaaaa);
  y=y+45;
//  if (max < 0) {
//    GD.cmd_text(x+220, y ,  31, 0, "-");
//  }
//  GD.cmd_number(x+242, y, 31, 4, abs(max));

  DIGIT_UTIL.renderValue(x + 220,  y, max, 4, operationType == SOURCE_VOLTAGE ? DigitUtilClass::typeVoltage : DigitUtilClass::typeCurrent); 

if (operationType == SOURCE_VOLTAGE) {
    GD.ColorRGB(COLOR_VOLT);
  } else {
    GD.ColorRGB(COLOR_CURRENT);
  }
  //GD.cmd_text(x+365, y ,  31, 0, operationType == SOURCE_VOLTAGE ? "mV" : "mA");
  DIGIT_UTIL.renderValue(x + 500,  y, measuredHigh, 4, operationType == SOURCE_VOLTAGE ? DigitUtilClass::typeVoltage : DigitUtilClass::typeCurrent); 

  GD.__end();
  //smu.pulse(min, max, duration);
 
  //sourcePulse();
  GD.resume();
  //SMU[0].sweep(5.00, -5.00, 0.1, 5000);
  GD.ColorA(255);

}



 
  
void FunctionPulseClass::sourcePulse() {
   if (operationType == SOURCE_CURRENT) {
      sourceCurrentPulse(min, max, duration);
  } else {
      sourceVoltagePulse(min, max, duration);
  }
}

 
 void FunctionPulseClass::sourceVoltagePulse(float high, float low, int duration) {
   // This is highly inaccurate. Should be implemented with hs timers and interrupts...
   //if (pulseHigh && pulseTimer+duration/2 < millis()) {
   bool useDynamicRange = (abs(low)<4500 && abs(high)<4500); // if both values are within one softspan DAC range, you can use dynamic

   if (pulseHigh) {
     pulseHigh = false;
    // pulseTimer = millis();
     bool useDynamicRange = (abs(low)<4500 && abs(high)<4500); // if both values are within one softspan DAC range, you can use dynamic
     SMU[0].fltSetCommitVoltageSource(low, useDynamicRange);
     measuredLow =  V_STATS.rawValue;
     //Serial.println("Set pulse low");
  // } else if (!pulseHigh && pulseTimer+duration/2 < millis()) {
   }
  else {
     pulseHigh = true;
     //pulseTimer = millis();
     SMU[0].fltSetCommitVoltageSource(high, useDynamicRange);
     measuredHigh =  V_STATS.rawValue;
     //Serial.println("Set pulse high");

   }
 }

 void FunctionPulseClass::sourceCurrentPulse(float high, float low, int duration) {
   // This is highly inaccurate. Should be implemented with hs timers and interrupts...
   //if (pulseHigh && pulseTimer+duration/2 < millis()) {
   if (pulseHigh) {
     pulseHigh = false;
     //pulseTimer = millis();
     SMU[0].fltSetCommitCurrentSource(low);
     measuredLow =  C_STATS.rawValue;
     //Serial.println("Set pulse low");
   }
  // } else if (!pulseHigh && pulseTimer+duration/2 < millis()) {
  else {
     pulseHigh = true;
     //pulseTimer = millis();
     SMU[0].fltSetCommitCurrentSource(high);
     measuredHigh =  C_STATS.rawValue;
     //Serial.println("Set pulse high");

   }
 }
