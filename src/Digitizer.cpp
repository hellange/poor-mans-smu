#include "Digitizer.h"

extern ADCClass SMU[];
extern VoltDisplayClass VOLT_DISPLAY;

//OPERATION_TYPE operationType;

float maxDigV = -100000.00, minDigV = 100000.00, curDigV;
float maxDigI = -100000.00, minDigI = 100000.00, curDigI;
int ramAdrPtr = 0;
int nrOfFloats = 400;
bool triggered = false;
int digitizeDuration = millis();
float lastVoltage = 0.0;
float ramEmulator[1000];
float ramEmulator2[1000];

int digitizeCounter = 0;
int dummyCounter=0;

void DigitizerClass::init(OPERATION_TYPE operationType_) {

}

bool DigitizerClass::loopDigitize() {
  //operationType = getOperationType();
  SMU[0].disable_ADC_DAC_SPI_units();
  
  if (digitize == false && !MAINMENU.active){   
    ramAdrPtr = 0;
    minDigV = 1000000;
    maxDigV = - 1000000;
    minDigI = 1000000;
    maxDigI = - 1000000;
    SMU[0].setSamplingRate(31250); //31250
    nrOfFloats = 400;
    digitize = true;
    bufferOverflow = false;     
  }
  //SMU[0].disable_ADC_DAC_SPI_units();
  if (bufferOverflow == true) {
    digitize = false;
    Serial.println("Initiate new samping round...");
  }
  //TODO: Move digitizine to a separate class
  if (bufferOverflow == false && digitize == true){
  } else {
    bufferOverflow = false;
    SMU[0].disable_ADC_DAC_SPI_units();
    GD.resume();
    GD.Clear();

    GD.Tag(171);
       GD.cmd_button(0,350,95,50,29,0, "trigger");
       int tag = GD.inputs.tag;
       if (tag == 171) {
         DIGITIZER.allowTrigger++;
       }

    //VOLT_DISPLAY.renderMeasured(10,50, minDigV, false);
    //VOLT_DISPLAY.renderMeasured(10,150, maxDigV, false);
    //  CURRENT_DISPLAY.renderMeasured(10,250, minDigI, false, false,current_range);
    //  CURRENT_DISPLAY.renderMeasured(10,350, maxDigI, false, false, current_range);
    //GD.cmd_number(10,320, 28, 6, digitizeCounter);
    //GD.cmd_number(210,320, 28, 6, adrAtTrigger);
    //GD.cmd_number(410,320, 28, 6, ramAdrPtr);
    
    GD.cmd_number(500,320, 28, 6, allowTrigger);


int yAxisPx = 220;
int xAxisPx = 400;

    GD.Begin(LINE_STRIP);
    GD.ColorA(255);
    GD.ColorRGB(0x0000ff);
    GD.Vertex2ii(xAxisPx, 20);
    GD.Vertex2ii(xAxisPx, 400);
    GD.Begin(LINE_STRIP);
    GD.Vertex2ii(0, yAxisPx);
    GD.Vertex2ii(800, yAxisPx);


    //    VOLT_DISPLAY.renderMeasured(10,200, fullSamplePeriod, false);
    digitizeCounter = 0;
    GD.Begin(LINE_STRIP);
    GD.ColorA(255);
    GD.ColorRGB(0x00ff00);

    float mva[400];
    float mia[400];
/*
    for (int x = 0; x<nrOfFloats; x++) {
      int adr = x*8;
      mva[x] = ramEmulator[adr/4]; //RAM.readRAMfloat(adr);
      mia[x] = ramEmulator[adr/4 + 1]; //RAM.readRAMfloat(adr+4);
    }
    */
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
  
    for (int x = 0; x<nrOfFloats; x++) {

      if (minDigV < 100000) {
        //float span = (maxDig - minDig);
        float mid = (maxDigV + minDigV) /2.0;
        float relative = mid - mva[x]; 
        float y = relative*20;
        // float y = mva[x] / 100.0;
        GD.Vertex2ii(x*2, yAxisPx - mva[x]/1000.0 * 50.0);
      }  
    }
    return true; // must end GD
  }
  return false;
}


void DigitizerClass::a() {

}

bool DigitizerClass::handleSamplingForDigitizer(int dataR) {
//   if ( !  (digitize == true && bufferOverflow==false && (dataR == 0 or dataR == 1))) {
//     //DIGITIZER.handleSamplingForDigitizer(dataR);
//     return false;
//   }

    
//     if (count % 20== 0) {
//       SMU[0].fltSetCommitVoltageSource(sampleVolt, false);
//       if (sampleVolt == 2000.0) {
//        sampleVolt = -2000.0;
//       } else {
//        sampleVolt = 2000.0;
//       }
//     }
//     count ++;
  dummyCounter++;

  if (dataR == 1) {
    return true;
  }
//     if (dataR == 1) {
//      float i = SMU[0].measureCurrent(AMP1);   
//      RAM.writeRAMfloat(ramAdrPtr, i);
//      ramAdrPtr += 4;
//      if (ramAdrPtr > maxFloats*4) {
//        bufferOverflow = true;
//      }
//      //Serial.println(i);
//      curDigI = i;
//      if (i > maxDigI) {
//        maxDigI= i;
//      } 
//      if (i < minDigI) {
//        minDigI = i;
//      }
//      
//      return;
//     }


  digitizeCounter ++;

  float v = SMU[0].measureMilliVoltage();  
       // float v = SMU[0].measureCurrent(AMP1);   

   //v=100.0 + random(20)/100.0;
   //simulatedWaveform += 0.05;
   // v = v + sin(simulatedWaveform)*5.0;
    


  bool zeroCross = lastVoltage < 0.0 && v > 0.0 ;
  bool positiveEdge = v > lastVoltage + 10.0;
  countSinceLastSample ++;
  if (countSinceLastSample<nrOfFloats) {
      positiveEdge = false; // dont trust edge detection before some samples after last full buffer
  }

  if (triggered) {
      ramEmulator2[samplesAfterTrigger++] = v;
  }

  bool continuous = false;
  if (!triggered) {
    if (continuous or positiveEdge) {
      triggered = true;
      adrAtTrigger = ramAdrPtr;
      //Serial.println("Triggered!");
      } else {
       lastVoltage = v;         
       //return true;
      }
    }

    lastVoltage = v;

    //RAM.writeRAMfloat(ramAdrPtr, v);
    ramEmulator[ramAdrPtr] = v;
    if (ramAdrPtr == 0) {
      digitizeDuration = millis();
    }
    ramAdrPtr ++;
    if (ramAdrPtr > nrOfFloats) {
        ramAdrPtr = 0;
    }
    //if (ramAdrPtr > nrOfFloats*4 *2) {
    if (samplesAfterTrigger == nrOfFloats/2) {
      bufferOverflow = true;
      triggered = false;
      samplesAfterTrigger = 0;
      countSinceLastSample = 0;
      lastVoltage=0.0;

    }
    curDigV = v;
    if (v > maxDigV) {
      maxDigV = v;
    } 
    if (v < minDigV) {
      minDigV = v;
    }      

  
    return true;
  }