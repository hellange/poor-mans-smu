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
#include "Mainmenu.h"
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

int timeAtStartup;
bool startupCalibrationDone1 = false;
bool startupCalibrationDone2 = false;

#define BUTTON_NULL 220
#define BUTTON_UNCAL 221



float rawMa_glob; // TODO: store in stats for analysis just as voltage

float DACVout;  // TODO: Dont use global

int noOfWidgets = 6;
int activeWidget = 0;

#include "dial.h"

int current_range = 0; // TODO: get rid of global
int timeSinceLastChange = 0;  // TODO: get rid of global

void setup()
{
   disable_ADC_DAC_SPI_units();
   delay(50);
   pinMode(6,OUTPUT); // LCD powerdown pin?
   digitalWrite(6, HIGH);
   
//  Serial.begin(115200);
//   while(!Serial) {
//  }

    pinMode(7,OUTPUT);
    pinMode(8,OUTPUT);
    pinMode(9,OUTPUT);
    pinMode(10,OUTPUT);

    pinMode(4,OUTPUT); // current range io pin
    digitalWrite(4, HIGH);

    
    //pinMode(11,OUTPUT);
    //pinMode(12,INPUT);
    //pinMode(13,OUTPUT);

    Serial.println("Initializing graphics controller FT81x...");
    Serial.flush();

    // bootup FT8xx
    // Drive the PD_N pin high
    Serial.flush();

    //brief reset on the LCD clear pin
    delay(200);
    digitalWrite(6, LOW);
    delay(200);
    digitalWrite(6, HIGH);
    delay(200);


   GD.begin(0);
   delay(100);

   Serial.println("...begin...");
   Serial.flush();
   GD.cmd_romfont(1, 34); // put FT81x font 34 in slot 1
   GD.Clear();
   GD.ColorRGB(0xaaaaff);
   GD.ColorA(200);
   GD.cmd_text(250, 200 ,   31, 0, "Poor man's SMU");
   GD.ColorRGB(0xaaaaaa);
   GD.cmd_text(250, 240 ,   28, 0, "Designed    by    Helge Langehaug");

   GD.swap();
   delay(500);

   GD.__end();
   Serial.println("...Done");
   Serial.flush();

   disable_ADC_DAC_SPI_units();
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
   C_FILTERS.init();
   
   V_CALIBRATION.init();
   C_CALIBRATION.init();

   V_DIAL.init();
   C_DIAL.init();

   timeAtStartup = millis();
}

void disable_ADC_DAC_SPI_units(){
   pinMode(7,OUTPUT); // mux master chip select
   digitalWrite(7, HIGH);
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

void sourceVoltagePanel(int x, int y) {

  // heading
  GD.ColorRGB(COLOR_VOLT);
  GD.ColorA(120);
  GD.cmd_text(x+20, y + 2 ,   29, 0, "SOURCE VOLTAGE");
  GD.cmd_text(x+20 + 1, y + 2 + 1 ,   29, 0, "SOURCE VOLTAGE");
  
  // primary
  VOLT_DISPLAY.renderMeasured(x + 17,y + 26, V_FILTERS.mean);

  // secondary volt
  GD.ColorRGB(COLOR_VOLT);
  GD.ColorA(180); // a bit lighter
  DIGIT_UTIL.renderValue(x + 320,  y-4 , V_STATS.rawValue, 4, DigitUtilClass::typeVoltage); 

  GD.ColorA(255);
  VOLT_DISPLAY.renderSet(x + 120, y + 131, SMU[0].getSetValuemV());

  GD.ColorRGB(0,0,0);
  GD.cmd_fgcolor(0xaaaa90);  
  
  GD.Tag(BUTTON_VOLT_SET);
  GD.cmd_button(x + 20,y + 132,95,50,29,OPT_NOTEAR,"SET");
  GD.Tag(BUTTON_VOLT_AUTO);
  GD.cmd_button(x + 350,y + 132,95,50,29,0,"AUTO");
}

void renderStatusIndicators(int x, int y) {
  showStatusIndicator(x+630, y+5, "FILTER", V_FILTERS.filterSize>1, false);
  showStatusIndicator(x+720, y+5, "NULL", V_CALIBRATION.nullValueIsSet(current_range), false);
  showStatusIndicator(x+630, y+45, "50Hz", false, false);
  showStatusIndicator(x+720, y+45, "4 1/2", false, false);
  showStatusIndicator(x+630, y+85, "COMP", SMU[0].compliance, true);
  showStatusIndicator(x+720, y+85, "UNCAL", !V_CALIBRATION.useCalibratedValues, true);
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





#define TAG_FILTER_SLIDER 123
#define TAG_FILTER_SLIDER_B 122

bool anyDialogOpen() {
  return V_DIAL.isDialogOpen() or C_DIAL.isDialogOpen();
}

void handleSliders(int x, int y) {
  
  y=y+40;

  GD.Tag(TAG_FILTER_SLIDER);
  GD.cmd_slider(400+x, y+30, 300,15, OPT_FLAT, V_FILTERS.filterSize * (65535/100), 65535);
  GD.cmd_track(400+x, y+30, 300, 20, TAG_FILTER_SLIDER);
  
  GD.Tag(TAG_FILTER_SLIDER_B);
  GD.cmd_slider(400+x, y+90, 300,15, OPT_FLAT, V_STATS.getNrOfSamplesBeforeStore()* (65535/100), 65535);
  GD.cmd_track(400+x, y+90, 300, 20, TAG_FILTER_SLIDER_B);
  
  GD.cmd_text(500+x,y, 27, 0, "Filter size:");
  GD.cmd_number(580+x,y, 27, 0, V_FILTERS.filterSize);
  GD.cmd_text(500+x,y+60, 27, 0, "Samples size:");
  GD.cmd_number(605+x,y+60, 27, 0, V_STATS.getNrOfSamplesBeforeStore());
  
  if (!anyDialogOpen()) {
    if (V_CALIBRATION.nullValueIsSet(current_range)) {
      GD.ColorRGB(0x00ff00);
    } else {
      GD.ColorRGB(0x000000);
    }
  } else {
    //GD.ColorA(100);
  }
    GD.Tag(BUTTON_NULL);
  GD.cmd_button(x+700,y+130,95,50,29,0,"NULL");


  if (!anyDialogOpen()) {
    if (V_CALIBRATION.useCalibratedValues == false) {
      GD.ColorRGB(0x00ff00);
    } else {
      GD.ColorRGB(0x000000);
    }
  } else {
    //GD.ColorA(100);
  }
  GD.Tag(BUTTON_UNCAL);
  GD.cmd_button(x+600,y+130,95,50,29,0,"UNCAL");

  GD.ColorA(255);
  GD.Tag(0); // Note: Prevents button in some cases to react also when touching other places in UI. Why ?


  GD.get_inputs();
  switch (GD.inputs.track_tag & 0xff) {
    case TAG_FILTER_SLIDER: {
      Serial.print("Set filter value:");
      int slider_val = 100.0 * GD.inputs.track_val / 65535.0;
      Serial.println(slider_val);
      V_FILTERS.setFilterSize(int(slider_val));
      // currently set same as for voltage
      C_FILTERS.setFilterSize(int(slider_val));

      break;
    }
    case TAG_FILTER_SLIDER_B:{
      Serial.print("Set samples value:");
      int slider_val = 100.0 * GD.inputs.track_val / 65535.0;
      Serial.println(slider_val);
      V_STATS.setNrOfSamplesBeforeStore(int(slider_val));
      // for now, just use same is current as for voltage
      C_STATS.setNrOfSamplesBeforeStore(int(slider_val));
      break;
    }
    default:
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

void renderBar(int x, int y, float rawValue, float setValue) {
  GD.Begin(RECTS);
  GD.LineWidth(10);
  for (int i=0;i<40;i++) {
    int percent = 100 * (abs(rawValue) / abs(setValue));
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
}

void currentPanel(int x, int y, boolean compliance, bool showBar) {
  if (x >= 800) {
    return;
  }
  y=y+28;
  if (showBar) {
    renderBar(x,y, C_STATS.rawValue, SMU[0].getSetValuemA());
    y=y+12;
  }
  
  GD.ColorA(255);

  CURRENT_DISPLAY.renderMeasured(x + 17, y, C_FILTERS.mean, compliance);
  CURRENT_DISPLAY.renderSet(x+120, y+105, SMU[0].getSetValuemA());

  y=y+105;
  
  GD.ColorRGB(0,0,0);
  GD.cmd_fgcolor(0xaaaa90);  
  GD.Tag(BUTTON_CUR_SET);
  GD.cmd_button(x+20,y,95,50,29,0,"LIM");
  GD.Tag(BUTTON_CUR_AUTO);
  GD.cmd_button(x+350,y,95,50,29,0,current_range==0 ? "1A" : "10mA");
  GD.Tag(0); // Note: Prevents button in some cases to react also when touching other places in UI. Why ?
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
  GD.ColorRGB(0x2c2c2c);
  GD.Vertex2ii(22,y);
  GD.Vertex2ii(778, 480);

  // Why does this blend to dark gray instead of being just black ?
  // Is it because the rectangle above "shines" though ? It is not set to transparrent...
  // must learn more about the blending....
  GD.Begin(RECTS);
  GD.ColorA(180); // slightly transparrent to grey shine though a bit
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
     currentPanel(scroll, yPos + 20, SMU[0].compliance, true);
  } else if (widgetNo == 1) {
    if (!anyDialogOpen()){
       if (scroll ==0){
         GD.ColorRGB(COLOR_CURRENT_TEXT);
         GD.cmd_text(20, yPos, 29, 0, "CURRENT TREND");
       }
       renderCurrentGraph(scroll, yPos, reduceDetails());
    }
  } else if (widgetNo == 2) {
      if (!anyDialogOpen()){
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

        float rawM = V_FILTERS.mean;
        float setM = SMU[0].getSetValuemV();
        V_CALIBRATION.renderCal(scroll,yPos, rawM, setM, false);
      }
      
  }

}

bool reduceDetails() {
  return scrollDir != 0 || MAINMENU.active == true;
}


int gestureDetected = GEST_NONE;
int scrollSpeed = 75;
void handleWidgetScrollPosition() {
  if (gestureDetected == GEST_MOVE_LEFT) {
//    if (activeWidget == noOfWidgets -1) {
//      Serial.println("reached right end");
//    } else {
//      scrollDir = -1;
//    }
    scrollDir = -1;
  } 
  else if (gestureDetected == GEST_MOVE_RIGHT) {
//    if (activeWidget == 0) {
//      Serial.println("reached left end");
//    } else {
//      scrollDir = 1;
//    }
    scrollDir = 1;
  } 
  
  scroll = scroll + scrollDir * scrollSpeed;
  if (scroll <= -800 && scrollDir != 0) {
    activeWidget ++;
    if (activeWidget > noOfWidgets -1) {
      // swap arund
      activeWidget = 0;
    }
    scrollDir = 0;
    scroll = 0;
  } else if (scroll >= 800 && scrollDir != 0) {
    activeWidget --;
    if (activeWidget < 0) {
      // swap around
      activeWidget = noOfWidgets - 1;
    }
    scrollDir = 0;
    scroll = 0;
  }
}

void bluredBackground() {
    GD.Begin(RECTS);
    //GD.ColorA(150);
    GD.ColorRGB(0x000000);
    GD.Vertex2ii(0,0);
    GD.Vertex2ii(800, 480);
}

void handleMenuScrolldown(){

  if (gestureDetected == GEST_MOVE_DOWN && MAINMENU.active == false) {
    MAINMENU.activate();
    return; // start the animation etc. next time, so UI that needs to reduce details have time to reach.
  }  
  
  // main menu
  if (MAINMENU.active) {
    MAINMENU.handle();
  }
 
}


void renderDisplay() {

  int x = 0;
  int y = 2;
 
  // register screen for gestures on top half
  GD.Tag(GESTURE_AREA_HIGH);
  GD.Begin(RECTS);
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
  GD.cmd_text(x + 30, 2, 27, 0, "Input 25.4V / - 25.3V"); // NOTE: Currently only dummy info

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
  sourceVoltagePanel(x,y);
  renderStatusIndicators(x,y);
 
  // register screen for gestures on lower half
  GD.Tag(GESTURE_AREA_LOW);
  GD.Begin(RECTS);
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
      if (activeWidget == noOfWidgets - 1) {
        // swap from last to first
        showWidget(LOWER_WIDGET_Y_POS, 0, scroll + 800);
      } else {
        showWidget(LOWER_WIDGET_Y_POS,activeWidget + 1, scroll + 800);
      }
    } 
    else if (scrollDir == 1) {
      if (activeWidget == 0) { 
        // swap from first to last
        showWidget(LOWER_WIDGET_Y_POS,noOfWidgets -1 , scroll - 800);
      } else {
        showWidget(LOWER_WIDGET_Y_POS,activeWidget - 1, scroll - 800);
      }
      showWidget(LOWER_WIDGET_Y_POS,activeWidget, scroll + 0);
    }   
  }
  
  handleMenuScrolldown();

  if(V_STATS.rawValue > 10.0) {
    GD.ColorRGB(0xdddddd);
    GD.cmd_number(600, 0, 27, 6, (int)(V_STATS.rawValue / C_STATS.rawValue));
    GD.cmd_text(670, 0,  27, 0, "ohm load");
  }
}

int gestOldX = 0;
int gestOldY = 0;
int gestDurationX = 0;
int gestDurationY = 0;

int detectGestures() {
  GD.get_inputs();
  //Serial.println(GD.inputs.tag);
  int touchX = GD.inputs.x;
  int touchY = GD.inputs.y;
  int gestDistanceX = touchX - gestOldX;
  int gestDistanceY = touchY - gestOldY;

  if ((GD.inputs.tag == GESTURE_AREA_LOW || GD.inputs.tag == GESTURE_AREA_HIGH) && gestureDetected == GEST_NONE) {
    if (touchX > 0 && touchY > LOWER_WIDGET_Y_POS && gestDistanceX < -20 && scrollDir == 0) {
      if (++gestDurationX >= 2) {
        Serial.println("gesture = move left");
        Serial.flush();
        gestureDetected = GEST_MOVE_LEFT;
        gestDurationX = 0;
      }
    }
    else if (touchX > 0 && touchY > LOWER_WIDGET_Y_POS && gestDistanceX > 20 && scrollDir == 0) {
      if (++gestDurationX >= 2) {
        Serial.println("gesture = move right");
        Serial.flush();
        gestureDetected = GEST_MOVE_RIGHT;
        gestDurationX = 0;
      }
    } 
    else if (touchY > 0 && touchY<150 && gestDistanceY > 10 && scrollDir == 0) {
       if (++gestDurationY >= 2) {
        Serial.println("gesture = move down from upper");
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
  return gestureDetected;
}

int timeBeforeAutoNull = millis() + 2000;
void handleAutoNullAtStartup() {
  if (!startupCalibrationDone1 && timeAtStartup + timeBeforeAutoNull < millis()) {
    current_range = 1;
  }
  
  if (!startupCalibrationDone1 && timeAtStartup + timeBeforeAutoNull + 100 < millis()) {
    float v = V_STATS.rawValue;    
    V_CALIBRATION.toggleNullValue(v, current_range);
    Serial.print("Removed voltage offset from 10mA range:");  
    Serial.println(v);  
    v = C_STATS.rawValue;
    C_CALIBRATION.toggleNullValue(v , current_range);
    Serial.print("Removed current offset from 10mA range:");  
    Serial.println(v);
    startupCalibrationDone1 = true;
  } 

  if (!startupCalibrationDone2 && timeAtStartup + timeBeforeAutoNull + 1000 < millis()) {
    current_range = 0;
  }
  if (!startupCalibrationDone2 && timeAtStartup + timeBeforeAutoNull + 1100 < millis()) {
    float v = V_STATS.rawValue;
    V_CALIBRATION.toggleNullValue(v, current_range);
    Serial.print("Removed voltage offset from 1A range:");  
    Serial.println(v);  
    v = C_STATS.rawValue;
    C_CALIBRATION.toggleNullValue(v,current_range);
    Serial.print("Removed current offset from 1A current range:");  
    Serial.println(v);  
    startupCalibrationDone2 = true;
  } 
}
void loop()

{
  handleAutoNullAtStartup();
    float milliAmpere = C_STATS.rawValue;
    if (startupCalibrationDone1 && startupCalibrationDone2) {
      Serial.print(milliAmpere,5);
      Serial.print("mA, current range:");
      Serial.println(current_range);

//      // auto current range switch. TODO: Move to hardware ? Note that range switch also requires change in limit
//      float hysteresis = 0.5;
//      float switchAt = 3.0;
//      
//        if (current_range == 0 && abs(milliAmpere) < switchAt - hysteresis) {
//          current_range = 1;
//          SMU[0].setCurrentRange(current_range);
//          Serial.println("switching to range 1");
//  
//        }
//        // TODO: Make separate function to calculate current based on shunt and voltage!
//        if (current_range == 1 && abs(milliAmpere) > switchAt) {
//          current_range = 0;
//          SMU[0].setCurrentRange(current_range);
//          Serial.println("switching to range 0");
//        }
    }
  
  GD.__end();

  int dataR = SMU[0].dataReady();
  if (dataR == -99) {
    Serial.println("DONT USE SAMPLE!");  
  }
  else if (dataR == 1) {
    
    float Cout = SMU[0].measureCurrent(current_range);

    Cout = Cout - C_CALIBRATION.nullValue[current_range];

    C_STATS.addSample(Cout);
    C_FILTERS.updateMean(Cout);

//  Serial.print("Measured raw:");  
//  Serial.print(CouVoutt, 3);
//  Serial.println(" mA");  
//  Serial.flush();
  }
  else if(dataR == 0) {
    float Vout = SMU[0].measureMilliVoltage();

//  Serial.print("Measured raw:");  
//  Serial.print(Vout, 3);
//  Serial.println(" mV");  
//  Serial.flush();

    Vout = Vout - V_CALIBRATION.nullValue[current_range];

    V_STATS.addSample(Vout);
    V_FILTERS.updateMean(Vout);
  }
  disable_ADC_DAC_SPI_units();
  GD.resume();

  if (!gestureDetected) {
    int tag = GD.inputs.tag;

    if (tag == BUTTON_VOLT_SET) {
      Serial.println("Volt set");
      V_DIAL.open(BUTTON_VOLT_SET, closeCallback, SMU[0].getSetValuemV());
    } else if (tag == BUTTON_CUR_SET) {
      Serial.println("Cur set");
      C_DIAL.open(BUTTON_CUR_SET, closeCallback, SMU[0].getSetValuemA());
    } else if (tag == BUTTON_NULL) {
      Serial.println("Null set");
      V_CALIBRATION.toggleNullValue(V_STATS.rawValue, current_range);
      C_CALIBRATION.toggleNullValue(C_STATS.rawValue, current_range);
    } else if (tag == BUTTON_UNCAL) {
      Serial.println("Uncal set");
      V_CALIBRATION.toggleCalibratedValues();
    } else if (tag == BUTTON_CUR_AUTO) {
      if (timeSinceLastChange + 500 < millis()){
      
        Serial.println("cur_auto set");
       
        // swap current range
        if (current_range == 0) {
          current_range = 1;
        } else {
          current_range = 0;
        }
        timeSinceLastChange = millis();
        SMU[0].setCurrentRange(current_range);

      }

    }
  }

  detectGestures();

  GD.Clear();
  renderDisplay();
  if (!startupCalibrationDone1 && !startupCalibrationDone2) {
    
    GD.Begin(RECTS);
    GD.ColorA(230);  // already opaque, why ?
    GD.ColorRGB(0x222222);
    GD.Vertex2ii(180, 160);
    GD.Vertex2ii(620, 280);
    GD.ColorRGB(0xffffff);
    GD.cmd_text(250, 200 , 29, 0, "Wait for null adjustment...");
  }

  if (anyDialogOpen()) {
    bluredBackground();
    if (V_DIAL.isDialogOpen()){
      V_DIAL.checkKeypress();
      V_DIAL.handleKeypadDialog();
    } else if (C_DIAL.isDialogOpen()) {
      C_DIAL.checkKeypress();
      C_DIAL.handleKeypadDialog();
    }

  }

  GD.swap(); 
   
  GD.__end();

}


float set_dac[]  = {-2000.00, -1000.00, 0.00, 100.00, 500.00, 1000.00, 1500.00, 2000.00, 3000.00, 4000.00, 4500.00, 5000.00, 6000.00, 7000.00, 8000.00, 9000.00, 10000.00};
  
  // actual output
float meas_dac[] = {-1999.86, -0990.80, 0.00, 100.00, 500.013, 1000.046, 1500.07, 2000.10, 2999.99, 4000.02, 4500.10, 5000.10, 6000.11, 7000.21, 7999.31, 9000.00, 10000.00};

float nonlinear_comp(float milliVolt) {
  // Nonlinearity
  Serial.print("Looking up in comp table for ");
  Serial.print(milliVolt);
  Serial.println(" millivolt");
  float v = milliVolt;
  for (int i=0;i<15;i++) {
    if (v > meas_dac[i] && v <= meas_dac[i+1]) {
      float adj_factor_low = set_dac[i] - meas_dac[i];
      float adj_factor_high = set_dac[i+1] - meas_dac[i+1];
      float adj_factor_diff = adj_factor_high - adj_factor_low;

      float range = set_dac[i+1] - set_dac[i];
      float partWithinRange = ( (v-set_dac[i]) / range); /* 0 to 1. Where then 0.5 is in the middle of the range */
      float adj_factor = adj_factor_low + adj_factor_diff * partWithinRange;

      Serial.print("meas:");  
      Serial.print(v, 4);
      Serial.print(", range:");  
      Serial.print(range, 4);
      Serial.print(", part:");  
      Serial.print(partWithinRange, 4);
      Serial.print(", diff:");  
      Serial.print(adj_factor_diff, 4);
      Serial.print(", factor:");  
      Serial.println(adj_factor, 4);

      Serial.flush();
      v = v + adj_factor; 
      
      return v;
    }
  } 
  return milliVolt;  
}


void closeCallback(int vol_cur_type, bool cancel) {
   Serial.print("SET type:");
   Serial.println(vol_cur_type);
  if (cancel) {
    return;
  }
  GD.__end();
  disable_ADC_DAC_SPI_units();
  if (vol_cur_type == BUTTON_VOLT_SET) {

    float mv = V_DIAL.getMv(); 
     if (V_CALIBRATION.useCalibratedValues == true) {
        mv = nonlinear_comp(mv);
     }
     if (SMU[0].fltSetCommitVoltageSource(mv / 1000.0)) printError(_PRINT_ERROR_VOLTAGE_SOURCE_SETTING);
  }
  if (vol_cur_type == BUTTON_CUR_SET) {
     if (SMU[0].fltSetCommitCurrentSource(C_DIAL.getMv() / 1000.0, _SOURCE_AND_SINK)) printError(_PRINT_ERROR_CURRENT_SOURCE_SETTING);
  }
  GD.resume();
}




void printError(int16_t  errorNum)
{
  Serial.print(F("Error("));
  Serial.print(errorNum);
  Serial.println(")");
}
