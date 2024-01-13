#include "Digitizer.h"
#include "Debug.h"

//extern ADCClass SMU[];
extern VoltDisplayClass VOLT_DISPLAY;

//OPERATION_TYPE operationType;

bool DigitizerClass::zoomed = false;
int DigitizerClass::adjustLevel = 0;
int DigitizerClass::ampLevel = 1;
float DigitizerClass::triggerLevel = 1000.0;
int DigitizerClass::multiplyBy = 1;
int DigitizerClass::triggerType = 0;

DigitizerClass DIGITIZER;

void DigitizerClass::init(SMU_HAL &SMU) {
   SMU1 = &SMU;

   if (digitizeVoltage == false) {
     triggerLevel = 100.0;
   }
}

void DigitizerClass::updateSmuWhenVoltageCurrentFocusChange() {
   if (digitizeVoltage) {
    SMU1->enableVoltageMeasurement(true);
    SMU1->enableCurrentMeasurement(false);
  } else {
    SMU1->enableVoltageMeasurement(false);
    SMU1->enableCurrentMeasurement(true);
  }
    SMU1->updateSettings();

}
void DigitizerClass::open() {
  GD.__end();
  prevSamplingRate = SMU1->getSamplingRate();
  SMU1->disable_ADC_DAC_SPI_units();
  SMU1->setSamplingRate(31250); //31250

  updateSmuWhenVoltageCurrentFocusChange();
  // if (digitizeVoltage) {
  //   SMU1->enableVoltageMeasurement(true);
  //   SMU1->enableCurrentMeasurement(false);
  // } else {
  //   SMU1->enableVoltageMeasurement(false);
  //   SMU1->enableCurrentMeasurement(true);
  // }

  //SMU1->updateSettings();
  GD.resume();
}

void DigitizerClass::close() {
    // TODO: Check that graphics are not "on" ???
    GD.__end();
    SMU1->disable_ADC_DAC_SPI_units();
    //SMU1->setSamplingRate(20); //TODO: Should get back to same as before, not a default one
    SMU1->enableVoltageMeasurement(true);
    SMU1->enableCurrentMeasurement(true);
    SMU1->updateSettings();
    SMU1->setSamplingRate(prevSamplingRate); //TODO: Should get back to same as before, not a default one


    digitize = false;
    bufferOverflow = false;

}

void DigitizerClass::loopDigitize(bool reduceDetails) {
  
  //operationType = getOperationType();
  if (reduceDetails) {
   // return;
  }
  if (digitize == false && !MAINMENU.active){
        SMU1->disable_ADC_DAC_SPI_units();
   
    //minDigV = 1000000;
    //maxDigV = - 1000000;
    //minDigI = 1000000;
    //maxDigI = - 1000000;
    //SMU1->setSamplingRate(31250); //31250
    
    // Make sure smu is updated according
    // to current or voltage used for digitizing
    updateSmuWhenVoltageCurrentFocusChange();

    digitize = true;
    bufferOverflow = false;     
  }
  if (bufferOverflow == true) {
    digitize = false;
  }
 
}

unsigned long digitizerCheckButtonTimer = millis();
void DigitizerClass::renderGraph(bool reduceDetails) {

      GD.Tag(0);
 


          if (!MAINMENU.active) {

      GD.ColorA(255);
      GD.ColorRGB(0x000000);
      GD.Tag(171);
      if (DIGITIZER.allowTrigger) {
         GD.cmd_button(200,420,100,40,29,0, "STOP");
      } else {
         GD.cmd_button(200,420,120,40,29,0, "SET AUTO");
      }
      GD.Tag(172);
      GD.cmd_button(50,420,140,40,29,0, "TRIGGER");

      GD.Tag(173);
      GD.cmd_button(610,420,200,40,29,0, "VOLT/CURRENT");


      GD.Tag(0);

      int tag = GD.inputs.tag;
      if (digitizerCheckButtonTimer+500 < millis()) {
        digitizerCheckButtonTimer = millis();
        if (tag == 171) {
          DIGITIZER.allowTrigger = ! DIGITIZER.allowTrigger;
        }
        else if (tag == 172) {
          triggerType ++;
          if (triggerType >2) {
            triggerType = 0;
          }
          if (triggerType ==0) {
            continuous = true;
          } else {
            continuous = false;
          }
        }
        else if (tag == 173) {
          digitizeVoltage = !digitizeVoltage;
          //TODO: Set trigger level to same as before switching between voltage and current !
          if (digitizeVoltage == false) {
            triggerLevel = 100.0;
          } else {
            triggerLevel = 1000.0;
          }
        }
      }
    

    //VOLT_DISPLAY.renderMeasured(10,50, minDigV, false);
    //VOLT_DISPLAY.renderMeasured(10,150, maxDigV, false);
    //  CURRENT_DISPLAY.renderMeasured(10,250, minDigI, false, false,current_range);
    //  CURRENT_DISPLAY.renderMeasured(10,350, maxDigI, false, false, current_range);
    //GD.cmd_number(210,320, 28, 6, adrAtTrigger);
    //GD.cmd_number(410,320, 28, 6, ramAdrPtr);

    if (digitizeVoltage) {
       GD.ColorRGB(COLOR_VOLT);
       GD.cmd_text(300, 40 ,  28, 0, "VOLT");
    } else {
       GD.ColorRGB(COLOR_CURRENT);
       GD.cmd_text(300, 40 ,  28, 0, "CURRENT");
    }

    if (allowTrigger) {
       //GD.cmd_number(500,320, 28, 6, allowTrigger);
       GD.ColorRGB(0x00ff00);
       GD.cmd_text(200, 380 ,  28, 0, "AUTO");
     } else  {
       GD.ColorRGB(0xff0000);
       GD.cmd_text(200, 380 ,  28, 0, "STOPPED");
     }
     
     if (adjustLevel == 0) {
       GD.ColorRGB(0x00ff00);
       GD.cmd_text(380, 380 ,  28, 0, "MEAS SCALE");
       GD.cmd_number(550, 380, 28, 6, ampLevel);
     } if (adjustLevel == 1) {
       GD.ColorRGB(0xaaaaaa);
       GD.cmd_text(380, 380 ,  28, 0, "TIME SCALE");
       GD.cmd_number(550, 380, 28, 6, zoomed);
     } if (adjustLevel == 2) {
       GD.ColorRGB(0xffff00);
       GD.cmd_text(380, 380 ,  28, 0, "TRIG LEVEL");       
       GD.cmd_number(550, 380, 28, 6, triggerLevel);
       int trigLevel0y = 240;
       int trigLevelAbjustedy = trigLevel0y - (triggerLevel/100.0)*multiplyBy;
       if (digitizeVoltage == false) {
         trigLevelAbjustedy = trigLevel0y - ((triggerLevel/100.0)*multiplyBy)*10.0*1.20;
         if (trigLevelAbjustedy < 0) {
          trigLevelAbjustedy = 0;
         }
       }
       // This will just somethimes seen as a green flash.
       // TODO: How to show that the trigger has occured and
       //       waveform has been acquired. 
       //       Check with not normal oscilloscopes shows triggering...
       if (triggered == true) {
         GD.ColorRGB(0x00ff00);
       } else {
          GD.ColorRGB(0xff0000);
       }

              GD.cmd_text(392, trigLevelAbjustedy -26,  31, 0, "-");


        GD.ColorRGB(0xffff00); // yellowish
        // show trigger voltage line
        GD.LineWidth(10);
        GD.Begin(LINE_STRIP);
        GD.Vertex2ii(0, trigLevelAbjustedy);
        GD.Vertex2ii(800, trigLevelAbjustedy);

     }



     if (triggerType == 0) {
       //GD.cmd_number(500,320, 28, 6, allowTrigger);
       GD.ColorRGB(0x0000ff);
       GD.cmd_text(50, 380 ,  28, 0, "CONTINOUS");
     } else if (triggerType == 1) {
      GD.ColorRGB(0xffff00);
       GD.cmd_text(50, 380 ,  28, 0, "LEVEL");
     }  else if (triggerType == 2) {
       GD.ColorRGB(0x00ff00);
       GD.cmd_text(50, 380 ,  28, 0, "EDGE");
     } else {
          GD.ColorRGB(0xff0000);
       GD.cmd_text(50, 380 ,  28, 0, "???");
     }


 
      GD.ColorA(100);
      GD.ColorRGB(COLOR_CURRENT);

      //DIGIT_UTIL.renderValue(170,  32 , v, 3, DigitUtilClass::typeCurrent); 
}


int yAxisPx = 240;
int xAxisPx = 400;
int height = 400;
int width = 790;
int xStep = 100;

// avoid drawing graph outside of target area (can happen if values are large...)
  GD.ScissorXY(0,30); // height of header
  GD.ScissorSize(800,440);
GD.ColorA(255);

//GD.ColorRGB(0x00ff00);
//GD.cmd_number(0,yAxisPx, 28, 6, 0);

GD.ColorRGB(0xbbbbee);
// origo horizontal and vertical
GD.LineWidth(5);
GD.Begin(LINE_STRIP);
GD.Vertex2ii(xAxisPx-width/2, yAxisPx);
GD.Vertex2ii(xAxisPx+width/2, yAxisPx);
GD.Begin(LINE_STRIP);
GD.Vertex2ii(xAxisPx, yAxisPx + height/2);
GD.Vertex2ii(xAxisPx, yAxisPx - height/2);

// vertical lines
GD.LineWidth(3);
for (int i=xStep; i<width/2; i=i+xStep ) {
    GD.Begin(LINE_STRIP);
    GD.Vertex2ii(xAxisPx + i, yAxisPx-height/2);
    GD.Vertex2ii(xAxisPx + i, yAxisPx+height/2);
    GD.Begin(LINE_STRIP);
    GD.Vertex2ii(xAxisPx - i, yAxisPx-height/2);
    GD.Vertex2ii(xAxisPx - i, yAxisPx+height/2);
}

/*
//horizontal lines
int yStep = 75;
for (int i=yStep; i<height/2; i=i+yStep) {
    GD.Begin(LINE_STRIP);
    GD.Vertex2ii(xAxisPx+width/2, yAxisPx + i);
    GD.Vertex2ii(xAxisPx-width/2, yAxisPx + i);
    GD.Begin(LINE_STRIP);
    GD.Vertex2ii(xAxisPx+width/2, yAxisPx - i);
    GD.Vertex2ii(xAxisPx-width/2, yAxisPx - i);
}
*/

    //    VOLT_DISPLAY.renderMeasured(10,200, fullSamplePeriod, false);

    GD.LineWidth(10);
    GD.Begin(LINE_STRIP);
    GD.ColorA(255);
    //GD.ColorRGB(0x00ff00);
    //clearMaxMin();
    //int multiplyBy = 1;
    if (ampLevel ==2) {
        multiplyBy = 2;
    } else if (ampLevel ==3) {
        multiplyBy = 4;
    } else if (ampLevel ==4) {
        multiplyBy = 10;
    }
    float pixelsPrVolt = multiplyBy * 10;
    if (digitizeVoltage == false) {
        pixelsPrVolt = multiplyBy * 100  / 0.8;
    }

    if (SMU1->getCurrentRange() == MILLIAMP10) {
        pixelsPrVolt=pixelsPrVolt*100; //TODO: Fix this current range scale etc !  For now just make it "visible" :-)
    }


    // calculate to and from values for main graph based on buffer and reolution 
    int resolution = 2; // 1 is best (using all samples)
    int from = 0;
    int to = nrOfFloats;
    if (zoomed) {
        from = nrOfFloats*0.25;
        to = nrOfFloats*0.75;
        resolution = 1;
    }
    //Render main graph
    //Note that number of lines must not be too high.
    //If will overflow the command buffer of the graphics chip.
    //That's one of the drawbacks with using FT8xx chip...
    GD.ColorRGB(digitizeVoltage?COLOR_VOLT:COLOR_CURRENT);
    GD.Begin(LINE_STRIP);
    int xCoordinate = 0;
    for (int x = from; x<to; x += resolution) {
      GD.Vertex2ii(xCoordinate, yAxisPx - mva[x]/1000.0 * pixelsPrVolt);
      xCoordinate +=4; 
    }

    if (!MAINMENU.active) {

 GD.ColorRGB(0xaaaaaa);
GD.cmd_text(0, yAxisPx-height/2 -10 , 27, 0, "Max:");
GD.ColorRGB(digitizeVoltage?COLOR_VOLT:COLOR_CURRENT);
if (maxDigV < 0.0000) {
    GD.cmd_text(50, yAxisPx-height/2+20 -10 , 27, 0, "-");
  }
if (SMU1->getCurrentRange() == AMP1) {
  GD.cmd_number(60,yAxisPx-height/2 -10, 27, 5, maxDigV);
  GD.cmd_text(60+55, yAxisPx-height/2 -10 , 27, 0, digitizeVoltage?"mV":"mA");
} else {
    GD.cmd_number(60,yAxisPx-height/2 -10, 27, 5, maxDigV*1000.0);
    GD.cmd_text(60+55, yAxisPx-height/2 -10 , 27, 0, digitizeVoltage?"uV":"uA");
}

GD.ColorRGB(0xaaaaaa);
GD.cmd_text(0, yAxisPx-height/2 +20 -10, 27, 0, "Min:");
//if (minDigV < 0.0000) {
    GD.ColorRGB(digitizeVoltage?COLOR_VOLT:COLOR_CURRENT);
  if (minDigV < 0.0000) {
    GD.cmd_text(50, yAxisPx-height/2+20 -10 , 27, 0, "-");
  }
  if (SMU1->getCurrentRange() == AMP1) {
    GD.cmd_number(60,yAxisPx-height/2+20 -10, 27, 5, abs(minDigV));
    GD.cmd_text(60+55, yAxisPx-height/2+20 -10 , 27, 0, digitizeVoltage?"mV":"mA");
  } else {
    GD.cmd_number(60,yAxisPx-height/2+20 -10, 27, 5, abs(minDigV)* 1000.0);
    GD.cmd_text(60+55, yAxisPx-height/2+20 -10 , 27, 0, digitizeVoltage?"uV":"uA");
  }

//} 
}
 // render scale digits
int time = 0;
int digits = 0;
int step = 4 * 10; //TODO: Adjust according to sampling speed etc
int axisScew = 5;
int yAxis = yAxisPx + 20;
GD.ColorRGB(0xaaaaaa);
// x-axis digits (minus time)
for (int x = xAxisPx; x > xAxisPx-width/2; x=x-step) {
    if (time != 0) {
        GD.cmd_text(x-10 - axisScew, yAxis, 26, 0, "-");
    }
    GD.cmd_number(x -axisScew,yAxis, 26, digits, time);
    time = time + resolution;
}
// x-axis digits (plus time)
time = resolution;
for (int x = xAxisPx + step; x < xAxisPx+width/2; x=x+step) {
    GD.cmd_number(x-axisScew,yAxis, 26, digits, time);
    time = time + resolution;
 }


 // y-axis (voltage/current)
 int valStep = 1;
 if (ampLevel == 1) { // x1
     valStep = 4;
     step = 50;
 }
 if (ampLevel == 2) { // x2
     valStep = 2;
     step = 50;
 } else if (ampLevel ==3) { // x4
     valStep = 1;
     step = 50;
 } else if (ampLevel ==4) { // x10
     valStep=0.4 /0.4;
     step = 50 /0.4;
 } 
digits = 5;
axisScew = 8;
int val = valStep;
// positive
for (int y= yAxisPx-step; y > yAxisPx-height/2; y=y-step) {
    GD.ColorRGB(0xaaaaaa);
       if (digitizeVoltage) {
    GD.cmd_number(15, y-axisScew, 26, digits, val * 5000 / 4);
    } else {
    GD.cmd_number(15, y-axisScew, 26, digits, val * (SMU1->getCurrentRange() == MILLIAMP10 ? 1000:100));
    }
    GD.Begin(LINE_STRIP);
    GD.ColorRGB(0xbbbbee);
    GD.LineWidth(3);
    GD.Vertex2ii(xAxisPx+width/2, y);
    GD.Vertex2ii(xAxisPx-width/2, y);

    val = val + valStep;
}
val = valStep;
// negative
for (int y= yAxisPx+step; y < yAxisPx+height/2; y=y+step) {
    GD.ColorRGB(0xaaaaaa);
    GD.cmd_text(0, y-axisScew, 26, 0, "-");
    if (digitizeVoltage) {
      GD.cmd_number(15, y-axisScew, 26, digits, val* 5000 / 4);
    } else {
      GD.cmd_number(15, y-axisScew, 26, digits, val * (SMU1->getCurrentRange() == MILLIAMP10 ? 1000:100));
    }

    GD.Begin(LINE_STRIP);
    GD.ColorRGB(0xbbbbee);
    GD.LineWidth(3);
    GD.Vertex2ii(xAxisPx+width/2, y);
    GD.Vertex2ii(xAxisPx-width/2, y);

    val = val + valStep;
 }


}

void DigitizerClass::updateMaxMin(float v) {
    if (v > maxDigV) {
        maxDigV = v;
      } 
      if (v < minDigV) {
        minDigV = v;
      } 
}

void DigitizerClass::copyDataBufferToDisplayBuffer() {
    for (int x=0; x<=nrOfFloats;x++) {
       mva[x] = 0.0;
   }
   clearMaxMin();
   // Before trigger 
   for (int x=0; x<=nrOfFloats/2;x++) {
       mva[x+nrOfFloats/2] = ramEmulator2[x];
       updateMaxMin(ramEmulator2[x]); // Note: uses all samples, but extreme may be masked out by display only some

   }

   int adr = adrAtTrigger;
      // After trigger 
   for (int x=nrOfFloats/2; x>=0 ;x--) {
       //mva[x] = 0.0;
        mva[x] = ramEmulator[adr];
        updateMaxMin(ramEmulator[adr]);// Note: uses all samples, but extreme may be masked out by display only some

        adr--;
        if (adr < 0) {
            adr = nrOfFloats;
        }
   }
  
}


void DigitizerClass::a() {

}

void DigitizerClass::clearMaxMin() {
  maxDigV = -100000.00, minDigV = 100000.00;
}



// For testing
float modulation = 0.05;
float ampDir = 1;
float modDir = 1;

float amplitude = 2;
unsigned long modulationTimer = millis();
  void DigitizerClass::updateModulation() {
        //for test
  if (modulationTimer + 100 < millis()) {
    if (modulation > 0.2) {
        modDir = -1;
    } else if (modulation <0.02) {
        modDir = 1;
    }
     //   modulation = modulation + 0.001 * modDir;

    modulationTimer = millis();
    amplitude += 0.1 * ampDir;
    if (amplitude>5) {
        ampDir = -1;
    } else if (amplitude <1){
        ampDir = +1;
    }
   }

  }



void DigitizerClass::handleSamplingForDigitizer(int dataR) {

  if (rendering) {
      return; // Dont' handle sampling while rendering
  }

 // No need to filter voltage or current as long as only single channel is used on ADC for sampling.
 // 0 volt,   1 current
 //   if (dataR == 0) {
 //     return;
 //   }


  if (bufferOverflow) {
      return;
  }

  
  //float v;  
  if (digitizeVoltage) {
      v = SMU1->measureMilliVoltage();
  } else {
      v = SMU1->measureCurrent(SMU1->getCurrentRange());
  }

  // edge or level trig
  bool edgeDetect = (triggerType == 2);

  bool trigg = false;
  if (edgeDetect == true) {
    // ----- edge trigger hack-----
      float edgeSteepnessToDetect = 1.0;

    bool positiveEdge = v > lastVoltage + edgeSteepnessToDetect 
         && lastVoltage > lastVoltageOld + edgeSteepnessToDetect
         && lastVoltageOld > lastVoltageOld2 + edgeSteepnessToDetect
         && lastVoltageOld2 > lastVoltageOld3 + edgeSteepnessToDetect;
     trigg = positiveEdge; //|| continuous;

  }
  else 
  {


    // ----- level trigger hack-----

    trigg = (lastVoltage > triggerLevel 
               && lastVoltageOld >triggerLevel 
               && lastVoltageOld2 >triggerLevel 
               && lastVoltageOld3 >triggerLevel);//  || continuous;
    if (trigg && waitForLower == false) {
      waitForLower=true;
    }
    else if (trigg && waitForLower == true) {
      trigg = false;
    }
    int delta = 100;
    if (digitizeVoltage == false) {
      delta = 10;
    }
    // Must go below it again to trigger again
    bool lower = lastVoltage < triggerLevel -delta
               && lastVoltageOld <triggerLevel -delta
               && lastVoltageOld2 <triggerLevel -delta
               && lastVoltageOld3 <triggerLevel -delta;
    if (lower==true && waitForLower == true) {
      waitForLower = false;
    }
  }
  trigg = trigg | continuous;






    //  SIMULATE WAVEFORM
    //  bool zeroCross = false;
    //   simulatedWaveform += modulation;
    //   v=sin(simulatedWaveform)*1000.0 * amplitude;
    //   v=v+random(100); // simulate noise;
    //   zeroCross = lastVoltage < 0.0 && v > 0.0  && v>lastVoltage; // zero crossing positive edge
    //   trigg = zeroCross;

  
  if (!allowTrigger) {
      trigg = false;
  }

  countSinceLastSample ++;
  if (countSinceLastSample<nrOfFloats) {
      trigg = false; // dont trust edge detection before some samples after last full buffer
  }

  if (triggered) {
      ramEmulator2[samplesAfterTrigger++] = v;
  }

  if (!triggered) {
    if (trigg) {
      triggered = true;
      adrAtTrigger = ramAdrPtr;
      } 
    }

    lastVoltageOld3 = lastVoltageOld2;

    lastVoltageOld2 = lastVoltageOld;
    lastVoltageOld = lastVoltage;
    lastVoltage = v;
    ramEmulator[ramAdrPtr] = v;

    ramAdrPtr ++;
    if (ramAdrPtr > nrOfFloats) {
        ramAdrPtr = 0;
    }
    
    // Check end of buffer
    if (samplesAfterTrigger == nrOfFloats/2) {
      bufferOverflow = true;
      triggered = false;
      samplesAfterTrigger = 0;
      countSinceLastSample = 0;
      lastVoltage=0.0;
      lastVoltageOld=0.0;
      lastVoltageOld2=0.0;
      lastVoltageOld3=0.0;

      copyDataBufferToDisplayBuffer();

      //for test
      updateModulation();
    }

  

  }

void DigitizerClass::rotaryEncChanged(float changeValue) {
  DEBUG.print("DigitizerClass rotaryEncChanged, value:");
  DEBUG.println(changeValue);
  if (adjustLevel == 1) {
    zoomed = !zoomed;
    DEBUG.print("zoomed=");
    DEBUG.println(zoomed);
  } else if (adjustLevel == 0) {
    if (changeValue < 0) {
      ampLevel = (ampLevel>1)?ampLevel - 1 : 1;
    } else {
      ampLevel = (ampLevel<4)?ampLevel + 1 : 4;
    }
     DEBUG.print("ampLevel=");
    DEBUG.println(ampLevel);
  } else if (adjustLevel == 2) {
    if (changeValue < 0) {
      triggerLevel = triggerLevel + 100.0 / multiplyBy; // make adjustment also dependent of volt scale
    } else {
      triggerLevel = triggerLevel - 100.0 / multiplyBy; // make adjustment also dependent of volt scale
    }
     DEBUG.print("ampLevel=");
    DEBUG.println(ampLevel);
  }

 

};
void DigitizerClass::rotaryEncButtonChanged(int key, bool quickPress, bool holdAfterLongPress, bool releaseAfterLongPress) {
  DEBUG.println("DigitizerClass rotaryEncChanged");
  adjustLevel = adjustLevel + 1;
  if (adjustLevel > 2) {
    adjustLevel = 0;
  }
  DEBUG.print("adjustLevel=");
  DEBUG.println(adjustLevel);
};
