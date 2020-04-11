#include "Arduino.h"
#include "PushButtons.h"
#include "GD2.h"


PushbuttonsClass PUSHBUTTONS;

void PushbuttonsClass::init(int analogPin_, int holdPeriodms_) {
  analogPin = analogPin_;
  holdPeriodms = holdPeriodms_;
  Serial.print("Init Pushbuttons using analog pin ");
  Serial.println(analogPin);
}

void PushbuttonsClass::setCallback(void (*callback)(int button, bool quickPress, bool holdAfterLongPress, bool releaseAfterLongPress)) {
  callbackFn = callback;
}

void PushbuttonsClass::handle() {
  
  int buttonValue = analogRead(analogPin)/10;

  // Based on how the voltage divider is constructed using the x number of buttons.
  // Measure and adjust...
  // Using 8 bit analog input on Teensy, it should be possible to "detect" many buttons
  // with just an analog input...
  if (buttonDetectedTimer + 50 < millis()) {
    buttonDetectedTimer = millis();
    if (buttonValue >95 && buttonValue <105) {
      buttonFunction = 0;
      keyHeldLong= false;
    }
    else if (buttonValue >82 && buttonValue <94) {
      buttonFunction = 1;
    }
    else if (buttonValue >70 && buttonValue <81) {
      buttonFunction = 2;  
    }
    else if (buttonValue >50 && buttonValue <70) {
      buttonFunction = 3;  
    }
   else  if (buttonValue <10) {
      buttonFunction = 4;
    }

    if (buttonFunction != prevButtonFunction) {
      prevButtonFunction = buttonFunction;
      if (buttonFunction != 0) {
        //we have detected a button pressed down !
        buttonDetected ++;
        keydownTimer = millis();
      }
      else {
        //we have detected button released !
        buttonDepressed ++;
        keydownTimer = 0;
        if (color == 0xff0000) {
          callbackFn(buttonFunction, false, false, true);
        } else {
          callbackFn(buttonFunction,true, false, false);
        }
      }
    }
    if (keydownTimer + holdPeriodms < millis() && buttonFunction != 0){
      color = 0xff0000;
      if (keyHeldLong == false) {
        keyHeldLong = true;
        callbackFn(99,false, true, false);
      }
    } else {
      color = 0x00ff00;
    }

   
  }

  if (buttonFunction != 0) {
    GD.Begin(RECTS);
  GD.ColorA(255);
  GD.ColorRGB(color);
  int y=(buttonFunction-1)*110;
  GD.Vertex2ii(0,50+y);
  GD.Vertex2ii(70,120+y, 22);
  }




  
  
  
  GD.cmd_number(730,0,27,0,buttonFunction);
  GD.cmd_number(750,0,27,0,buttonDetected);
  GD.cmd_number(780,0,27,0,buttonDepressed);



}
