
#include "RotaryEncoder.h"
#include "Debug.h"

RotaryEncoderClass ROTARY_ENCODER;

#define CLK 14
#define DT 15
#define SW 16

void RotaryEncoderClass::init(void (*changedFn_)(float value)) {
  changedFn = changedFn_;
  knobLeft.begin(DT, CLK, EncoderTool::CountMode::half);
}

void RotaryEncoderClass::handle(bool reduceResolution)
{
  //knobLeft.tick();
  long newLeft;
  int speed = 0;
  int dir = 0;
  if (!knobLeft.valueChanged()) { // do we have a new value?
   return;
  }
 
  newLeft = knobLeft.getValue();//.read()/2;
  //newRight = knobRight.read();
  if (newLeft != positionLeft /*|| newRight != positionRight*/) {    
    if (newLeft < positionLeft) {
      dir=-1;
    } else {
      dir=+1;
    }

    // We dont want the value to change just linearly with rotation.
    // When turning fast, we want value to change non-linear, meaning
    // faster turning shall change value faster than the actual turning.
    // TODO: Find the terminology normally used.
    float feelyDampening = 5.0;  // How fast will value change when you turn the knob.  
                                 // Higher value gives less velocity dependant value
    float feelyStartMs = 100;  // Minimum value for how fast you must turn knob before feely velocity dependency kicks in.
                               // Large value gives longer time before feely is enabled.
                               // Value is directly related to milliseconds between each encoder "click" 

    // For now We just use this very simple velocity dependent algorithm...
    int msSinceLast = millis() - millisAtLastStep;
    if (stepless_dynamic) {
        if (msSinceLast > feelyStartMs) {
          speed = 1;
        } else {
          speed = 1 + (feelyStartMs - msSinceLast) / feelyDampening;
        }
    } else {
      speed = 1;
    }

    // When beyond the resolution of the device (i.e. 0.000001V)
    if (reduceResolution) {
      speed = speed * 2.0;
    }
       
    millisAtLastStep = millis();
    //DEBUG.print("Left = ");
    //DEBUG.print(newLeft);
    //DEBUG.print(", Right = ");
    //DEBUG.print(newRight);
    //DEBUG.print(" speed= ");
    //DEBUG.print( speed);
    //DEBUG.print(" dir= ");
    //DEBUG.print( dir);
    //DEBUG.println();
    positionLeft = newLeft;
    changedFn(speed*dir / 10.0); // minimum step is 100uV
    millisAtLastVoltageCommit = millis();    
  }
 
}
