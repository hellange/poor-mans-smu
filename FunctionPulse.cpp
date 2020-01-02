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
}

void FunctionPulseClass::close() {
      closedFn(999); 
}

void FunctionPulseClass::handleButtonAction(int inputTag) {
}

void FunctionPulseClass::render(int x, int y) {

  float max = 2000.0;
  float min = -2000.0;
  if (operationType == SOURCE_CURRENT) {
    max = 20.0;
    min = -20.0;
  }
    // heading
    if (operationType == SOURCE_VOLTAGE) {
      GD.ColorRGB(COLOR_VOLT);
    } else {
      GD.ColorRGB(COLOR_CURRENT);
    }

  GD.ColorA(120);
  GD.cmd_text(x+20, y + 2 ,   29, 0, "SOURCE PULSE");
  GD.cmd_text(x+20 + 1, y + 2 + 1 ,   29, 0, "SOURCE PULSE");
  float hz = 2;
  float duration = 1000.0/hz;
  GD.ColorA(255);
  y=y+30;
  GD.cmd_number(x+242, y, 1, 3, hz);
  GD.cmd_text(x+410, y ,  1, 0, "Hz");
  y=y+110;
  if (min < 0) {
    GD.cmd_text(x+220, y ,  31, hz, "-");
  }
  GD.cmd_number(x+242, y, 31, 4, abs(min));
  GD.cmd_text(x+345, y ,  31, 0, "to");

  if (max < 0) {
    GD.cmd_text(x+385, y ,  31, 0, "-");
  }
  GD.cmd_number(x+395, y, 31, 4, abs(max));
  GD.cmd_text(x+490, y ,  31, 0, operationType == SOURCE_VOLTAGE ? "mV" : "mA");

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
