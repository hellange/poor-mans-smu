#include "Arduino.h"
#include "PushButtons.h"
#include "GD2.h"
#include "TrendGraph.h"


TrendGraphClass TRENDGRAPH;

void TrendGraphClass::init() {
  if (RAM.initFailure) {
    Serial.print("Init TrendGraph not working because of initial RAM failure!");
  } else {
    Serial.print("Init TrendGraph");
  }
}

int lastAdjust=millis();

void TrendGraphClass::loop(OPERATION_TYPE operationType) {



 //int logAddress = RAM.getCurrentLogAddress();
 //int pixels = 400;
 //int max = RAM.getMaxLogAddress();
 GD.LineWidth(20);
  GD.Begin(LINE_STRIP);
  GD.ColorA(255);
  GD.ColorRGB(0xff0000);
  //uint16_t logAdr = logAddress;
 float maxV = -1000000.0;
 float minV = 1000000.0;
 int x = 0;
 float span = 0.0;


 int adr = RAM.getCurrentLogAddress();
 //for (int adr = logAddress<200 ? 0: logAddress - 200; adr<logAddress; adr++) {
 for (int i = 0; i< 200; i++) {

 timedLog logData;
   GD.__end();

   logData = RAM.readLogData(adr);
   GD.resume();
   
   float v = logData.value.val;
   
   if (RAM.useNormalRam()) {
     if (v < 50000.0) {
       if (v>maxV) {
       //Serial.print("Registred max:");
       //Serial.println(v,3);
       maxV = v;
       }
       if (v<minV) {
         minV = v;
       }
       
     }
   } else {
     maxV = RAM.max;
     minV = RAM.min;
   }
   

    adr = RAM.nextAdr(adr);
    if (adr == -1) {
     break;
    }

 }


    //if (lastAdjust + 5000 > millis()) {
     if (maxV - minV <0.020) {
       float valueLeft = 0.020 - (maxV-minV);
       maxV = maxV + valueLeft/2.0;
       minV = minV - valueLeft/2.0;
     }
     //lastAdjust = millis();
    //}


  GD.ColorRGB(0xffffff);

 float mid = 0.0;
 adr = RAM.getCurrentLogAddress();

 //for (int adr = logAddress<200 ? 0: logAddress - 200; adr<logAddress; adr++) {
 for (int i = 0; i< 200; i++) {

    adr = RAM.nextAdr(adr);
    if (adr == -1) {
      continue;
    }

    
   timedLog logData;
   GD.__end();

   logData = RAM.readLogData(adr);
   GD.resume();
   
   float v = logData.value.val;

   span = maxV - minV;
    
  
   mid = maxV - (span/2.0);

   float y =  (mid - v) *300.0 / span;
   


   
   GD.Vertex2ii(150 + 600-x, 240 + (int)y);
   x=x+3;


 }
 //VOLT_DISPLAY.renderMeasured(100,200, span);


 GD.LineWidth(5);
 GD.ColorA(255);
  GD.ColorRGB(0x00ff00);
 for (int y=80;y<=80+300;y=y+30) {
 
  GD.Begin(LINE_STRIP);
  GD.Vertex2ii(160, y);
  GD.Vertex2ii(750, y);
 }
   GD.ColorRGB(0xffffff);

DIGIT_UTIL.renderValue(10,  80-10 ,maxV, 1, 1); 
DIGIT_UTIL.renderValue(10,  80+150-10,mid, 1, 1); 
//DIGIT_UTIL.renderValue(15,  80+150+30-10 ,span, 1, 1); 
DIGIT_UTIL.renderValue(10,  80+300-10 ,minV, 1, 1); 

 //VOLT_DISPLAY.renderMeasured(100,10, maxV);
 //VOLT_DISPLAY.renderMeasured(100,400, minV);



 if (RAM.initFailure) {
     GD.ColorRGB(0xff0000);

     GD.cmd_text(340,200,28,0,"ERROR: RAM NOT WORKING !");

  }

  GD.ColorRGB(0xffffff);

 
 x = 0;
  adr = RAM.getCurrentLogAddress();

 //for (int adr = logAddress<200 ? 0: logAddress - 200; adr<logAddress; adr=adr+40) {   
for (int i = 0; i< 200; i++) {

    adr = RAM.nextAdr(adr);
    if (adr == -1) {
      continue;
    }
    if (adr%40 == 0) {
    
 timedLog logData;
   GD.__end();

   logData = RAM.readLogData(adr);
   GD.resume();
   //float volt = logData.value.val;
   uint32_t t = logData.time.val;




unsigned long allSeconds=t/1000;
int runHours= allSeconds/3600;
int secsRemaining=allSeconds%3600;
int runMinutes=secsRemaining/60;
int runSeconds=secsRemaining%60;

//char buf[21];
//sprintf(buf,"Runtime%02d:%02d:%02d",runHours,runMinutes,runSeconds);
//Serial.println(buf);


  GD.cmd_number(150+600-x-30,400, 27, 2, runHours);
     GD.cmd_text(150+600-x+20-30, 400 ,   27, 0, ":");

  GD.cmd_number(150+600-x+ 25-30,400, 27, 2, runMinutes);
       GD.cmd_text(150+600-x+45-30, 400 ,   27, 0, ":");

  GD.cmd_number(150+600-x+ 50-30 ,400, 27, 2, runSeconds);

  //GD.cmd_number(150+600-x,400, 27, 0, t/1000);
   x=x+3*40;
    }
  
 }



}
  void TrendGraphClass::rotaryChangedFn(float changeVal) {
     Serial.println("TRENDGRAPH ENCODED ROTATION DETECTED");
  }