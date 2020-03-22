
#include "RotaryEncoder.h"

RotaryEncoderClass ROTARY_ENCODER;

#define CLK 14
#define DT 15
#define SW 16
Encoder knobLeft(DT, CLK);

void RotaryEncoderClass::init(void (*changedFn_)(float value)) {
  changedFn = changedFn_;
}
void RotaryEncoderClass::handle(bool use100uVSetResolution)
{
 long newLeft, newRight;
 int speed = 0;
 int dir = 0;

 
  newLeft = knobLeft.read()/2;
  //newRight = knobRight.read();
  if (newLeft != positionLeft /*|| newRight != positionRight*/) {

    // TODO: Seems to give one event at startup. Why ?  Ignore that first event
    if (!ready) {
      ready = true;
      return;
    }
    
    if (newLeft < positionLeft) {
      dir=-1;
    } else {
      dir=+1;
    }

    bool stepless_dynamic = false;  // decide if the dynamic speed shall be directly dependent on rotation speed or if there shall just be a few different speeds 

   
   
    float feely = 0.6;  // how fast will value change when you turn the knob.  High value changes faster than low value
   
    if (stepless_dynamic) {
      speed = millis() - millisSinceLastStep;
      if (speed > 100) {
        speed = 100;
      }
      speed = 1001 - speed * 10;
       
    } else {

      if (millisSinceLastStep + 10 * feely > millis()) {
        speed = 10000;
      } else
      if (millisSinceLastStep + 50 * feely > millis()) {
        speed = 1000;
      } else
      if (millisSinceLastStep + 100 * feely > millis()) {
        speed = 500;
      } 
      else if (millisSinceLastStep + 150 * feely > millis()) {
        speed = 100;
      } 
      else if (millisSinceLastStep + 200 * feely > millis()) {
        speed = 10;
      } else {
        speed = 1;
      }
      
    
    }

    if (!use100uVSetResolution) {
      speed = speed * 2.0;
    }
   
   

    
    millisSinceLastStep = millis();
    Serial.print("Left = ");
    Serial.print(newLeft);
    //Serial.print(", Right = ");
    //Serial.print(newRight);
    Serial.print(" speed= ");
    Serial.print( speed);
      Serial.print(" dir= ");
    Serial.print( dir);
    
    Serial.println();
    positionLeft = newLeft;


    changedFn(speed*dir / 100.0); // minimum step is 10uV
    //changedFn(speed*dir / 10.0); // minimum step is 100uV
/*
    float mv = SMU[0].getSetValuemV();

    float newVoltage = mv + speed*dir / 10.0;
    
      if (operationType == SOURCE_VOLTAGE) {
        //if (SMU[0].fltSetCommitVoltageSource(newVoltage, true)) printError(_PRINT_ERROR_VOLTAGE_SOURCE_SETTING);
      }
      */
      millisSinceLastVoltageCommit = millis();
    


    
    //positionRight = newRight;
  }
 

 
}
