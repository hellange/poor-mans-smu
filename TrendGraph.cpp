#include "Arduino.h"
#include "PushButtons.h"
#include "GD2.h"
#include "TrendGraph.h"


TrendGraphClass TRENDGRAPH;

void TrendGraphClass::init() {
  Serial.print("Init TrendGraph");
}




void TrendGraphClass::loop(OPERATION_TYPE operationType) {

 int logAddress = RAM.getCurrentLogAddress();
 int pixels = 400;
 int max = RAM.getMaxLogAddress();
 GD.LineWidth(20);
  GD.Begin(LINE_STRIP);
  GD.ColorA(255);
  GD.ColorRGB(0xff0000);
  uint16_t logAdr = logAddress;
 float maxV = -1000000.0;
 float minV = 1000000.0;
 int x = 0;
 float span;


 int adr = RAM.getCurrentLogAddress();
 //for (int adr = logAddress<200 ? 0: logAddress - 200; adr<logAddress; adr++) {
 for (int i = 0; i< 200; i++) {

 timedLog logData;
   GD.__end();

   logData = RAM.readLogData(adr);
   GD.resume();
   
   float v = logData.value.val;
   if (v>maxV) {
    maxV = v;
   } else if (v<minV) {
    minV = v;
   }
    adr = RAM.nextAdr(adr);
    if (adr == -1) {
    //  break;
    }

 }

  GD.ColorRGB(0xffffff);

 float mid;
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


DIGIT_UTIL.renderValue(10,  80 ,maxV, 1, 1); 
DIGIT_UTIL.renderValue(10,  80+150 ,mid, 1, 1); 
DIGIT_UTIL.renderValue(10,  80+150+30 ,span, 1, 1); 
DIGIT_UTIL.renderValue(10,  80+300 ,minV, 1, 1); 

 //VOLT_DISPLAY.renderMeasured(100,10, maxV);
 //VOLT_DISPLAY.renderMeasured(100,400, minV);

 
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
   float volt = logData.value.val;
   uint32_t t = logData.time.val;




unsigned long allSeconds=t/1000;
int runHours= allSeconds/3600;
int secsRemaining=allSeconds%3600;
int runMinutes=secsRemaining/60;
int runSeconds=secsRemaining%60;

char buf[21];
sprintf(buf,"Runtime%02d:%02d:%02d",runHours,runMinutes,runSeconds);
Serial.println(buf);


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
