#include "Logger.h"
#include "Arduino.h"

extern "C" uint8_t external_psram_size;

int LoggerClass::rotary = 1;
int LoggerClass::scrollRotary = 0;

bool LoggerClass::encButtonScroll = true;
bool LoggerClass::encButtonZoom = false;


void LoggerClass::init() {
//	while (!Serial) ; // wait

  // Based on using Teensy 4.1 with extra RAM mounted
  uint8_t size = external_psram_size;
	Serial.printf("EXTMEM Memory: %d Mbyte\n", size);
	if (size == 0) {
    notAvailable = true;
    Serial.println("WARNING: No external ram !!!");
    return;
  } 
  notAvailable = false;
  memory_begin = (uint32_t *)(0x70000000);
	memory_end = (uint32_t *)(0x70000000 + size * 1048576);

  clear();
   // fillBuffer(maxSize*0.8);

    	Serial.print("INFO: Buffer filled, percentage full:");
      Serial.print(percentageFull,2);
      Serial.print("samples:");
      Serial.println(samples);

//LogDataWithTimeStamp viewBuffer[100];
//getLastSamples(viewBuffer, 15, 1);
//render();

}

void LoggerClass::clear() {
  minimum = undefinedValue;
  maximum = undefinedValue;
  samples = 0;
  address = 0;
  scrollAddress = 0;
  full = false;
  empty = true;
  percentageFull = 0.0;
  totalSamplesCounted = 0;
  autoScrolling=true;
  scrollRotary = 0;
}

int oldRot = 0;
void LoggerClass::registerValue2(float value) {
  if (notAvailable) {
    return;
  }
  if (millis()< waitLogStart) {
    return;
  }
 LogDataWithTimeStamp logData;
 logData.value=value;
 logData.timestamp= millis();
 registerValue(logData);


// --- scaling option---
if (encButtonScroll == false) {

scrollAddress = address;
 if (oldRot != rotary) {
    updateViewData(samplesPrViewPoint);
    oldRot = rotary;
 } else {
   samplesPrViewPoint = rotary;
 }
}




//  --- manual scroll option---
if (encButtonScroll == true) {
if (autoScrolling) {
    scrollAddress = address;
}
// Check if rotary operation has been performed
if (oldRot != scrollRotary) {
    // yes, rotary has changed
    if (autoScrolling) {
      // If at the latest address, stop autoscroll
      scrollAddress = address - scrollRotary;
      autoScrolling=false;
    } else {
      // update the manual scroll address based on rotary movement
      scrollAddress = scrollAddress - (scrollRotary - oldRot);

    }
    // Check if manual scroll is at the latest sample
    bool future = ((int)address - (int)scrollAddress)<=0;  
    if (!autoScrolling && future) {
      // yes, then start autoscroll from lates sample again
      autoScrolling = true;
      scrollRotary = 0;
      scrollAddress = address;
    } 

    oldRot = scrollRotary;
 }
    // Serial.print("Address:");
    // Serial.print(address);
    // Serial.print(", ScrollAddress");
    // Serial.print(scrollAddress);
    // Serial.print(", scrollRotary:");
    // Serial.println(scrollRotary);




  // if (scrollAddress>=address) {
  //   scrollAddress = address;
  //   scrolling = true;
  //   scrollRotary = 0;

  // }
}




 // Important the the update of view data is "in sync" with number of samples used.
 // If not, the grah can look like it's shivering...
 if (totalSamplesCounted%samplesPrViewPoint == 0) {
   updateViewData(samplesPrViewPoint);
 }

}

void LoggerClass::registerValue(LogDataWithTimeStamp logData) {

         //samplesPrViewPoint = rotary;

  totalSamplesCounted ++;
  if (!empty) {
    address ++;
  }

  if (address >= maxSize) {
    address = 0;
    full = true;
  } 
  
  if (!full) {
    samples++;
  }
  
  percentageFull = 100.0*((float)samples/(float)maxSize);
  LogDataWithTimeStamp *adrP = (LogDataWithTimeStamp*)(memory_begin);
  // Serial.print("Store ");
  // Serial.print(logData.value,3);
  // Serial.print(" at address ");
  // Serial.print(address);
  // Serial.print(", Using ");
  // Serial.print(percentageFull,1);
  // Serial.println("% of log buffer");



  adrP[address++] = logData;

  // Don't trust the first few samples...
  if ((minimum == undefinedValue || maximum == undefinedValue) && samples>10 ) {
    minimum = logData.value;
    maximum = logData.value;
  } else if (logData.value < minimum) {
    minimum = logData.value;
  } else if (logData.value > maximum) {
    maximum = logData.value;
  }
   
}


LogDataWithTimeStamp viewBuffer[MAX_SAMPLES_IN_DISPLAY_BUFFER];
bool noViewBuffer = true;
void LoggerClass::updateViewData(int samplesPr) {

  getLastSamples(viewBuffer, nrOfWindowSamples, samplesPr);
  noViewBuffer = false;
  // for (int i=0;i<nrOfWindowSamples;i++) {
  //   Serial.print("Data:");
  //   Serial.print(viewBuffer[i].value,3);
  //   Serial.print(",timestamp:");
  //   Serial.println(viewBuffer[i].timestamp);
  // }
  // Serial.print("MAX:");
  // Serial.print(loggerMaxValue);
  // Serial.print(",MIN:");
  // Serial.println(loggerMinValue);

}

void LoggerClass::loop() {

   GD.LineWidth(5);
 GD.ColorA(255);
  GD.ColorRGB(0xdddddd);
 for (int y=80;y<=80+300;y=y+30) {
 
  GD.Begin(LINE_STRIP);
  GD.Vertex2ii(150, y);
  GD.Vertex2ii(758, y);
 }
  

  if (noViewBuffer) {
    return;
  }
  //LogDataWithTimeStamp buffer[300];
  //int nrOfWindowSamples = 5;
  //getLastSamples(buffer, nrOfWindowSamples, 1);

  if (nrOfWindowSamples>200) {
    GD.LineWidth(15);
  } else {
    GD.LineWidth(20);
  }
  GD.Begin(LINE_STRIP);
  GD.ColorA(255);
  GD.ColorRGB(0x00ff00);
float mid;
  for (int i=0;i<nrOfWindowSamples-1;i++) {
    float v = viewBuffer[i].value;
    //Serial.print("val:");
    //Serial.println(v,3);
    //loggerMaxValue =1.2330;
    //loggerMinValue =1.2350;
    float span = loggerMaxValue - loggerMinValue;
    
  
    mid = loggerMaxValue - (span/2.0);

    float y =  (mid - v) *300.0 / span;
    
    if (i > nrOfWindowSamples - 5) {
          GD.ColorA(200);;
    }
    else if (i > nrOfWindowSamples -3) {
          GD.ColorA(150);

    }
    if (viewBuffer[i].timestamp != 0) {
      // nrOfWindowSamples should be so 600/nrOfWindowSamples should be an integer
      GD.Vertex2ii(753 - i * (600/nrOfWindowSamples), 232 + (int)y);
    }
    
  }
  //delay(100);
  GD.ColorA(255);


  GD.ColorRGB(0xffffff);

  DIGIT_UTIL.renderValue(0,  80-10 ,loggerMaxValue, 1, 1); 
  DIGIT_UTIL.renderValue(0,  80+150-10,mid, 1, 1); 
  DIGIT_UTIL.renderValue(0,  80+300-10 ,loggerMinValue, 1, 1); 


  GD.cmd_number(10,420, 28, 6, (int)rotary);
    GD.cmd_number(100,420, 28, 6, scrollRotary);

  if (encButtonZoom) {
   GD.cmd_text(250, 410 ,   30, 0, "zoom");

  } 
  if (encButtonScroll) {
   GD.cmd_text(250, 410 ,   30, 0, "scroll");
  }



  //IGIT_UTIL.renderValue(10,  80+70 ,rotary*100, 2, 0); 

for (int i=0;i<300;i=i+49) {

  
  int t = viewBuffer[i].timestamp;
  if (t==0) {
    break;
  }
  unsigned long allSeconds=t/1000;
  int runHours= allSeconds/3600;
  int secsRemaining=allSeconds%3600;
  int runMinutes=secsRemaining/60;
  int runSeconds=secsRemaining%60;

  //char buf[21];
  //sprintf(buf,"Runtime%02d:%02d:%02d",runHours,runMinutes,runSeconds);
  //Serial.println(buf);
  GD.ColorRGB(0xdddddd);

  GD.cmd_number(150+600-i*(600/nrOfWindowSamples)-30,400, 27, 2, runHours);
  GD.cmd_text(150+600-i*(600/nrOfWindowSamples)+20-30, 400 ,   27, 0, ":");

  GD.cmd_number(150+600-i*(600/nrOfWindowSamples)+ 25-30,400, 27, 2, runMinutes);
  GD.cmd_text(150+600-i*(600/nrOfWindowSamples)+45-30, 400 ,   27, 0, ":");

  GD.cmd_number(150+600-i*(600/nrOfWindowSamples)+ 50-30 ,400, 27, 2, runSeconds);


  GD.LineWidth(5);
  GD.ColorA(255);
  GD.ColorRGB(0xdddddd);
 
  GD.Begin(LINE_STRIP);
  GD.Vertex2ii(160+600-i*(600/nrOfWindowSamples)+ 25-30,80);
  GD.Vertex2ii(160+600-i*(600/nrOfWindowSamples)+ 25-30,380);

  }

}


// void LoggerClass::render() {
  
//   LogDataWithTimeStamp buffer[300];
//   int nrOfWindowSamples = 100;
//   getLastSamples(buffer, nrOfWindowSamples, 5);
//   for (int i=0;i<nrOfWindowSamples;i++) {
//     Serial.print("Data:");
//     Serial.print(buffer[i].value);
//     Serial.print(",timestamp:");
//     Serial.println(buffer[i].timestamp);
//   }

// }

bool scrolling=true;
uint32_t backlash = 0;
int LoggerClass::getLastSamples(LogDataWithTimeStamp *buffer, int bufferSize, int samplesPr) {
  LogDataWithTimeStamp *adrP = (LogDataWithTimeStamp*)(memory_begin);
bool serial = false;
bool fillEmpty = false;

// readjust max/min. Comment out if you want it to "remember"
loggerMaxValue = -10000.0;
loggerMinValue = 10000.0;
  
  
  

uint32_t lastSampleAdr = scrollAddress; //address;
/*
  uint32_t older = rotary;
  uint32_t lastSampleAdr = address - older; // - backlash;

  if (lastSampleAdr != address) {
    backlash = address - lastSampleAdr; 
    scrolling=false;
  } else {
    lastSampleAdr = address - older;
    scrolling = true;
  }
  */
  
  

  // if (!scrolling && lastSampleAdr - older +backlash >= address) {
  //   scrolling = true;
  //   lastSampleAdr = address;
  // }

/*
  if (lastSampleAdr+older+backlash <=address) {
    lastSampleAdr = address - older; //start from latest, meaning autoscrolling
    scrolling=false;
    backlash=lastSampleAdr-address;
    if (backlash = 0) {

    }
  } else {
    backlash=0;
    //scrolling = true;
    //lastSampleAdr = address;
  }
  if (scrolling) {
        lastSampleAdr = address;

  }
   */

  // Hack to allow scrolling in history.
  // Only works if buffer is not full.
  // Needs currect logic to select correct address from buffer if full...
  // Works as a poor man's scroll...
  //if (!full && lastSampleAdr>older) {
  //  lastSampleAdr = lastSampleAdr-older;
 // }




  for (int i=0;i<bufferSize;i++) {
    
    float mean = 0.0;
    float max = -999999.99;
    for (int s=0; s<samplesPr; s++){
      if (lastSampleAdr == 0 && full) {
        lastSampleAdr = maxSize-1;
        //buffer[i] = adrP[lastSampleAdr];
        Serial.print(" RUNDING ");
        //break;
      } else if (lastSampleAdr == 0 && !full) {
         buffer[i].value = -999999.99;
         buffer[i].timestamp = 0; 
         fillEmpty=true;
         //Serial.println(" NO_MORE ");
         
         //break;
      } else {
        //buffer[i] = adrP[lastSampleAdr];
        lastSampleAdr--;
      }
      if (serial) {
        Serial.print("Fetching from address=");
        Serial.print(lastSampleAdr);
        Serial.print(",data");
        Serial.println(adrP[lastSampleAdr].value,2);
      }

      
      if (adrP[lastSampleAdr].value > max) {
        max = adrP[lastSampleAdr].value;
      } 
      //adrP[lastSampleAdr].value
      mean = mean + adrP[lastSampleAdr].value ;

    }

    
    //float value = adrP[lastSampleAdr].value;
    float value = mean/samplesPr;
    //float value = max; 

    if (!fillEmpty) {
      buffer[i].value = value ;
      buffer[i].timestamp = adrP[lastSampleAdr].timestamp;

      if (value > loggerMaxValue) {
        loggerMaxValue = value;
      }
      if (value < loggerMinValue) {
        loggerMinValue = value;
      }

 
    }
  
  }
       if (loggerMaxValue - loggerMinValue <0.020) {
         float valueLeft = 0.020 - (loggerMaxValue-loggerMinValue);
         loggerMaxValue = loggerMaxValue + valueLeft/2.0;
         loggerMinValue = loggerMinValue - valueLeft/2.0;
       }
  
}

void LoggerClass::fillBuffer(int nrOfValues) {
 for (int i = 0 ; i< nrOfValues; i++) {
    LogDataWithTimeStamp d;
    d.value=i;
    d.timestamp = i*100;
    registerValue(d);

    //Serial.print("percentagefull:");
    //Serial.println(percentageFull,1);

  }
}

void LoggerClass::printBuffer() {
 



  for (uint32_t adr = 0; adr<maxSize;adr++) {
   LogDataWithTimeStamp *adrP = (LogDataWithTimeStamp*)(memory_begin);
    Serial.print("Time:");
    Serial.print(adrP[adr].timestamp);
    Serial.print(",value:");
    Serial.print(adrP[adr].value);
    Serial.println(",");
    delay(100);
  }
}


  void LoggerClass::rotaryChangedFn(float changeVal) {
     Serial.println("LOGGER ENCODED ROTATION DETECTED");
     Serial.print(".CHANGEDVAL:");
     Serial.println(changeVal,2);
     
     // Override dynamic speed for now...
    //  if (changeVal < 0) {
    //    changeVal = -0.1;
    //  } else {
    //   changeVal = 0.1; 
    //  }
     

     
    // for manual scroll
if (encButtonScroll == true) {

      if (changeVal>10) {
        changeVal = 10;
      }
     scrollRotary = scrollRotary + changeVal * 10;
}

    // for zoom 
if (encButtonScroll == false) {

   rotary = rotary + changeVal*10;
     if (rotary<=1) {
        rotary=1;
    }
}
  }


/*
int LoggerClass::next(uint32_t adr_) {
  uint32_t adr = adr_;
  adr = adr -1;
  //Serial.print(adr);
  if (adr < 0 && full) {
    adr = maxLogAddress;
    //Serial.print("!!!!!!!!!!!!!!!  1 ");
    //Serial.print("Adr is");
   // Serial.print(adr);
  }
  if (adr < 0 && !full) {
    adr = -1; //currentLogAddress;
    //Serial.print("!!!!!!!!!!!!!!!  2");
  }
  return adr;
}
*/



void LoggerClass::rotarySwitchFn(int key, bool quickPress, bool holdAfterLongPress, bool releaseAfterLongPress) {
  if (encButtonScroll) {
    encButtonScroll=false;
    encButtonZoom = true;
  } else {
    encButtonScroll=true;
    encButtonZoom = false;
  }

  Serial.print("Logger rotarySwitch operated");
  Serial.println(quickPress==true?"QUICK" : "");
  Serial.println(holdAfterLongPress==true?"HOLDING" : "");
  Serial.println(releaseAfterLongPress==true?"RELEASED AFTER HOLDING" : "");
}