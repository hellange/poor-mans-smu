/****************************************************************
 *  
 *  Initial GUI prototype for
 *  P O O R  M A N ' s  S M U
 *  
 *  by Helge Langehaug (2018, 2019)
 * 
 *****************************************************************/

#include <SPI.h>
#include "GD2.h"
#include "colors.h"
#include "volt_display.h"
#include "current_display.h"
#include "Stats.h"
#include "Calibration.h"
#include "Filters.h"
#include "digit_util.h"
#include "tags.h"

#include "Arduino.h"
#include "Wire.h"

#include "SMU_HAL_dummy.h"
#include "SMU_HAL_717x.h"

#define _SOURCE_AND_SINK 111

#define _PRINT_ERROR_VOLTAGE_SOURCE_SETTING 0
#define _PRINT_ERROR_CURRENT_SOURCE_SETTING 1
#define _PRINT_ERROR_SERIAL_COMMAND 2
#define _PRINT_ERROR_NO_CALIBRATION_DATA_FOUND 3
#define _PRINT_ERROR_FACTORY_CALIBRATION_RESTORE_FAILED_CATASTROPHICALLY


#define GEST_NONE 0
#define GEST_MOVE_LEFT 1
#define GEST_MOVE_RIGHT 2
#define GEST_MOVE_DOWN 3
#define LOWER_WIDGET_Y_POS 250

//!touchscreen I2C address
#define I2C_FT6206_ADDR0    0x38

//uncomment below if you want to use read AD/DA 
ADCClass SMU[1] = {
  ADCClass()
};

// uncomment below if you want to use dummy
//SMU_HAL_dummy SMU[1] = {
//  SMU_HAL_dummy()
//};

int scroll = 0;
int scrollDir = 0;
int scrollMainMenu = 0;
int scrollMainMenuDir = 0;
boolean mainMenuActive = false;


#define BUTTON_NULL 220
#define BUTTON_UNCAL 221
StatsClass V_STATS;
StatsClass C_STATS;
FiltersClass V_FILTERS;
CalibrationClass CALIBRATION;

float rawMa_glob; // TODO: store in stats for analysis just as voltage

float DACVout;  // TODO: Dont use global

int noOfWidgets = 6;
int activeWidget = 0;



#include "dial.h"

void setup()
{
    disableSPIunits();
delay(50);
    pinMode(6,OUTPUT); // LCD powerdown pin?
   digitalWrite(6, HIGH);
   
//  Serial.begin(115200);
//   while(!Serial) {
//  }
     pinMode(11,OUTPUT);
   pinMode(12,INPUT);
   pinMode(13,OUTPUT);

   Serial.println("Initializing graphics controller FT81x...");
   Serial.flush();

   // bootup FT8xx
   // Drive the PD_N pin high
   Serial.flush();




 // brief reset on the LCD clear pin
//   delay(200);
 //     digitalWrite(6, LOW);
//         delay(200);
//    digitalWrite(6, HIGH);
//   delay(200);




   GD.begin(0);
      delay(100);

   Serial.println("...begin...");
   Serial.flush();
   GD.cmd_romfont(1, 34); // put FT81x font 34 in slot 1
   delay(100);
   GD.Clear();
   GD.ColorRGB(0xaaaaff);
  GD.ColorA(120);
  GD.cmd_text(250, 200 ,   31, 0, "Poor man's SMU");
     GD.ColorRGB(0xaaaaaa);
  
    GD.cmd_text(250, 240 ,   28, 0, "Designed    by    Helge Langehaug");

     delay(100);

  GD.swap();
 GD.__end();
   Serial.println("...Done");
   Serial.flush();

   disableSPIunits();
   delay(100);
   Serial.println("Start measuring...");
   SMU[0].init();


  float setMv = 0.0;
  float setMa = 10.0;
  SMU[0].fltSetCommitCurrentSource(setMa / 1000.0, _SOURCE_AND_SINK);
  SMU[0].fltSetCommitVoltageSource(setMv / 1000.0);
  Serial.println("Done!");

  V_STATS.init(DigitUtilClass::typeVoltage);
  C_STATS.init(DigitUtilClass::typeCurrent);
  V_FILTERS.init();
   CALIBRATION.init();


  
}

void disableSPIunits(){
 



// preliminary set the mux address for ADC here...
     pinMode(7, OUTPUT);  // mux master chip select
  digitalWrite(7, HIGH);
 

  
  
  pinMode(10, OUTPUT);   // lcd
  digitalWrite(10, HIGH);
}


void showStatusIndicator(int x,int y,const char* text, bool enable, bool warn) {
  GD.Begin(RECTS);
  GD.ColorRGB(warn ? 0xFF0000 : COLOR_VOLT);
  GD.ColorA(enable ? 150 : 25);
  
  GD.LineWidth(150);
  GD.Vertex2ii(x, 15 + y);
  GD.Vertex2ii(x + 60, y + 24);
  GD.ColorRGB(0xffffff);
  GD.ColorA(enable ? 200 : 25);
  GD.cmd_text(x+ 2, y + 10, 27, 0, text);
  GD.ColorA(255);
}
void voltagePanel(int x, int y) {

  // heading
  GD.ColorRGB(COLOR_VOLT);
  GD.ColorA(120);
  GD.cmd_text(x+20, y + 2 ,   29, 0, "SOURCE VOLTAGE");
  GD.cmd_text(x+20 + 1, y + 2 + 1 ,   29, 0, "SOURCE VOLTAGE");

  //GD.ColorA(200);
  //GD.cmd_text(x+20, y + 2 ,   29, 0, "SOURCE VOLTAGE");
   
  // primary
  VOLT_DISPLAY.renderMeasured(x + 17,y + 26, V_FILTERS.mean);

  // secondary volt
  GD.ColorRGB(COLOR_VOLT);
  GD.ColorA(180); // a bit lighter
  DIGIT_UTIL.renderValue(x + 320,  y-4 , V_STATS.rawValue, 4, DigitUtilClass::typeVoltage); 

  GD.ColorA(255);
  VOLT_DISPLAY.renderSet(x + 120, y + 26 + 105, SMU[0].getSetValuemV());

  GD.ColorRGB(0,0,0);
  GD.cmd_fgcolor(0xaaaa90);  
  
  GD.Tag(BUTTON_VOLT_SET);
  GD.cmd_button(x + 20,y + 132,95,50,29,OPT_NOTEAR,"SET");
  GD.Tag(BUTTON_VOLT_AUTO);
  GD.cmd_button(x + 350,y + 132,95,50,29,0,"AUTO");

  //rawValue
  //renderDeviation(x + 667,y + 125, V_FILTERS.mean /*V_STATS.rawValue*/, SMU[0].getSetValuemV(), false);

  showStatusIndicator(x+630, y+5, "FILTER", V_FILTERS.filterSize>1, false);
  showStatusIndicator(x+720, y+5, "NULL", CALIBRATION.nullValue!=0.0, false);
  showStatusIndicator(x+630, y+45, "50Hz", false, false);
  showStatusIndicator(x+720, y+45, "4 1/2", false, false);
  showStatusIndicator(x+630, y+85, "COMP", SMU[0].compliance(), true);
  showStatusIndicator(x+720, y+85, "UNCAL", !CALIBRATION.useCalibratedValues, true);

}

 
 void showAnalogPin(int x, int y, int radius, int radiusStart, int degreeRelativeToTop, int needleColor, int lineWidth, boolean needle) {
  int maxDegree = 60; 
  
  degreeRelativeToTop = degreeRelativeToTop <-maxDegree ? -maxDegree: degreeRelativeToTop;
  degreeRelativeToTop = degreeRelativeToTop > maxDegree ? maxDegree : degreeRelativeToTop;
  
  float oneDegreeRad = 2*3.1415 / 360.0;
  float rad = (3.1415/2.0) - degreeRelativeToTop * oneDegreeRad;
  
  GD.ColorRGB(needleColor);
  GD.ColorA(255);
  GD.Begin(LINE_STRIP);
  GD.LineWidth(lineWidth);
  GD.Vertex2ii(x+cos(rad)*radius, y-sin(rad)*radius);
  GD.Vertex2ii(x+cos(rad)*radiusStart, y-sin(rad)*radiusStart);

  if (needle){
    GD.Vertex2ii(x+cos(rad)*radius, y-sin(rad)*radius);
    GD.Vertex2ii(x+cos(rad*1.04)*radiusStart, y-sin(rad*1.04)*radiusStart);
    GD.Vertex2ii(x+cos(rad*0.96)*radiusStart, y-sin(rad*0.96)*radiusStart);
    GD.Vertex2ii(x+cos(rad)*radius, y-sin(rad)*radius);

  }

}




int slider_value; // TODO: Not global !
#define tag_filter_slider 123
void handleSliders(int x, int y) {
  
  y=y+50;
  GD.Tag(tag_filter_slider);


  
  GD.ColorRGB(COLOR_VOLT);
  GD.cmd_slider(500+x, y+30, 204,15, OPT_FLAT, slider_value, 65535);
  GD.cmd_track(500+x, y+30, 204, 20, 123);
  
  GD.ColorRGB(0xffffff);
  GD.cmd_text(550+x,y, 27, 0, "Filter size:");
  GD.cmd_number(630+x,y, 27, 0, V_FILTERS.filterSize);

  if (!DIAL.isDialogOpen()) {
    GD.Tag(BUTTON_NULL);
    if (CALIBRATION.nullValue!=0.0) {
      GD.ColorRGB(0x00ff00);
    } else {
      GD.ColorRGB(0x000000);
    }
  } else {
    GD.ColorA(100);
  }

  GD.cmd_button(x+700,y+130,95,50,29,0,"NULL");


  if (!DIAL.isDialogOpen()) {
    GD.Tag(BUTTON_UNCAL);
    if (CALIBRATION.useCalibratedValues == false) {
      GD.ColorRGB(0x00ff00);
    } else {
      GD.ColorRGB(0x000000);
    }
  } else {
    GD.ColorA(100);
  }

  GD.cmd_button(x+600,y+130,95,50,29,0,"UNCAL");

  GD.ColorA(255);

  



  
  GD.get_inputs();
  switch (GD.inputs.track_tag & 0xff) {
    case tag_filter_slider:
      slider_value = GD.inputs.track_val;
      Serial.print("Set filter value:");
      int v = 100.0 * slider_value / 65535.0;
      Serial.println(v);
      V_FILTERS.setFilterSize(int(v));
      break;

  }
}


void renderExperimental(int x, int y, float valM, float setM, bool cur) {

 handleSliders(x,y);


 
  y=y+65;
  x=x+100;
  
  if (cur) {
     //Special handling: set current must currently be positive even if sink/negative.
     //                  This give error when comparing negative measured and positive set.
     //                  Use absolute values to give "correct" comparision...
     setM = abs(setM);
     valM = abs(valM);
  }

  
  float deviationInPercent = 100.0 * ((setM - valM) / setM);
  if (setM == 0.0) {
    deviationInPercent = 100;
  }
  float degrees = -deviationInPercent * 700.0;
  renderAnalogGauge(x,y,240, degrees, deviationInPercent, "Deviation from SET");

 

 
}

void renderAnalogGauge(int x, int y, int size, float degrees, float value, char* title) {
  //experimental feature showing deviation from set value
  
  int gaugeRadius = size/2;

  for (int i=-50; i<=50; i=i+10) {
      int needleColor;
      if (i==30 || i ==-30) {
        needleColor = COLOR_ORANGE;
      }
      else if (i>-40 && i<40) {
        needleColor = 0xffffff;
      } else {
        needleColor = 0xff0000;
      }
      showAnalogPin(x+gaugeRadius, y+gaugeRadius+10, gaugeRadius, gaugeRadius - 10, i, needleColor, gaugeRadius/4, false);
  }

  GD.Begin(LINE_STRIP);
  GD.ColorRGB(0x888888);
  GD.ColorA(255);
  GD.LineWidth(10);

  GD.Vertex2ii(x, y);
  GD.Vertex2ii(x, y+gaugeRadius+10);
  GD.Vertex2ii(x+gaugeRadius*2, y+gaugeRadius+10);
  GD.Vertex2ii(x+gaugeRadius*2, y);
  GD.Vertex2ii(x, y);

  showAnalogPin(x+gaugeRadius, y+gaugeRadius+10, gaugeRadius, 30, degrees, 0xffffff, 20, true);

  
  GD.Begin(RECTS);
  GD.ColorRGB(0x223322);
  GD.ColorA(255);
  GD.Vertex2ii(x+4, y+gaugeRadius-25 + 2);
  GD.Vertex2ii(x+gaugeRadius*2-4, y+gaugeRadius+10-4);
  
  GD.ColorRGB(0xffffff);
  GD.cmd_text(x+gaugeRadius/2, y-20, 27, 0, title);
  
  GD.ColorRGB(0xdddddd);


  y=y+gaugeRadius-22;
  x=x+gaugeRadius*1.2/2;
  float deviationInPercent = abs(value);

  int font = 29;
     if (deviationInPercent < 1.0) {
      GD.cmd_text(x+8, y, font, 0, "0.");
      GD.cmd_number(x+30, y, font, 3, deviationInPercent * 1000.0);
      GD.cmd_text(x+85, y, font, 0, "%");
    } else if (deviationInPercent > 10.0){
      GD.ColorRGB(255,0,0); // RED
      GD.cmd_text(x+25, y, font, 0, ">10%");
    } else if (deviationInPercent >= 1.0 && deviationInPercent <10.0){
      int whole = (int)deviationInPercent;
      GD.cmd_number(x+5, y, font, 1, whole );
      GD.cmd_text(x+20, y, font, 0, ".");
      GD.cmd_number(x+30, y, font, 2, (deviationInPercent - (float)whole) * 100.0);
      GD.cmd_text(x+65, y, font, 0, "%");
    }
 }



void renderVoltageGraph(int x,int y, bool scrolling) {
  V_STATS.renderTrend(x, y, scrolling);
}
void renderCurrentGraph(int x,int y, bool scrolling) {
  C_STATS.renderTrend(x, y, scrolling);
}

void renderHistogram(int x,int y, bool scrolling) {
  V_STATS.renderHistogram(x,y,scrolling);
}


void currentPanel(int x, int y, boolean overflow) {
  if (x >= 800) {
    return;
  }
  
  y=y+48;  

  GD.Begin(RECTS);
    GD.LineWidth(10);
  for (int i=0;i<40;i++) {
    int percent = 100 * (C_STATS.rawValue / SMU[0].getSetValuemA());
    if (percent > i*2.5) {
       GD.ColorA(255);
    } else {
       GD.ColorA(60);
    }
    if (i>34) {
       GD.ColorRGB(0xff0000);
    } else if (i>29) {
       GD.ColorRGB(COLOR_ORANGE);
    }else {
       GD.ColorRGB(0x00cc00);
    }
    GD.Vertex2ii(x+20 + i*14 ,y);
    GD.Vertex2ii(x+20 + (i+1)*14 - 3, y+9);
  }
  
  y=y+12;
  GD.ColorA(255);

  CURRENT_DISPLAY.renderMeasured(x + 17, y, C_STATS.rawValue, overflow);
  CURRENT_DISPLAY.renderSet(x+120, y+105, SMU[0].getSetValuemA());
  

  y=y+105;
  
  GD.ColorRGB(0,0,0);
  GD.cmd_fgcolor(0xaaaa90);  
  GD.Tag(BUTTON_CUR_SET);
  GD.cmd_button(x+20,y,95,50,29,0,"LIM");
  GD.Tag(BUTTON_CUR_AUTO);
  GD.cmd_button(x+350,y,95,50,29,0,"AUTO");
  
}

void drawBall(int x, int y, bool set) {
  GD.Begin(POINTS);
  GD.PointSize(16 * 6);  
  if (set == true) {
    GD.ColorRGB(255,255,255); 
  } else {
    GD.ColorRGB(0,0,0); 
  }
  GD.Vertex2ii(x, y);
}

void widgetBodyHeaderTab(int y, int activeWidget) {
  y=y+10;
  GD.Begin(RECTS);

  // tab light shaddow
  GD.ColorA(255);
  GD.LineWidth(350);
  GD.ColorRGB(0x555555);
  GD.Vertex2ii(21,y-2);
  GD.Vertex2ii(778, 480);
  
  // tab 
  GD.ColorA(255);
  GD.LineWidth(350);
  GD.ColorRGB(0x2a2a2a);
  GD.Vertex2ii(22,y);
  GD.Vertex2ii(778, 480);

  // Why does this blend to dark gray instead of being just black ?
  // Is it because the rectangle above "shines" though ? It is not set to transparrent...
  // must learn more about the blending....
  GD.Begin(RECTS);
  GD.ColorA(230); // slightly transparrent to grey shine though a bit
  GD.LineWidth(10);
  GD.ColorRGB(0x000000);
  GD.Vertex2ii(0,y+15);
  GD.Vertex2ii(799, 480);

  // strip between tab heading and content
  GD.Begin(LINE_STRIP);
  GD.ColorA(200);
  GD.LineWidth(10);
  GD.ColorRGB(COLOR_CURRENT_TEXT);
  GD.Vertex2ii(0,y+15+2);
  GD.Vertex2ii(798, y+15+2);
  GD.ColorA(255);


  y=y-2;
  int x = 400 - 30 * noOfWidgets/2;
  for (int i = 0; i < noOfWidgets; i++) {
    drawBall(x+ i*30,y,activeWidget == i);
  }
}


void showWidget(int y, int widgetNo, int scroll) {
  int yPos = y-6;
  if (widgetNo ==0) {
     if (scroll ==0){
       //GD.ColorRGB(0xbbbbbb); // gray
       GD.ColorRGB(COLOR_CURRENT_TEXT);
       GD.cmd_text(20, yPos, 29, 0, "MEASURE CURRENT");
     }
     currentPanel(scroll, yPos, SMU[0].compliance());
  } else if (widgetNo == 1) {
    if (!DIAL.isDialogOpen()){
       if (scroll ==0){
         GD.ColorRGB(COLOR_CURRENT_TEXT);
         GD.cmd_text(20, yPos, 29, 0, "CURRENT TREND");
       }
       renderCurrentGraph(scroll, yPos, reduceDetails());
    }
  } else if (widgetNo == 2) {
      if (!DIAL.isDialogOpen()){
        if (scroll ==0){
          GD.ColorRGB(COLOR_VOLT);
          GD.cmd_text(20, yPos, 29, 0, "VOLTAGE TREND");
        }
        renderVoltageGraph(scroll, yPos, reduceDetails());
      }
  } else if (widgetNo == 3) {
      if (scroll ==0){
        GD.ColorRGB(COLOR_VOLT);
        GD.cmd_text(20, yPos, 29, 0, "VOLTAGE HISTOGRAM");
      }
      renderHistogram(scroll, yPos, reduceDetails());
  } else if (widgetNo == 4) {
      if (scroll ==0){
        GD.ColorRGB(COLOR_VOLT);
        GD.cmd_text(20, yPos, 29, 0, "EXPERIMENTAL");
      }
      float rawM = V_FILTERS.mean;
      float setM = SMU[0].getSetValuemV();
      renderExperimental(scroll,yPos, rawM, setM, false);
  } else if (widgetNo == 5) {
      if (scroll ==0){
        GD.ColorRGB(COLOR_VOLT);
        GD.cmd_text(20, yPos, 29, 0, "CAL");
      }
      float rawM = V_FILTERS.mean;
      float setM = SMU[0].getSetValuemV();
      CALIBRATION.renderCal(scroll,yPos, rawM, setM, false);
  }

}

bool reduceDetails() {
  return scrollDir != 0 || DIAL.isDialogOpen() || mainMenuActive == true;
}


int gestureDetected = GEST_NONE;
int scrollSpeed = 75;
void handleWidgetScrollPosition() {
  if (gestureDetected == GEST_MOVE_LEFT) {
    if (activeWidget == noOfWidgets -1) {
      Serial.println("reached right end");
    } else {
      scrollDir = -1;
    }
  } 
  else if (gestureDetected == GEST_MOVE_RIGHT) {
    if (activeWidget == 0) {
      Serial.println("reached left end");
    } else {
      scrollDir = 1;
    }
  } 
  
  scroll = scroll + scrollDir * scrollSpeed;
  if (scroll <= -800 && scrollDir != 0) {
    activeWidget ++;
    scrollDir = 0;
    scroll = 0;
  } else if (scroll >= 800 && scrollDir != 0) {
    activeWidget --;
    scrollDir = 0;
    scroll = 0;
  }
}

void bluredBackground() {
    GD.Begin(RECTS);
    GD.ColorA(150);
    GD.ColorRGB(0x000000);
    GD.Vertex2ii(0,0);
    GD.Vertex2ii(800, 480);
}
void handleMenuScrolldown(){

  if (gestureDetected == GEST_MOVE_DOWN && mainMenuActive == false) {
    mainMenuActive = true;
    scrollMainMenuDir = 1;
    return; // start the animation etc. next time, so UI that needs to reduce details have time to reach.
  }  

  // main menu
  if (mainMenuActive) {

    bluredBackground();

  
    scrollMainMenu = scrollMainMenu + scrollMainMenuDir*25;
    if (scrollMainMenu > 350) {
      scrollMainMenu = 350;
      scrollMainMenuDir = 0;
    }

    GD.Begin(RECTS);
    GD.LineWidth(200);
    GD.ColorA(200);
    GD.ColorRGB(0x888888);
    GD.Vertex2ii(50,0);
    GD.Vertex2ii(750, scrollMainMenu+40);

    GD.Begin(RECTS);
    GD.LineWidth(180);
    GD.ColorA(230);
    GD.ColorRGB(0x000000);
    GD.Vertex2ii(750, scrollMainMenu+40);
    GD.ColorRGB(0xffffff);

    GD.ColorA(230);

    int buttonWidth = 200;
    int buttonHeight = 70;
               
    const char * t[3][3] = {
    {"SOURCE VOLT\0", "SOURCE CURRENT\0", "BATTERY\0"},
    {"ELECTRONIC LOAD\0", "VOLTMETER\0", "PULSE GENERATOR\0"},
    {"SWEEP\0", "RESISTANCE\0", "SETTINGS\0"}};
    GD.ColorRGB(0x444444);
    for (int y =0;y<3;y++) {
      for (int x =0;x<3;x++) {
        GD.cmd_button(70+(buttonWidth+30)*x,scrollMainMenu-280+(buttonHeight+20)*y,buttonWidth,buttonHeight,28,0,t[y][x]);
      }
    }
    GD.Tag(MAIN_MENU_CLOSE);
    GD.cmd_button(360,scrollMainMenu-10,80,40,28,0,"CLOSE");
   



    if(GD.inputs.tag == MAIN_MENU_CLOSE && scrollMainMenuDir == 0) {
      scrollMainMenuDir = -1;
    }
  } 
  if (scrollMainMenuDir == -1){
      scrollMainMenu = scrollMainMenu + scrollMainMenuDir*15;
      if (scrollMainMenu < 0) {
        scrollMainMenu = 0;
        mainMenuActive = false;
        scrollMainMenuDir = 0;
      }
  }
  
}


void renderDisplay() {

  int x = 0;
  int y = 2;
 

  // register screen for gestures on top half
  GD.Tag(GESTURE_AREA_HIGH);
  GD.Begin(RECTS);
  GD.ColorA(200);
  GD.ColorRGB(0x000000);
  GD.Vertex2ii(0,0);
  GD.Vertex2ii(800, LOWER_WIDGET_Y_POS);

  // top header
  GD.Begin(RECTS);
  GD.ColorA(255);
  GD.ColorRGB(0x181818);
  GD.Vertex2ii(0,0);
  GD.Vertex2ii(800, 22);
  
  
  GD.ColorA(255);
  GD.ColorRGB(0xdddddd);
  GD.cmd_text(x + 30, 2, 27, 0, "Input 25.4V / - 25.3V");

  // line below top header
  y=y+23;
  GD.Begin(LINE_STRIP);
  GD.ColorA(200);
  GD.LineWidth(15);
  GD.ColorRGB(COLOR_VOLT);
  GD.Vertex2ii(1,y);
  GD.Vertex2ii(799, y);
  GD.ColorA(255);

  y=y+7;
  // show upper panel
  voltagePanel(x,y);

  // register screen for gestures on lower half
  GD.Tag(GESTURE_AREA_LOW);
  GD.Begin(RECTS);
  //GD.ColorA(200);
  GD.ColorRGB(0x000000);
  GD.Vertex2ii(0,LOWER_WIDGET_Y_POS);
  GD.Vertex2ii(800, 480);
  
  handleWidgetScrollPosition();

  widgetBodyHeaderTab(LOWER_WIDGET_Y_POS, activeWidget);

  if (activeWidget >= 0) {
    if (scrollDir == 0) {
      showWidget(LOWER_WIDGET_Y_POS, activeWidget, 0);
    }
    else if (scrollDir == -1) {
      showWidget(LOWER_WIDGET_Y_POS,activeWidget, scroll); 
      showWidget(LOWER_WIDGET_Y_POS,activeWidget + 1, scroll + 800);
    } 
    else if (scrollDir == 1) {
      showWidget(LOWER_WIDGET_Y_POS,activeWidget - 1, scroll - 800);
      showWidget(LOWER_WIDGET_Y_POS,activeWidget, scroll + 0);
    }   
  }
  
  handleMenuScrolldown();

}


int gestOldX = 0;
int gestOldY = 0;
int gestDurationX = 0;
int gestDurationY = 0;

void detectGestures() {
  GD.get_inputs();
  //Serial.println(GD.inputs.tag);
  int touchX = GD.inputs.x;
  int touchY = GD.inputs.y;
  int gestDistanceX = touchX - gestOldX;
  int gestDistanceY = touchY - gestOldY;

  if ((GD.inputs.tag == GESTURE_AREA_LOW || GD.inputs.tag == GESTURE_AREA_HIGH) && gestureDetected == GEST_NONE) {
    if (touchX > 0 && touchY > LOWER_WIDGET_Y_POS && gestDistanceX < -20 && scrollDir == 0) {
      if (++gestDurationX >= 2) {
        Serial.println("move left");
        Serial.flush();
        gestureDetected = GEST_MOVE_LEFT;
        gestDurationX = 0;
      }
    }
    else if (touchX > 0 && touchY > LOWER_WIDGET_Y_POS && gestDistanceX > 20 && scrollDir == 0) {
      if (++gestDurationX >= 2) {
        Serial.println("move right");
        Serial.flush();
        gestureDetected = GEST_MOVE_RIGHT;
        gestDurationX = 0;
      }
    } 
    else if (touchY > 0 && touchY<150 && gestDistanceY > 10 && scrollDir == 0) {
       if (++gestDurationY >= 2) {
        Serial.println("move down from upper");
        Serial.flush();
        gestureDetected = GEST_MOVE_DOWN;
        gestDurationY = 0;
      }

    } 
    
  } else {
    gestureDetected = GEST_NONE;
    gestDurationX = 0;
    gestDurationY = 0;
  }
  gestOldY = GD.inputs.y;  
  gestOldX = GD.inputs.x;  
}




unsigned long startupMillis =  millis();

bool readyToDoStableMeasurements() {
  // wait a second after startup before starting to store measurements
  if (millis() > startupMillis + 3000) {
    return true;
  } else {
    return false;
  }
}

int aliveCounter = 0;
void loop3(){
  GD.resume();
    GD.Clear();
    GD.ColorRGB(COLOR_VOLT);
  GD.ColorA(120);
  GD.cmd_text(20, 20 ,   29, 0, "Hi !");
  GD.swap();

  
    Serial.print("Alive !"); 
        Serial.println(aliveCounter++);  
 
Serial.flush();
delay(1000);
}


void loop2()
{

  // ADC2.init();
//delay(5000);

   long ret = SMU[0].dataReady();
   if(ret < 0) {
   } else {
     float v = SMU[0].measureMilliVoltage();

     Serial.print("raw ");
     Serial.print(AD7176_regs[4].value, HEX); 
     Serial.print("=");
     Serial.print(v);
     
     float offset = -0.0001;
     v=v+offset;
     Serial.print(" adjusted offset ");
     Serial.print(v);
  
     float gain_factor = 0.0484; // arduino 5V
     gain_factor = 0.0372; // teensy 3.3v
     v = v + v * gain_factor;
     
     Serial.print(" adjusted gain ");
     Serial.print(v);
     Serial.println(" mv");
     //delay(1000);
   }
}

float Vout = 0.0;
float VoutLast = 0.0;
void loop()
{
  GD.__end();
  disableSPIunits();
  //delay(1);
  // have problems with dataReady check.
  // preliminary say that new sample is when sample is different from last.
  // Should be enough noise in last signifigant bit so that shoule work...
  //Vout = SMU[0].measureMilliVoltage();
 //if (Vout != VoutLast) {

  int dataR = SMU[0].dataReady();
  if (dataR == -99) {
    Serial.println("DONT USE SAMPLE!");  
  }
  else if (dataR == 1) {
    float Cout = SMU[0].measureCurrent();
    C_STATS.addSample(Cout);
//  Serial.print("Measured raw:");  
//  Serial.print(Cout, 3);
//  Serial.println(" mA");  
//  Serial.flush();
  }
  else if(dataR == 0) {
    
 
 Vout = SMU[0].measureMilliVoltage();

  VoutLast = Vout;
//  Serial.print("Measured raw:");  
//  Serial.print(Vout, 3);
//  Serial.println(" mV");  
//  Serial.flush();


  //if (readyToDoStableMeasurements()) {
    // Dont sample voltage and current while scrolling because polling is slow.
    // TODO: Remove this limitation when sampling is based on interrupts.
    //if (scrollDir == 0) {
       //V_STATS.addSample(SMU[0].measureMilliVoltage() * 1000.0);

       Vout = CALIBRATION.adjust(Vout);
   
       V_STATS.addSample(Vout);
       V_FILTERS.updateMean(Vout);

    //}
   // }
    
  }
  disableSPIunits();
  //delay(1);
  GD.resume();



  if (!gestureDetected) {
    if (GD.inputs.tag == BUTTON_VOLT_SET) {
            Serial.println("Vol set");

      DIAL.open(BUTTON_VOLT_SET, closeCallback, SMU[0].getSetValuemV());
    } else if (GD.inputs.tag == BUTTON_CUR_SET) {
      Serial.println("Cur set");
      DIAL.open(BUTTON_CUR_SET, closeCallback, SMU[0].getSetValuemA());
    } else if (GD.inputs.tag == BUTTON_NULL) {
      Serial.println("Null set");
      CALIBRATION.toggleNullValue(Vout);
    } else if (GD.inputs.tag == BUTTON_UNCAL) {
      Serial.println("Uncal set");
      CALIBRATION.toggleCalibratedValues();
    }
  }

  //GD.get_inputs();

  
  detectGestures();

  GD.Clear();
  renderDisplay();

 
  
  if (DIAL.isDialogOpen()) {
    bluredBackground();
    DIAL.checkKeypress();
    DIAL.handleKeypadDialog();
  }

  GD.swap(); 
   
  GD.__end();

}



void closeCallback(int vol_cur_type, bool cancel) {
   Serial.print("SET type:");
   Serial.println(vol_cur_type);
   Serial.println(DIAL.type() );
  if (cancel) {
    return;
  }
    GD.__end();
  disableSPIunits();
  if (DIAL.type() == BUTTON_VOLT_SET) {
     if (SMU[0].fltSetCommitVoltageSource(DIAL.getMv() / 1000.0)) printError(_PRINT_ERROR_VOLTAGE_SOURCE_SETTING);
  }
  if (DIAL.type() == BUTTON_CUR_SET) {
     if (SMU[0].fltSetCommitCurrentSource(DIAL.getMv() / 1000.0, _SOURCE_AND_SINK)) printError(_PRINT_ERROR_CURRENT_SOURCE_SETTING);
  }
  GD.resume();
}




void printError(int16_t  errorNum)
{
  Serial.print(F("Error("));
  Serial.print(errorNum);
  Serial.println(")");
}
