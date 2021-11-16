#include "Digitizer.h"

extern ADCClass SMU[];
extern VoltDisplayClass VOLT_DISPLAY;

//OPERATION_TYPE operationType;

bool DigitizerClass::zoomed = false;
bool DigitizerClass::adjustLevel = false;
int DigitizerClass::ampLevel = 2;

DigitizerClass DIGITIZER;


void DigitizerClass::init(OPERATION_TYPE operationType_) {

}

void DigitizerClass::open() {
  GD.__end();
  SMU[0].disable_ADC_DAC_SPI_units();
  SMU[0].enableVoltageMeasurement = true;
  SMU[0].enableCurrentMeasurement = false;
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

int digitizerCheckButtonTimer = millis();
void DigitizerClass::renderGraph() {

    
      GD.Tag(171);
      GD.ColorA(255);
      GD.ColorRGB(0x000000);
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
          GD.ColorRGB(0xff0000);

    if (allowTrigger) {
       //GD.cmd_number(500,320, 28, 6, allowTrigger);
       GD.cmd_text(200, 380 ,  28, 0, "AUTO");
     } else  {
       GD.cmd_text(200, 380 ,  28, 0, "STOPPED");
     }
     
     if (adjustLevel) {
       GD.cmd_text(400, 380 ,  28, 0, "ADJ VOLT");
       GD.cmd_number(550, 380, 27, 6, ampLevel);

     } else {
       GD.cmd_text(400, 380 ,  27, 0, "ADJ TIME");
       GD.cmd_number(550, 380, 27, 6, zoomed);
     }



     if (continuous) {
       //GD.cmd_number(500,320, 28, 6, allowTrigger);
       GD.cmd_text(50, 380 ,  28, 0, "CONTINOUS");
     } else  {
       GD.cmd_text(50, 380 ,  28, 0, "EDGE");
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

GD.LineWidth(5);
GD.Begin(LINE_STRIP);
GD.Vertex2ii(xAxisPx-width/2, yAxisPx);
GD.Vertex2ii(xAxisPx+width/2, yAxisPx);
GD.Begin(LINE_STRIP);
GD.Vertex2ii(xAxisPx, yAxisPx + height/2);
GD.Vertex2ii(xAxisPx, yAxisPx - height/2);

GD.LineWidth(3);

for (int i=xStep; i<width/2; i=i+xStep ) {
    GD.Begin(LINE_STRIP);
    GD.Vertex2ii(xAxisPx + i, yAxisPx-height/2);
    GD.Vertex2ii(xAxisPx + i, yAxisPx+height/2);
    GD.Begin(LINE_STRIP);
    GD.Vertex2ii(xAxisPx - i, yAxisPx-height/2);
    GD.Vertex2ii(xAxisPx - i, yAxisPx+height/2);
}

for (int i=yStep; i<height/2; i=i+yStep) {
    GD.Begin(LINE_STRIP);
    GD.Vertex2ii(xAxisPx+width/2, yAxisPx + i);
    GD.Vertex2ii(xAxisPx-width/2, yAxisPx + i);
    GD.Begin(LINE_STRIP);
    GD.Vertex2ii(xAxisPx+width/2, yAxisPx - i);
    GD.Vertex2ii(xAxisPx-width/2, yAxisPx - i);
}

    //    VOLT_DISPLAY.renderMeasured(10,200, fullSamplePeriod, false);

    GD.LineWidth(10);
    GD.Begin(LINE_STRIP);
    GD.ColorA(255);
    GD.ColorRGB(0x00ff00);
    clearMaxMin();
    float pixelsPrVolt = ampLevel * 10;

    // render main graph
    int resolution = 2; // 1 is best
    int from = 0;
    int to = nrOfFloats;
    if (zoomed) {
        from = nrOfFloats*0.25;
        to = nrOfFloats*0.75;
        resolution = 1;
    }
    int xCoordinate = 0;
    for (int x = from; x<to; x += resolution) {
      GD.Vertex2ii(xCoordinate, yAxisPx - mva[x]/1000.0 * pixelsPrVolt);
xCoordinate +=4;
      if (mva[x] > maxDigV) {
        maxDigV = mva[x];
      } 
      if (mva[x] < minDigV) {
        minDigV = mva[x];
      } 
 
    }
GD.ColorRGB(0xffffff);
GD.cmd_text(0, yAxisPx-height/2 , 28, 0, "Max:");
GD.ColorRGB(0x00ff00);
GD.cmd_number(50,yAxisPx-height/2, 28, 6, maxDigV);
GD.cmd_text(50+80, yAxisPx-height/2 , 28, 0, "mV");

GD.ColorRGB(0xffffff);
GD.cmd_text(200, yAxisPx-height/2 , 28, 0, "Min:");
if (minDigV < 0.0000) {
  GD.ColorRGB(0x00ff00);
  GD.cmd_text(250, yAxisPx-height/2 , 28, 0, "-");
  GD.cmd_number(260,yAxisPx-height/2, 28, 6, abs(minDigV));
  GD.cmd_text(260+80, yAxisPx-height/2 , 28, 0, "mV");

} else {
  GD.ColorRGB(0x00ff00);
  GD.cmd_number(250,yAxisPx-height/2, 28, 6, abs(minDigV));
  GD.cmd_text(250+80, yAxisPx-height/2 , 28, 0, "mV");
}
    
}

void DigitizerClass::copyDataBufferToDisplayBuffer() {
    for (int x=0; x<=nrOfFloats;x++) {
       mva[x] = 0.0;
   }
   for (int x=0; x<=nrOfFloats/2;x++) {
       mva[x+nrOfFloats/2] = ramEmulator2[x];
   }

   int adr = adrAtTrigger;
   for (int x=nrOfFloats/2; x>=0 ;x--) {
       //mva[x] = 0.0;
        mva[x] = ramEmulator[adr];
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
int modulationTimer = millis();
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

  if (dataR == 1) {
    return;
  }

  if (bufferOverflow) {
      return;
  }


  float v = SMU[0].measureMilliVoltage();  

  bool zeroCross = false;
  
  bool positiveEdge = v > lastVoltage + 10.0;
  
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
      ampLevel = ampLevel - 1;
    } else {
      ampLevel = ampLevel + 1;
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
