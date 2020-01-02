#include "FunctionPulse.h"

#include <SPI.h>
#include "GD2.h"
#include "tags.h"
#include "colors.h"

FunctionPulseClass FUNCTION_PULSE;

void FunctionPulseClass::init(ADCClass& smu_) {
  smu = smu_;
}
void FunctionPulseClass::open(OPERATION_TYPE operationType_, void (*closedFn_)(int type)) {
  closedFn = closedFn_;
  operationType = operationType_;
  smu.setCurrentRange(AMP1);
  if (operationType == SOURCE_CURRENT) {
    max = 20.0;
    min = -20.0;
  }
}

void FunctionPulseClass::close() {
      closedFn(999); 
}

int sinceLastPress = millis();
void FunctionPulseClass::handleButtonAction(int inputTag) {

  float buttonStep = operationType == SOURCE_CURRENT ? 1.0 : 100.0;
  
  if (sinceLastPress + 100 < millis()) {
    if (inputTag == PULSE_BUTTON_INC) {
      hz = hz + 10;
      sinceLastPress = millis();
    } else if (inputTag == PULSE_BUTTON_DEC) {
      hz = hz -10;
      if (hz < 0) {
        hz = 1;
      }
      sinceLastPress = millis();
    } else if (inputTag == PULSE_BUTTON_INC2) {
      min = min + buttonStep;
      sinceLastPress = millis();
    }  else if (inputTag == PULSE_BUTTON_DEC2) {
      min = min - buttonStep;
      sinceLastPress = millis();
    }  else if (inputTag == PULSE_BUTTON_INC3) {
      max = max + buttonStep;
      sinceLastPress = millis();
    }  else if (inputTag == PULSE_BUTTON_DEC3) {
      max = max - buttonStep;
      sinceLastPress = millis();
    }
  }
}

void FunctionPulseClass::render(int x, int y) {
  


  float duration = 1000.0/hz;
  
  // heading
 

  GD.ColorRGB(0x000000);

    GD.Tag(PULSE_BUTTON_DEC);
  GD.cmd_button(x+20,y+50,100,40,28,0,"Dec");
  GD.Tag(0);
  
  GD.Tag(PULSE_BUTTON_INC);
  GD.cmd_button(x+130,y+50,100,40,28,0,"Inc");
  GD.Tag(0);



  y=y+50;

   GD.Tag(PULSE_BUTTON_DEC2);
  GD.cmd_button(x+20,y+50,100,40,28,0,"Dec");
  GD.Tag(0);
  
  GD.Tag(PULSE_BUTTON_INC2);
  GD.cmd_button(x+130,y+50,100,40,28,0,"Inc");
  GD.Tag(0);

 
  y=y+50;

  
  GD.Tag(PULSE_BUTTON_DEC3);
  GD.cmd_button(x+20,y+50,100,40,28,0,"Dec");
  GD.Tag(0);
  
  GD.Tag(PULSE_BUTTON_INC3);
  GD.cmd_button(x+130,y+50,100,40,28,0,"Inc");
  GD.Tag(0);


  y=y-100;

  if (operationType == SOURCE_VOLTAGE) {
    GD.ColorRGB(COLOR_VOLT);
  } else {
    GD.ColorRGB(COLOR_CURRENT);
  }
  
  GD.ColorA(120);
  GD.cmd_text(x+20, y + 2 ,   29, 0, "SOURCE PULSE");
  GD.cmd_text(x+20 + 1, y + 2 + 1 ,   29, 0, "SOURCE PULSE");


  GD.ColorA(255);
  y=y+10;
  GD.cmd_number(x+242, y, 1, 3, hz);
  GD.cmd_text(x+410, y ,  1, 0, "Hz");
  
  y=y+100;
  if (min < 0) {
    GD.cmd_text(x+220, y ,  31, hz, "-");
  }
  GD.cmd_number(x+242, y, 31, 4, abs(min));

  y=y+40;
  if (max < 0) {
    GD.cmd_text(x+220, y ,  31, 0, "-");
  }
  GD.cmd_number(x+242, y, 31, 4, abs(max));
  
  GD.cmd_text(x+385, y ,  31, 0, operationType == SOURCE_VOLTAGE ? "mV" : "mA");

  GD.__end();
  //smu.pulse(min, max, duration);
  if (operationType == SOURCE_CURRENT) {
      sourceCurrentPulse(min, max, duration);
  } else {
      sourceVoltagePulse(min, max, duration);
  }
  GD.resume();
  //SMU[0].sweep(5.00, -5.00, 0.1, 5000);
  GD.ColorA(255);

}

 
 void FunctionPulseClass::sourceVoltagePulse(float high, float low, int duration) {
   // This is highly inaccurate. Should be implemented with hs timers and interrupts...
   if (pulseHigh && pulseTimer+duration/2 < millis()) {
     pulseHigh = false;
     pulseTimer = millis();
     smu.fltSetCommitVoltageSource(low);
     //Serial.println("Set pulse low");
   } else if (!pulseHigh && pulseTimer+duration/2 < millis()) {
     pulseHigh = true;
     pulseTimer = millis();
     smu.fltSetCommitVoltageSource(high);
     //Serial.println("Set pulse high");

   }
 }

 void FunctionPulseClass::sourceCurrentPulse(float high, float low, int duration) {
   // This is highly inaccurate. Should be implemented with hs timers and interrupts...
   if (pulseHigh && pulseTimer+duration/2 < millis()) {
     pulseHigh = false;
     pulseTimer = millis();
     smu.fltSetCommitCurrentSource(low);
     //Serial.println("Set pulse low");
   } else if (!pulseHigh && pulseTimer+duration/2 < millis()) {
     pulseHigh = true;
     pulseTimer = millis();
     smu.fltSetCommitCurrentSource(high);
     //Serial.println("Set pulse high");

   }
 }
