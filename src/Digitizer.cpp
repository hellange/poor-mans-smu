#include "Digitizer.h"

extern ADCClass SMU[];
extern VoltDisplayClass VOLT_DISPLAY;

//OPERATION_TYPE operationType;

bool DigitizerClass::zoomed = false;
bool DigitizerClass::adjustLevel = false;
int DigitizerClass::ampLevel = 1;

DigitizerClass DIGITIZER;

bool digitizeVoltage = false;

void DigitizerClass::init(OPERATION_TYPE operationType_) {

}

void DigitizerClass::open() {
  GD.__end();
  prevSamplingRate = SMU[0].getSamplingRate();
  SMU[0].disable_ADC_DAC_SPI_units();
  if (digitizeVoltage) {
    SMU[0].enableVoltageMeasurement = true;
    SMU[0].enableCurrentMeasurement = false;
  } else {
    SMU[0].enableVoltageMeasurement = false;
    SMU[0].enableCurrentMeasurement = true;
  }

  SMU[0].updateSettings();
  GD.resume();
}

void DigitizerClass::close() {
    // TODO: Check that graphics are not "on" ???
    GD.__end();
    SMU[0].disable_ADC_DAC_SPI_units();
    //SMU[0].setSamplingRate(20); //TODO: Should get back to same as before, not a default one
    SMU[0].enableVoltageMeasurement = true;
    SMU[0].enableCurrentMeasurement = true;
    SMU[0].updateSettings();
    SMU[0].setSamplingRate(prevSamplingRate); //TODO: Should get back to same as before, not a default one


    digitize = false;
    bufferOverflow = false;
}

void DigitizerClass::loopDigitize() {
  //operationType = getOperationType();
  
  if (digitize == false && !MAINMENU.active){
        SMU[0].disable_ADC_DAC_SPI_units();
   
    //minDigV = 1000000;
    //maxDigV = - 1000000;
    //minDigI = 1000000;
    //maxDigI = - 1000000;
    SMU[0].setSamplingRate(31250); //31250
    digitize = true;
    bufferOverflow = false;     
  }
  if (bufferOverflow == true) {
    digitize = false;
  }
 
}

unsigned long digitizerCheckButtonTimer = millis();
void DigitizerClass::renderGraph() {

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

      GD.Tag(0);

      int tag = GD.inputs.tag;
      if (digitizerCheckButtonTimer+500 < millis()) {
        digitizerCheckButtonTimer = millis();
        if (tag == 171) {
          DIGITIZER.allowTrigger = ! DIGITIZER.allowTrigger ;
        }
        if (tag == 172) {
          continuous = !continuous;
        }
      }
    

    //VOLT_DISPLAY.renderMeasured(10,50, minDigV, false);
    //VOLT_DISPLAY.renderMeasured(10,150, maxDigV, false);
    //  CURRENT_DISPLAY.renderMeasured(10,250, minDigI, false, false,current_range);
    //  CURRENT_DISPLAY.renderMeasured(10,350, maxDigI, false, false, current_range);
    //GD.cmd_number(210,320, 28, 6, adrAtTrigger);
    //GD.cmd_number(410,320, 28, 6, ramAdrPtr);

    if (allowTrigger) {
       //GD.cmd_number(500,320, 28, 6, allowTrigger);
       GD.ColorRGB(0x00ff00);
       GD.cmd_text(200, 380 ,  28, 0, "AUTO");
     } else  {
       GD.ColorRGB(0xff0000);
       GD.cmd_text(200, 380 ,  28, 0, "STOPPED");
     }
     
     if (adjustLevel) {
       GD.ColorRGB(0x00ff00);
       GD.cmd_text(400, 380 ,  28, 0, "ADJ VOLT");
       GD.cmd_number(550, 380, 28, 6, ampLevel);
     } else {
       GD.ColorRGB(0xff0000);
       GD.cmd_text(400, 380 ,  28, 0, "ADJ TIME");
       GD.cmd_number(550, 380, 28, 6, zoomed);
     }



     if (continuous) {
       //GD.cmd_number(500,320, 28, 6, allowTrigger);
       GD.ColorRGB(0x0000ff);
       GD.cmd_text(50, 380 ,  28, 0, "CONTINOUS");
     } else  {
       GD.ColorRGB(0x00ff00);
       GD.cmd_text(50, 380 ,  28, 0, "EDGE");
     }


 
      GD.ColorA(100);
      GD.ColorRGB(COLOR_CURRENT);

      DIGIT_UTIL.renderValue(170,  32 , v, 3, DigitUtilClass::typeCurrent); 
}


int yAxisPx = 240;
int xAxisPx = 400;
int height = 400;
int width = 790;
int xStep = 100;
int yStep = 75;

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
    int multiplyBy = 1;
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

    int resolution = 2; // 1 is best
    int from = 0;
    int to = nrOfFloats;
    if (zoomed) {
        from = nrOfFloats*0.25;
        to = nrOfFloats*0.75;
        resolution = 1;
    }

    // experimental small graph with higher resolution
    /*
    GD.ColorRGB(0x0000ff);
    GD.Begin(LINE_STRIP);
    int xCoordinate2 = 330;
    int from2 = nrOfFloats*0.30;
    int to2 = nrOfFloats*0.70;
    for (int x = from2; x<to2; x += 1) {
      GD.Vertex2ii(xCoordinate2, -100 + yAxisPx - mva[x]/1000.0 * pixelsPrVolt);
      xCoordinate2 +=1;
    }
    */

    //main graph
    GD.ColorRGB(COLOR_CURRENT);
    GD.Begin(LINE_STRIP);
    int xCoordinate = 0;
    for (int x = from; x<to; x += resolution) {
      GD.Vertex2ii(xCoordinate, yAxisPx - mva[x]/1000.0 * pixelsPrVolt);
      xCoordinate +=4;
      //updateMaxMin(mva[x]);
    }

    if (!MAINMENU.active) {

 GD.ColorRGB(0xaaaaaa);
GD.cmd_text(0, yAxisPx-height/2 -10 , 27, 0, "Max:");
GD.ColorRGB(COLOR_CURRENT);
GD.cmd_number(50,yAxisPx-height/2 -10, 27, 6, maxDigV);
GD.cmd_text(50+70, yAxisPx-height/2 -10 , 27, 0, "mA");

GD.ColorRGB(0xaaaaaa);
GD.cmd_text(0, yAxisPx-height/2 +20 -10, 27, 0, "Min:");
if (minDigV < 0.0000) {
  GD.ColorRGB(COLOR_CURRENT);
  GD.cmd_text(50, yAxisPx-height/2+20 -10 , 27, 0, "-");
  GD.cmd_number(60,yAxisPx-height/2+20 -10, 27, 6, abs(minDigV));
  GD.cmd_text(60+70, yAxisPx-height/2+20 -10 , 27, 0, "mA");

} else {
  GD.ColorRGB(COLOR_CURRENT);
  GD.cmd_number(50,yAxisPx-height/2+20 -10, 27, 6, abs(minDigV));
  GD.cmd_text(50+70, yAxisPx-height/2+20 -10 , 27, 0, "mA");
}
    }
    
 // render scale digits
int time = 0;
int digits = 0;
int step =50;
int axisScew = 5;
int yAxis = yAxisPx + 20;
GD.ColorRGB(0xaaaaaa);
// x-axis digits (minus time)
for (int x = xAxisPx; x > xAxisPx-width/2; x=x-step) {
    if (time != 0) {
        GD.cmd_text(x-20 - axisScew, yAxis, 26, 0, "-");
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
digits = 4;
int xAxisScew = 100;
int val = valStep;
// positive
for (int y= yAxisPx-step; y > yAxisPx-height/2; y=y-step) {
    GD.ColorRGB(0xaaaaaa);
    GD.cmd_number(15, y-axisScew, 26, digits, val * 100);
    
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
    GD.cmd_number(15, y-axisScew, 26, digits, val* 100);

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
   for (int x=0; x<=nrOfFloats/2;x++) {
       mva[x+nrOfFloats/2] = ramEmulator2[x];
       updateMaxMin(ramEmulator2[x]); // Note: uses all samples, but extreme may be masked out by display only some

   }

   int adr = adrAtTrigger;
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
      v = SMU[0].measureMilliVoltage();
  } else {
      v = SMU[0].measureCurrent(AMP1);
      //v = SMU[0].measureMilliVoltage();

  }

  bool zeroCross = false;
  
  float edgeSteepnessToDetect = 1.0;
  bool positiveEdge = v > lastVoltage + edgeSteepnessToDetect 
        && lastVoltage > lastVoltageOld + edgeSteepnessToDetect
        && lastVoltageOld > lastVoltageOld2 + edgeSteepnessToDetect;
  
  bool trigg = positiveEdge || continuous;



    //  SIMULATE WAVEFORM
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

      copyDataBufferToDisplayBuffer();

      //for test
      updateModulation();
    }

  

  }

void DigitizerClass::rotaryEncChanged(float changeValue) {
  Serial.print("DigitizerClass rotaryEncChanged, value:");
  Serial.println(changeValue);
  if (!adjustLevel) {
    zoomed = !zoomed;
    Serial.print("zoomed=");
    Serial.println(zoomed);
  } else {
    if (changeValue < 0) {
      ampLevel = (ampLevel>1)?ampLevel - 1 : 1;
    } else {
      ampLevel = (ampLevel<4)?ampLevel + 1 : 4;
    }
     Serial.print("ampLevel=");
    Serial.println(ampLevel);
  }

 

};
void DigitizerClass::rotaryEncButtonChanged(int key, bool quickPress, bool holdAfterLongPress, bool releaseAfterLongPress) {
  Serial.println("DigitizerClass rotaryEncChanged");
  adjustLevel = !adjustLevel;
  Serial.print("adjustLevel=");
  Serial.println(adjustLevel);
};
