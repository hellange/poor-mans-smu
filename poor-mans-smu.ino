/****************************************************************
 *  
 *  Initial GUI prototype for
 *  P O O R  M A N ' s  S M U
 *  
 *  by Helge Langehaug (2018, 2019, 2020)
 * 
 *****************************************************************/

//uncomment the line below if you want to use real AD/DA 
//#define USE_SIMULATOR


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
#include "operations.h"
#include "Arduino.h"
#include "Wire.h"
#include "Mainmenu.h"
#include "SMU_HAL_dummy.h"
#include "SMU_HAL_717x.h"
#include "dial.h"
#include "FunctionPulse.h"
#include "FunctionSweep.h"
#include "Fan.h"
#include "RamClass.h"

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



#ifndef USE_SIMULATOR
ADCClass SMU[1] = {
  ADCClass()
};
#else
SMU_HAL_dummy SMU[1] = {
  SMU_HAL_dummy()
};
#endif

int scroll = 0;
int scrollDir = 0;


bool autoNullStarted = false;

int timeAtStartup;
bool startupCalibrationDone0 = false;
bool startupCalibrationDone1 = false;
bool startupCalibrationDone2 = false;

int noOfWidgets = 6;
int activeWidget = 0;

CURRENT_RANGE current_range = AMP1; // TODO: get rid of global
int timeSinceLastChange = 0;  // TODO: get rid of global

float MAX_CURRENT_10mA_RANGE = 5.0; // current values set because the ADC limit is 6 volt now...
float MAX_CURRENT_1A_RANGE = 1100.0;


OPERATION_TYPE operationType = SOURCE_VOLTAGE;


FUNCTION_TYPE functionType = SOURCE_DC;

OPERATION_TYPE getOperationType() {
  if (digitalRead(3) == HIGH) {
    return SOURCE_VOLTAGE;
  } else {
    return SOURCE_CURRENT;
  }
}
IntervalTimer myTimer;

#define SAMPLING_BY_INTERRUPT


void setup()
{
   disable_ADC_DAC_SPI_units();
   delay(50);
   //TODO: Organise pin numbers into definitions ?
   pinMode(6,OUTPUT); // LCD powerdown pin?
   digitalWrite(6, HIGH);
   pinMode(5,OUTPUT); // RAM

    pinMode(7,OUTPUT);
    pinMode(8,OUTPUT);
    pinMode(9,OUTPUT);
    pinMode(10,OUTPUT);

    pinMode(4,OUTPUT); // current range io pin for switching on/off 100ohm shunt
    digitalWrite(4, HIGH);

    pinMode(3,INPUT); // current range selector

    pinMode(2,INPUT); // Fan speed feedback
    FAN.init();
    
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
   SMU[0].setSamplingRate(20);
   operationType = getOperationType();

   if (operationType == SOURCE_VOLTAGE) {
     SMU[0].fltSetCommitVoltageSource(0.0, true);
     Serial.println("Source voltage");
     SMU[0].fltSetCommitLimit(0.1, _SOURCE_AND_SINK); 
   } else {
     SMU[0].fltSetCommitCurrentSource(0.0);
     Serial.println("Source current");
     SMU[0].fltSetCommitLimit(10.0, _SOURCE_AND_SINK); 
   }
   Serial.println("Done!");
   Serial.flush();

   V_STATS.init(DigitUtilClass::typeVoltage);
   C_STATS.init(DigitUtilClass::typeCurrent);
   
   V_FILTERS.init(1234);
   C_FILTERS.init(5678);
   
   V_CALIBRATION.init(SOURCE_VOLTAGE);
   C_CALIBRATION.init(SOURCE_CURRENT);

   SOURCE_DIAL.init();
   LIMIT_DIAL.init();

   FUNCTION_PULSE.init(/*SMU[0]*/);
   FUNCTION_SWEEP.init(/*SMU[0]*/);

   RAM.init();
  

   timeAtStartup = millis();

  // SPI.usingInterrupt(2);
  // pinMode(2,INPUT);
  // attachInterrupt(2, handleSampling, CHANGE);

#ifdef SAMPLING_BY_INTERRUPT
  myTimer.begin(handleSampling, 30); // in microseconds
  SPI.usingInterrupt(myTimer);
#endif


  //TC74 
  Wire.begin();
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

void sourcePulsePanel(int x, int y) {
  FUNCTION_PULSE.render(x,y);
}

void sourceSweepPanel(int x, int y) {
  FUNCTION_SWEEP.render(x,y);
}

void sourceCurrentPanel(int x, int y) {

  // heading
  GD.ColorRGB(COLOR_CURRENT);
  GD.ColorA(120);
  GD.cmd_text(x+20, y + 2 ,   29, 0, "SOURCE CURRENT");
  GD.cmd_text(x+20 + 1, y + 2 + 1 ,   29, 0, "SOURCE CURRENT");
  
  // primary
  CURRENT_DISPLAY.renderMeasured(x + 17,y + 26, C_FILTERS.mean, false, current_range == MILLIAMP10);

  // secondary volt
  GD.ColorRGB(COLOR_CURRENT);
  GD.ColorA(180); // a bit lighter
  DIGIT_UTIL.renderValue(x + 320,  y-4 , C_STATS.rawValue, 4, DigitUtilClass::typeCurrent); 

  GD.ColorA(255);
  CURRENT_DISPLAY.renderSet(x + 120, y + 131, SMU[0].getSetValuemV());

  GD.ColorRGB(0,0,0);
  GD.cmd_fgcolor(0xaaaa90);  
  
  GD.Tag(BUTTON_SOURCE_SET);
  GD.cmd_button(x + 20,y + 132,95,50,29,OPT_NOTEAR,"SET");
  
 
  GD.Tag(BUTTON_CUR_AUTO);
  GD.cmd_button(x+350,y+132,95,50,29,0,current_range==AMP1 ? "1A" : "10mA");
  GD.Tag(0); // Note: Prevents button in some cases to react also when touching other places in UI. Why ?
  
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
  
  GD.Tag(BUTTON_SOURCE_SET);
  GD.cmd_button(x + 20,y + 132,95,50,29,OPT_NOTEAR,"SET");
  GD.Tag(BUTTON_VOLT_AUTO);
  GD.cmd_button(x + 350,y + 132,95,50,29,0,"AUTO");
}

void renderStatusIndicators(int x, int y) {
  showStatusIndicator(x+630, y+5, "FILTER", V_FILTERS.filterSize>1, false);
  if (operationType == SOURCE_VOLTAGE) {
    showStatusIndicator(x+720, y+5, "NULL_V", V_CALIBRATION.nullValueIsSet(current_range), false);
  } else {
    showStatusIndicator(x+720, y+5, "NULL_C", C_CALIBRATION.nullValueIsSet(current_range), false);
  }
  showStatusIndicator(x+630, y+45, "50Hz", false, false);
  showStatusIndicator(x+720, y+45, "4 1/2", false, false);
  showStatusIndicator(x+630, y+85, "COMP", SMU[0].compliance, true);
    if (operationType == SOURCE_VOLTAGE) {
      showStatusIndicator(x+720, y+85, "UNCAL_V", !V_CALIBRATION.useCalibratedValues, true);
    } else {
      showStatusIndicator(x+720, y+85, "UNCAL_C", !C_CALIBRATION.useCalibratedValues, true);
    }
}

 void showAnalogPin(int x, int y, int radius, int radiusStart, int degreeRelativeToTop, int needleColor, int lineWidth, boolean needle) {
  if (reduceDetails()){
    return;
  }
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


bool anyDialogOpen() {
  // make sure buttons below the dialog do not reach if finger is not removed from screen
  // when dialog disappears. Use a timer for now...
  return SOURCE_DIAL.isDialogOpen() or LIMIT_DIAL.isDialogOpen();
}

// TODO: Move to separate util class ?
int timeSinceIndicated = millis() + 1000;
int indicateColor(int normalColor, int indicatorColor,int period) {
   if (timeSinceIndicated + 100 > millis()) {
     return indicatorColor;
   } else {
     return normalColor;
   }
}
void startIndicator() {
  timeSinceIndicated= millis();
}

void handleSliders(int x, int y) { 
  
  y=y+40;

  GD.Tag(TAG_FILTER_SLIDER);
  GD.cmd_slider(500+x, y+30, 280,15, OPT_FLAT, V_FILTERS.filterSize * (65535/100), 65535);
  GD.cmd_track(500+x, y+30, 280, 20, TAG_FILTER_SLIDER);
  
  GD.Tag(TAG_FILTER_SLIDER_B);
  GD.cmd_slider(500+x, y+90, 280,15, OPT_FLAT, V_STATS.getNrOfSamplesBeforeStore()* (65535/100), 65535);
  GD.cmd_track(500+x, y+90, 280, 20, TAG_FILTER_SLIDER_B);
  
  GD.cmd_text(500+x,y, 27, 0, "Filter size:");
  GD.cmd_number(580+x,y, 27, 0, V_FILTERS.filterSize);
  GD.cmd_text(500+x,y+60, 27, 0, "Samples size:");
  GD.cmd_number(605+x,y+60, 27, 0, V_STATS.getNrOfSamplesBeforeStore());

  if (!anyDialogOpen()) {
   GD.Tag(BUTTON_CLEAR_BUFFER);
//   GD.ColorRGB(0x000000);
//   // just make sure that the button is lit a short period to indicate that you have pusked it...
//   if (timeSinceButtonPushed + 100 > millis()) {
//     GD.ColorRGB(0x00ff00);
//   } else {
//     GD.ColorRGB(0x000000);
//   }
    GD.ColorRGB(indicateColor(0x000000, 0x00ff00, 50));
    GD.cmd_button(x+500,y+130,95,50,29,0,"CLEAR");
  }
  
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
  }
  
  GD.Tag(BUTTON_UNCAL);
  GD.cmd_button(x+600,y+130,95,50,29,0,"UNCAL");
  GD.Tag(0); // hack to avoid UNCAL button to be called when you press SET on voltage. (when the experimental widget is shown)... 
             // Dont fully understand why this is needed...
  
  GD.ColorA(255);

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
  if (!reduceDetails()) {
    renderAnalogGauge(x+90,y,240, degrees, deviationInPercent, "Deviation from SET");
  }
  GD.ColorRGB(0x000000);
  GD.Tag(BUTTON_SAMPLE_RATE_5);
  GD.cmd_button(x-50,y,95,40,29,0,"5Hz");

  
  GD.Tag(BUTTON_SAMPLE_RATE_20);
  GD.cmd_button(x-50,y+50,95,40,29,0,"20Hz");

  
  GD.Tag(BUTTON_SAMPLE_RATE_100);
  GD.cmd_button(x-50,y+100,95,40,29,0,"100Hz");

 
  
  GD.Tag(0);
  
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

  if (x+gaugeRadius < 800) {
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
  }
  
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
  if (reduceDetails()) {
    return;
  }
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
    int position_x1 = x+20 + i*14;
    int position_x2 = position_x1 + 11;
    if (position_x1 > 0 && position_x2 < 800) {
      GD.Vertex2ii(position_x1 ,y);
      GD.Vertex2ii(x+20 + (i+1)*14 - 3, y+9);
    }
    
  }
}

void measureVoltagePanel(int x, int y, boolean compliance) {
  if (x >= 800) {
    return;
  }
  y=y+28;

  VOLT_DISPLAY.renderMeasured(x + 17, y, V_FILTERS.mean);
  VOLT_DISPLAY.renderSet(x+120, y+105, SMU[0].getLimitValue());

  y=y+105;
  
  GD.ColorRGB(0,0,0);
  GD.cmd_fgcolor(0xaaaa90);  
  GD.Tag(BUTTON_LIM_SET);
  GD.cmd_button(x+20,y,95,50,29,0,"LIM");
 
  GD.Tag(0); // Note: Prevents button in some cases to react also when touching other places in UI. Why ?
  
}
void measureCurrentPanel(int x, int y, boolean compliance, bool showBar) {
  if (x >= 800) {
    return;
  }
  
  y=y+28;
  if ( (current_range == MILLIAMP10 && abs(C_STATS.rawValue) > MAX_CURRENT_10mA_RANGE) or (current_range == AMP1 && abs(C_STATS.rawValue) > MAX_CURRENT_1A_RANGE)) {
    if (showBar) {
      y=y+12; // dont show bar when overflow... just add extra space so the panel gets same size as without overflow...
    }
    GD.ColorA(255);
    CURRENT_DISPLAY.renderOverflow(x + 17, y);
  } else {
    if (showBar) {
      renderBar(x,y, C_STATS.rawValue, SMU[0].getLimitValue());
      y=y+12;
    }
    GD.ColorA(255);
    CURRENT_DISPLAY.renderMeasured(x + 17, y, C_FILTERS.mean, compliance, current_range == MILLIAMP10); 
  }
  CURRENT_DISPLAY.renderSet(x+120, y+105, SMU[0].getLimitValue());

  y=y+105;
  
  GD.ColorRGB(0,0,0);
  GD.cmd_fgcolor(0xaaaa90);  
  GD.Tag(BUTTON_LIM_SET);
  GD.cmd_button(x+20,y,95,50,29,0,"LIM");
  GD.Tag(BUTTON_CUR_AUTO);
  GD.cmd_button(x+350,y,95,50,29,0,current_range==AMP1 ? "1A" : "10mA");
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
  if (widgetNo ==0 && operationType == SOURCE_VOLTAGE) {
     if (scroll ==0){
       GD.ColorRGB(COLOR_CURRENT_TEXT);
       GD.cmd_text(20, yPos, 29, 0, "MEASURE CURRENT");
     }
     measureCurrentPanel(scroll, yPos + 20, SMU[0].compliance, true);
  } else if (widgetNo ==0 && operationType == SOURCE_CURRENT) {
     if (scroll ==0){
       GD.ColorRGB(COLOR_VOLT);
       GD.cmd_text(20, yPos, 29, 0, "MEASURE VOLTAGE");
     }
     measureVoltagePanel(scroll, yPos + 20, SMU[0].compliance);
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

      if (operationType == SOURCE_VOLTAGE) {
        float rawM = V_FILTERS.mean;
        float setM = SMU[0].getSetValuemV();
        renderExperimental(scroll,yPos, rawM, setM, false);
      } else {
        float rawM = C_FILTERS.mean;
        float setM = SMU[0].getSetValuemV();
        renderExperimental(scroll,yPos, rawM, setM, false);
      }
  } else if (widgetNo == 5) {
      if (scroll ==0){
        GD.ColorRGB(COLOR_VOLT);
        GD.cmd_text(20, yPos, 29, 0, "CAL");

        if (operationType == SOURCE_VOLTAGE) {
          float rawM = V_FILTERS.mean;
          float setM = SMU[0].getSetValuemV();
          V_CALIBRATION.renderCal(scroll,yPos, rawM, setM, false, reduceDetails());
        } else {
          float rawM = C_FILTERS.mean;
          float setM = SMU[0].getSetValuemV();
          C_CALIBRATION.renderCal(scroll,yPos, rawM, setM, false, reduceDetails());
        }
      }
      
  }

}

bool reduceDetails() {
  return scrollDir != 0 || MAINMENU.active == true or anyDialogOpen();
}


int gestureDetected = GEST_NONE;
int scrollSpeed = 100;
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

void displayWidget() {
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
  
}

void bluredBackground() {
    GD.Begin(RECTS);
    GD.ColorA(150);
    GD.ColorRGB(0x000000);
    GD.Vertex2ii(0,0);
    GD.Vertex2ii(800, 480);
}

void handleMenuScrolldown(){

  if (gestureDetected == GEST_MOVE_DOWN && MAINMENU.active == false) {
    MAINMENU.open(closeMainMenuCallback);
    return; // start the animation etc. next time, so UI that needs to reduce details have time to reach.
  }  
  
  // main menu
  if (MAINMENU.active) {
    MAINMENU.render();
  }
 
}

void renderMainHeader() {
  // register screen for gestures on top half, required for pulling menu from top
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
  GD.cmd_text(20, 0, 27, 0, "Input 25.4V / - 25.3V"); // NOTE: Currently only dummy info
  showLoadResistance(590,0);
  showFanSpeed(220,0);
  //GD.cmd_number(470,0,27,3,LM60_getTemperature());
  GD.cmd_number(470,0,27,3,TC74_getTemperature());

  GD.cmd_text(500,0,27,0,"C");

  GD.cmd_number(370,0,27,0,RAM.getCurrentLogAddress());

  // line below top header
  int y = 25;
  GD.Begin(LINE_STRIP);
  GD.ColorA(200);
  GD.LineWidth(15);
  GD.ColorRGB(COLOR_VOLT);
  GD.Vertex2ii(1,y);
  GD.Vertex2ii(799, y);
  GD.ColorA(255);
}

void showFanSpeed(int x, int y) {
   GD.cmd_text(x,y,27,0, "Fan:");
  GD.cmd_number(x+30,y,27,5, FAN.getRPMValueFiltered());
  GD.cmd_text(x+80,y,27,0, "RPM");
//  Serial.print(FAN.getPWMFanRPM());
//  Serial.print("(");
//  Serial.print(FAN.getFanWidth());
//  Serial.flush();
}
void showLoadResistance(int x, int y) {
   // calculate resistance only if the voltage and current is above some limits. Else the calculation will just be noise...
    if(abs(V_FILTERS.mean) > 10.0 or abs(C_FILTERS.mean) > 0.010) {
      float resistance = abs(V_FILTERS.mean / C_FILTERS.mean);
      GD.ColorRGB(0xdddddd);
      int kOhm, ohm, mOhm;
      bool neg;
      //DIGIT_UTIL.separate(&a, &ma, &ua, &neg, rawMa);
      DIGIT_UTIL.separate(&kOhm, &ohm, &mOhm, &neg, resistance);

      if (kOhm > 0) {
        GD.cmd_number(x, y, 27, 3, kOhm);
        GD.cmd_text(x+30, y,  27, 0, ".");
        GD.cmd_number(x+35, y, 27, 3, ohm);
        GD.cmd_text(x+70, y,  27, 0, "kOhm");
      } else {
        GD.cmd_number(x, y, 27, 3, ohm);
        GD.cmd_text(x+30, y,  27, 0, ".");
        GD.cmd_number(x+35, y, 27, 3, mOhm);
        GD.cmd_text(x+70, y,  27, 0, "ohm");
      }

    }
}
void renderUpperDisplay(OPERATION_TYPE operationType, FUNCTION_TYPE functionType) {

  int x = 0;
  int y = 32;
 
  // show upper panel
  if (functionType == SOURCE_DC) {
    if (operationType == SOURCE_VOLTAGE) {
        sourceVoltagePanel(x,y);
    } else {
        sourceCurrentPanel(x,y);
    }
    renderStatusIndicators(x,y);

  } else if (functionType == SOURCE_PULSE) {
    sourcePulsePanel(x,y);
  } else if (functionType == SOURCE_SWEEP) {
    sourceSweepPanel(x,y);
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

int timeBeforeAutoNull = millis() + 5000;

void forceAutoNull() {
  Serial.println("Force auto null...");
  timeBeforeAutoNull = millis();
  startupCalibrationDone0 = false;
  startupCalibrationDone1 = false;
  startupCalibrationDone2 = false;
  autoNullStarted = true;

}

void handleAutoNullAtStartup() {

//   Serial.print("x ");
//   Serial.print(startupCalibrationDone0);
//   Serial.print(" ");
//   Serial.println(timeBeforeAutoNull < millis());
  if (autoNullStarted && !startupCalibrationDone0 && timeBeforeAutoNull < millis()) {
    Serial.println("Performing auto null...");
    V_CALIBRATION.useCalibratedValues = false;
    current_range = MILLIAMP10;
    //Serial.println("Null calibration initiated...");
    SMU[0].setCurrentRange(current_range);
    if (operationType == SOURCE_VOLTAGE) {
      SMU[0].fltSetCommitVoltageSource(0.0, true);
    } else {
      SMU[0].fltSetCommitCurrentSource(0.0);
    }
    //V_FILTERS.init();
    SMU[0].setSamplingRate(5);
    V_FILTERS.setFilterSize(5);
    startupCalibrationDone0 = true;
   
  }
  int msWaitPrCal = 5000;
  if (autoNullStarted && !startupCalibrationDone1 && /*timeAtStartup + */timeBeforeAutoNull + msWaitPrCal < millis()) {
    //float v = V_STATS.rawValue; 
    float v = V_FILTERS.mean;   
    V_CALIBRATION.toggleNullValue(v, current_range);
    Serial.print("Removed voltage offset from 10mA range:");  
    Serial.println(v,3);  
    //v = C_STATS.rawValue;
    
    v = C_FILTERS.mean;   
    C_CALIBRATION.toggleNullValue(v , current_range);
    Serial.print("Removed current offset from 10mA range:");  
    Serial.println(v,3);
    startupCalibrationDone1 = true;
  } 
 
  if (autoNullStarted && !startupCalibrationDone2 && /*timeAtStartup + */timeBeforeAutoNull + msWaitPrCal + 100 < millis()) {
    current_range = AMP1;
    SMU[0].setCurrentRange(current_range);
    if (operationType == SOURCE_VOLTAGE) {
      SMU[0].fltSetCommitVoltageSource(0.0, true);
    } else {
      SMU[0].fltSetCommitCurrentSource(0.0);
    }
  }
  if (autoNullStarted && !startupCalibrationDone2 && /*timeAtStartup +*/ timeBeforeAutoNull + msWaitPrCal*2 < millis()) {
    //float v = V_STATS.rawValue;
    float v = V_FILTERS.mean;
    V_CALIBRATION.toggleNullValue(v, current_range);
    Serial.print("Removed voltage offset from 1A range:");  
    Serial.println(v,3);  
    //v = C_STATS.rawValue;
    v = C_FILTERS.mean;
    C_CALIBRATION.toggleNullValue(v,current_range);
    Serial.print("Removed current offset from 1A current range:");  
    Serial.println(v,3);  
    startupCalibrationDone2 = true;
    //V_FILTERS.init(1234);
    V_FILTERS.setFilterSize(5);
    //C_FILTERS.init(2345);
    C_FILTERS.setFilterSize(5);
    V_CALIBRATION.useCalibratedValues = true;
    autoNullStarted = false;

  } 

}

void notification(char *text) {
   GD.Begin(RECTS);
    GD.ColorA(230);  // already opaque, why ?
    GD.ColorRGB(0x222222);
    GD.Vertex2ii(180, 160);
    GD.Vertex2ii(620, 280);
    GD.ColorRGB(0xffffff);
    GD.cmd_text(250, 200 , 29, 0, text);
}


bool digitize = false;

int ramAdrPtr = 0;
int maxFloats = 32000; // 32000 is max for the ram
int nrOfFloats = 100;

bool bufferOverflow = false;
float maxDigV = -100000.00, minDigV = 100000.00, curDigV;
float maxDigI = -100000.00, minDigI = 100000.00, curDigI;
int digitizeDuration = millis();
float lastVoltage = 0.0;
bool triggered = false;
int count = 1;
float sampleVolt = 10.0;
int logTimer = millis();
static void handleSampling() {

     int dataR = SMU[0].dataReady();
      //Serial.print("DataReady:");  
      //Serial.println(dataR, HEX); 

 
         
   if (digitize == true && bufferOverflow==false && (dataR == 0 or dataR == 1)) {

    
//     if (count % 20== 0) {
//       SMU[0].fltSetCommitVoltageSource(sampleVolt, false);
//       if (sampleVolt == 2000.0) {
//        sampleVolt = -2000.0;
//       } else {
//        sampleVolt = 2000.0;
//       }
//     }
//     count ++;

       if (dataR == 1) {
        return;
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

      float v = SMU[0].measureMilliVoltage();   
      bool continuous = true;
      if (!triggered) {
        if (continuous or (lastVoltage < 1000.0 && v > 1500.0)) {
          triggered = true;
          Serial.println("Triggered!");
          Serial.println(lastVoltage);
        } else {
          lastVoltage = v;
          return;
        }
      }
      lastVoltage = v;

      RAM.writeRAMfloat(ramAdrPtr, v);
      if (ramAdrPtr == 0) {
        digitizeDuration = millis();
      }
      ramAdrPtr += 4;
      if (ramAdrPtr > nrOfFloats*4) {
        bufferOverflow = true;
        triggered = false;
        //lastVoltage = 0.0;
        Serial.println("Overflow!");
        ramAdrPtr = 0;
      }
      curDigV = v;
      if (v > maxDigV) {
        maxDigV = v;
      } 
      if (v < minDigV) {
        minDigV = v;
      }      
      return;
   }
 
  if (dataR == -1) {
    return;
  }
  if (dataR == -99) {
    Serial.println("DONT USE SAMPLE!");  
  } 
  else if (dataR == -98) {
    Serial.println("OVERFLOW"); // haven't been able to get this to work...
  }
  
  else if (dataR == 1) {
    
    float Cout = SMU[0].measureCurrent(current_range);

    Cout = Cout - C_CALIBRATION.nullValue[current_range];

    C_STATS.addSample(Cout);
    C_FILTERS.updateMean(Cout, false);
   
//    
//  Serial.print("Measured raw:");  
//  Serial.print(Cout, 3);
//  Serial.print(" mA");  
//  Serial.print(", mean:");
//  Serial.println(C_FILTERS.mean);
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



    
    V_FILTERS.updateMean(Vout, true);

    // store now and then
    if (logTimer + 30000 < millis()) {
     logTimer = millis();
     RAM.logData(V_FILTERS.mean);
    }
    
  }
  // Auto range current measurement while sourcing voltage. 
  // Note that this gives small glitches in voltage.
  // TODO: Find out how large glitches and if it's a real problem...
  // TODO: Not in digitizing?  Other functions where it should be disabled ?
  if (operationType == SOURCE_VOLTAGE) {
      //handleAutoCurrentRange();
  }
  
}

void handleAutoCurrentRange() {
     if (!autoNullStarted && !V_CALIBRATION.autoCalInProgress && !C_CALIBRATION.autoCalInProgress) {
      float milliAmpere = C_STATS.rawValue;
//      Serial.print(milliAmpere,5);
//      Serial.print("mA, current range:");
//      Serial.println(current_range);

      // auto current range switch. TODO: Move to hardware ? Note that range switch also requires change in limit
      float hysteresis = 0.5;
      float switchAt = MAX_CURRENT_10mA_RANGE;
      
        if (current_range == AMP1 && abs(milliAmpere) < switchAt - hysteresis) {
          current_range = MILLIAMP10;
          SMU[0].setCurrentRange(current_range);
          Serial.println("switching to range 1");
  
        }
        // TODO: Make separate function to calculate current based on shunt and voltage!
        Serial.print("Check 10mA range and if it should switch to 1A... ma=");
        Serial.print(milliAmpere);
        Serial.print(" ");
        Serial.println(switchAt);
        if (current_range == MILLIAMP10 && abs(milliAmpere) > switchAt) {
          current_range = AMP1;
          SMU[0].setCurrentRange(current_range);
          Serial.println("switching to range 0");
        }
    }
}

int displayUpdateTimer = millis();

int TC74_getTemperature() {
  // I2C based
  int temperature;
    Wire.beginTransmission(72);
    //start the transmission

   byte val = 0;
    Wire.write(val);

    Wire.requestFrom(72, 1);
    if (Wire.available()) {
    temperature = Wire.read();
    //Serial.println(temperature);
    }
  Wire.endTransmission();
  return temperature;
  
}

int LM60_getTemperature() {
   analogReadRes(10);
  float maxNumber = 1023;//4095.0;
  float refV = 3.3; // LM60 connected to 3.3V
  float ar = analogRead(1);
  float voltage = refV*(ar/maxNumber);
  // DM60 datasheet: Vout = (+6.25mV x t) +424mV 
  //                 => Vout - 424mV = +6.25mV x t 
  //                 => t = (Vout-424mV) / 6.25mV
  int temp = (voltage - 0.424) / 0.00625;
  //Serial.print(" ");
  //Serial.println(temp,3);
  return (int)temp;
}

void loop() {



  
  V_CALIBRATION.autoCalADCfromDAC();
  C_CALIBRATION.autoCalADCfromDAC();
  // No need to update display more often that the eye can detect.
  // Too often will cause jitter in the sampling because display and DAC/ADC share same SPI port.
  // Will that be improved by Teensy 4 where there are more that one SPI port ?
  //
  // Note that the scrolling speed and gesture detection speed will be affected.
  // 
  if (displayUpdateTimer + 20 > millis()) {
    return; 
  }
  //Serial.print("Temperature:");
  //Serial.println(LM60_getTemperature());
 
   
  displayUpdateTimer = millis();
  
  if (functionType == DIGITIZE) {
    loopDigitize();
  } else if (functionType == GRAPH) {
   loopGraph();
  }
    else {
    loopMain();
  }
}

int loopUpdateTimer = millis();


void loopGraph() {
  if (loopUpdateTimer + 10 > millis() ) {
    return;
  }
  loopUpdateTimer = millis();
  operationType = getOperationType();
    disable_ADC_DAC_SPI_units();

 GD.resume();
      GD.Clear();
detectGestures();
    renderMainHeader();

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


      handleMenuScrolldown();

 int tag = GD.inputs.tag;
  // // TODO: don't need to check buttons for inactive menus or functions...
  MAINMENU.handleButtonAction(tag);
 
      GD.swap();
      GD.__end();
}


int samplingDelayTimer = millis();

void loopDigitize() {
  if (loopUpdateTimer + 10 > millis() ) {
    return;
  }
  
  loopUpdateTimer = millis();
  //handleAutoNullAtStartup();
  operationType = getOperationType();
  disable_ADC_DAC_SPI_units();
  

   if (digitize == false && !MAINMENU.active /*&&  samplingDelayTimer + 50 < millis()*/){
     
    ramAdrPtr = 0;
     minDigV = 1000000;
     maxDigV = - 1000000;
     minDigI = 1000000;
     maxDigI = - 1000000;
     SMU[0].setSamplingRate(10000);
     nrOfFloats = 400;
          //bufferOverflowed=false;
     digitize = true;
         bufferOverflow = false;

     
   }

  disable_ADC_DAC_SPI_units();
  
  if (bufferOverflow == true) {
    digitize = false;
    Serial.println("Initiate new samping round...");
    samplingDelayTimer = millis();
    
  }
  //TODO: Move digitizine to a separate class
  if (bufferOverflow == false && digitize == true){
//    //GD.cmd_number(100, 100, 1, 6, ramAdrPtr);  
//    GD.resume();
//    GD.Clear();
//    VOLT_DISPLAY.renderMeasured(100,200, maxDigV);
//    VOLT_DISPLAY.renderMeasured(100,300, minDigV);
//    //VOLT_DISPLAY.renderMeasured(100,400, curDigV);
//    Serial.println(ramAdrPtr);
//    GD.swap();
//    GD.__end();
  } else {
         SMU[0].setSamplingRate(10);

    //digitize = false; 
    bufferOverflow = false;

    
    GD.resume();
      GD.Clear();
        //SMU[0].setSamplingRate(100);

        

    detectGestures();
    renderMainHeader();
    VOLT_DISPLAY.renderMeasured(10,50, minDigV);
    VOLT_DISPLAY.renderMeasured(10,150, maxDigV);
    CURRENT_DISPLAY.renderMeasured(10,250, minDigI, false, false);
    CURRENT_DISPLAY.renderMeasured(10,350, maxDigI, false, false);

  GD.Begin(LINE_STRIP);
  GD.ColorA(255);
  GD.ColorRGB(0x00ff00);

  float mva[400];
  float mia[400];

  GD.__end();
  for (int x = 0; x<nrOfFloats/2; x++) {
    int adr = x*8;
    mva[x] = RAM.readRAMfloat(adr);
    mia[x] = RAM.readRAMfloat(adr+4);
  }
  GD.resume();
  
  for (int x = 0; x<nrOfFloats/2; x++) {
    int adr = x*8;
    //GD.__end();
    //float mv = RAM.readRAMfloat(adr);
    //float i = RAM.readRAMfloat(adr+4);
    //GD.resume();
   // Serial.println(mv,5);

   if (minDigV < 100000) {
      //float span = (maxDig - minDig);
      float mid = (maxDigV + minDigV) /2.0;
      float relative = mid - mva[x];
     
      float y = relative*50;
     // float y = mva[x] / 100.0;
      GD.Vertex2ii(x*4, 200 - y);
   }
    
  }

  GD.Begin(LINE_STRIP);
  GD.ColorA(255);
  GD.ColorRGB(0xff0000);

  for (int x = 0; x<nrOfFloats/2; x++) {
    int adr = x*8;
    //GD.__end();
    //float mv = RAM.readRAMfloat(adr);
    //float i = RAM.readRAMfloat(adr+4);
    //GD.resume();
   // Serial.println(mv,5);

    if (minDigI < 100000) {
      float relative = minDigI - mia[x];
      //float y = relative*50;
      float y = mia[x]/10.0;
      GD.Vertex2ii(x*4, 400 - y);
    }
    
  }
  
    //VOLT_DISPLAY.renderMeasured(100,50, V_FILTERS.mean);
    //CURRENT_DISPLAY.renderMeasured(100,150, C_FILTERS.mean, false, false);

      handleMenuScrolldown();

  int tag = GD.inputs.tag;
   // TODO: don't need to check buttons for inactive menus or functions...
  MAINMENU.handleButtonAction(tag);
   GD.swap();
    GD.__end();
  
  }
  
}

void loopMain()
{


 
  handleAutoNullAtStartup();
  operationType = getOperationType();
  

  if (startupCalibrationDone2) {
   
    //if (!V_CALIBRATION.autoCalDone) {
    //  V_CALIBRATION.startAutoCal();
    //}

    RAM.startLog();
    
  }
  // Auto range current measurement while sourcing voltage. 
  // Note that this gives small glitches in voltage.
  // TODO: Find out how large glitches and if it's a real problem...
  //if (operationType == SOURCE_VOLTAGE) {
  //    handleAutoCurrentRange();
  //}
  GD.__end();

  #ifndef SAMPLING_BY_INTERRUPT 
    handleSampling(); 
  #endif
  disable_ADC_DAC_SPI_units();
  GD.resume();

  if (!gestureDetected) {
    int tag = GD.inputs.tag;

    if (tag == BUTTON_SOURCE_SET) {
      Serial.println("Source set");
      SOURCE_DIAL.open(operationType, SET,  closeSourceDCCallback, SMU[0].getSetValuemV());
    } else if (tag == BUTTON_LIM_SET) {
      Serial.println("Limit set");
      LIMIT_DIAL.open(operationType, LIMIT, closeSourceDCCallback, SMU[0].getLimitValue());
    } else if (tag == BUTTON_NULL) {
      Serial.println("Null set");
      V_CALIBRATION.toggleNullValue(V_STATS.rawValue, current_range);
      C_CALIBRATION.toggleNullValue(C_STATS.rawValue, current_range);
    } else if (tag == BUTTON_UNCAL) {
      Serial.println("Uncal set");
      V_CALIBRATION.toggleCalibratedValues();
      C_CALIBRATION.toggleCalibratedValues();
    } else if (tag == BUTTON_CLEAR_BUFFER) {
      Serial.println("clearbuffer set");
      V_STATS.clearBuffer();
      C_STATS.clearBuffer();
      startIndicator(); 
    } else if (tag == BUTTON_CUR_AUTO) { //TODO: Change name
      if (timeSinceLastChange + 500 < millis()){
        Serial.println("current range set");
       
        // swap current range
        if (current_range == AMP1) {
          current_range = MILLIAMP10;
        } else {
          current_range = AMP1;
        }
        timeSinceLastChange = millis();
        SMU[0].setCurrentRange(current_range);
        if (operationType == SOURCE_CURRENT){
          //Serial.println("SHOULD SET NEW OUTPUT WHEN SWITCHING CURRENT RANGE IN SOURCE CURRENT MODE ????");
          //if (SMU[0].fltSetCommitCurrentSource(SMU[0].getSetValuemV())) printError(_PRINT_ERROR_VOLTAGE_SOURCE_SETTING);
        }

      }
    } else if (tag == BUTTON_SAMPLE_RATE_5 or tag == BUTTON_SAMPLE_RATE_20 or tag == BUTTON_SAMPLE_RATE_100) { //TODO: Change name
      if (timeSinceLastChange + 500 < millis()){
        timeSinceLastChange = millis();
      } 
     
      if (tag == BUTTON_SAMPLE_RATE_5) {
        SMU[0].setSamplingRate(5);
      }
      if (tag == BUTTON_SAMPLE_RATE_20) {
        SMU[0].setSamplingRate(20);
      }
      if (tag == BUTTON_SAMPLE_RATE_100) {
        SMU[0].setSamplingRate(100);
      }

    } 
    #ifndef USE_SIMULATOR // dont want to mess up calibration while in simulation mode...
    else if (tag == BUTTON_DAC_GAIN_COMP_POS_UP) {
       if (timeSinceLastChange + 500 > millis()){
        return;
      } 
       timeSinceLastChange = millis();
       float mv = SOURCE_DIAL.getMv();
       if (operationType == SOURCE_VOLTAGE) {
         if (mv < 0) {
            V_CALIBRATION.adjDacGainCompNeg(0.000005);
         } else {
            V_CALIBRATION.adjDacGainCompPos(0.000005);
         }
         GD.__end();
         if (SMU[0].fltSetCommitVoltageSource(mv, true)) printError(_PRINT_ERROR_VOLTAGE_SOURCE_SETTING);
         GD.resume();
       } else {
          if (mv < 0) {
            C_CALIBRATION.adjDacGainCompNeg(0.00001);
         } else {
            C_CALIBRATION.adjDacGainCompPos(0.00001);
         }
         GD.__end();
         if (SMU[0].fltSetCommitCurrentSource(mv)) printError(_PRINT_ERROR_VOLTAGE_SOURCE_SETTING);
         GD.resume();
       }
      
    } else if (tag == BUTTON_DAC_GAIN_COMP_POS_DOWN) {
       if (timeSinceLastChange + 200 > millis()){
        return;
      } 
       timeSinceLastChange = millis();

       float mv = SOURCE_DIAL.getMv();
       if (operationType == SOURCE_VOLTAGE) {
         if (mv < 0) {
            V_CALIBRATION.adjDacGainCompNeg(-0.000001);
         } else {
            V_CALIBRATION.adjDacGainCompPos(-0.000001);
         }
         //if (operationType == SOURCE_VOLTAGE) {
         GD.__end();
         if (SMU[0].fltSetCommitVoltageSource(mv, true)) printError(_PRINT_ERROR_VOLTAGE_SOURCE_SETTING);
         GD.resume();
       } else {
          if (mv < 0) {
            C_CALIBRATION.adjDacGainCompNeg(-0.00001);
         } else {
            C_CALIBRATION.adjDacGainCompPos(-0.00001);
         }
         GD.__end();
         if (SMU[0].fltSetCommitCurrentSource(mv)) printError(_PRINT_ERROR_VOLTAGE_SOURCE_SETTING);
         GD.resume();
       }
       
    } else if (tag == BUTTON_ADC_GAIN_COMP_POS_UP) {
       if (timeSinceLastChange + 200 > millis()){
        return;
        
      } 
              timeSinceLastChange = millis();

       float mv = SOURCE_DIAL.getMv();
       if (operationType == SOURCE_VOLTAGE) {
  
         if (mv < 0) {
            V_CALIBRATION.adjAdcGainCompNeg(0.000001);
         } else {
            V_CALIBRATION.adjAdcGainCompPos(0.000001);
         }
       } else {
         if (mv < 0) {
            C_CALIBRATION.adjAdcGainCompNeg(0.00001);
         } else {
            C_CALIBRATION.adjAdcGainCompPos(0.00001);
         }
       }
      
    } else if (tag == BUTTON_ADC_GAIN_COMP_POS_DOWN) {
       if (timeSinceLastChange + 200 > millis()){
        return;
      } 
              timeSinceLastChange = millis();

       float mv = SOURCE_DIAL.getMv();
       if (operationType == SOURCE_VOLTAGE) {
         if (mv < 0) {
            V_CALIBRATION.adjAdcGainCompNeg(-0.000001);
         } else {
            V_CALIBRATION.adjAdcGainCompPos(-0.000001);
         }
       } else {
        if (mv < 0) {
            C_CALIBRATION.adjAdcGainCompNeg(-0.00001);
         } else {
            C_CALIBRATION.adjAdcGainCompPos(-0.00001);
         }
       }
      
       
    } else if (tag == BUTTON_DAC_ZERO_COMP_UP) {
       if (timeSinceLastChange + 200 > millis()){
        return;
      } 
       timeSinceLastChange = millis();

       float mv = SOURCE_DIAL.getMv();
       if (operationType == SOURCE_VOLTAGE) {
          V_CALIBRATION.adjDacZeroComp(+0.000002);

          GD.__end();
         if (SMU[0].fltSetCommitVoltageSource(mv, true)) printError(_PRINT_ERROR_VOLTAGE_SOURCE_SETTING);
         GD.resume();
         
       } else {
          C_CALIBRATION.adjDacZeroComp(+0.000002);
          GD.__end();
         if (SMU[0].fltSetCommitCurrentSource(mv)) printError(_PRINT_ERROR_VOLTAGE_SOURCE_SETTING);
         GD.resume();

          
       }

       
       
    }
    else if (tag == BUTTON_DAC_ZERO_COMP_DOWN) {
       if (timeSinceLastChange + 200 > millis()){
        return;
      } 
       timeSinceLastChange = millis();

       float mv = SOURCE_DIAL.getMv();
       if (operationType == SOURCE_VOLTAGE) {
          V_CALIBRATION.adjDacZeroComp(-0.000002);

          GD.__end();
         if (SMU[0].fltSetCommitVoltageSource(mv, true)) printError(_PRINT_ERROR_VOLTAGE_SOURCE_SETTING);
         GD.resume();
         
       } else {
          C_CALIBRATION.adjDacZeroComp(-0.000002);
          GD.__end();
         if (SMU[0].fltSetCommitCurrentSource(mv)) printError(_PRINT_ERROR_VOLTAGE_SOURCE_SETTING);
         GD.resume();

          
       }

       
       
    }
    
    else if (tag == BUTTON_DAC_NONLINEAR_CALIBRATE) {
       if (timeSinceLastChange + 1000 > millis()){
        return;
       } 
       timeSinceLastChange = millis();
       if (operationType == SOURCE_VOLTAGE) {
               Serial.println("Start auto calibration of dac non linearity in voltage source mode");

         V_CALIBRATION.startAutoCal();
       } else {
                       Serial.println("Start auto calibration of dac non linearity in current source mode");

         C_CALIBRATION.startAutoCal();
       }
       
    
    } else if (tag == BUTTON_DAC_ZERO_CALIBRATE) {
      if (timeSinceLastChange + 1000 > millis()){
        return;
       } 
       Serial.println("Start zero calibration of dac....");
       timeSinceLastChange = millis();
       forceAutoNull();
       
    }
    #endif // end not using simulator
    
    // TODO: don't need to check buttons for inactive menus or functions...
    MAINMENU.handleButtonAction(tag);
    FUNCTION_PULSE.handleButtonAction(tag);
    FUNCTION_SWEEP.handleButtonAction(tag);

    
  }

  GD.Clear();
  renderMainHeader();
  renderUpperDisplay(operationType, functionType);

   // register screen for gestures on lower half
  GD.Tag(GESTURE_AREA_LOW);
  GD.Begin(RECTS);
  GD.ColorRGB(0x000000);
  GD.Vertex2ii(0,LOWER_WIDGET_Y_POS);
  GD.Vertex2ii(800, 480);

  detectGestures();

  handleWidgetScrollPosition();
  displayWidget();
  handleMenuScrolldown();

  if (V_CALIBRATION.autoCalInProgress or C_CALIBRATION.autoCalInProgress) {
    notification("Auto calibration in progress...");
  }
  if (autoNullStarted && !startupCalibrationDone2) {
    notification("Wait for null adjustment...");
  }

  if (anyDialogOpen()) {
    bluredBackground();
    if (SOURCE_DIAL.isDialogOpen()){
      SOURCE_DIAL.checkKeypress();
      SOURCE_DIAL.handleKeypadDialog();
    } else if (LIMIT_DIAL.isDialogOpen()) {
      LIMIT_DIAL.checkKeypress();
      LIMIT_DIAL.handleKeypadDialog();
    }

  }

  GD.swap(); 
  GD.__end();
}

void closedPulse(OPERATION_TYPE t) {
  
}

void closedSweep(OPERATION_TYPE t) {
  
}


void closeMainMenuCallback(FUNCTION_TYPE functionType_) {
  
  Serial.print("Closed main menu callback. Selected function:");
  Serial.println(functionType_);
  Serial.flush();

  // do a close on the existing function
  if (functionType == SOURCE_PULSE) {
    FUNCTION_PULSE.close();
  } else if (functionType == SOURCE_SWEEP) {
    FUNCTION_SWEEP.close();
  }
   
  // TODO: Add cleanup from previous function before starting new...
  functionType = functionType_;
  if (functionType == SOURCE_PULSE) {
    FUNCTION_PULSE.open(operationType, closedPulse);
  } else if (functionType == SOURCE_SWEEP) {
    FUNCTION_SWEEP.open(operationType, closedSweep);
  }
}

void closeSourceDCCallback(int set_or_limit, bool cancel) {
  Serial.print("set or limit:");
  Serial.println(set_or_limit);
  Serial.flush();
  if (cancel) {
    return;
  }
  GD.__end();
  disable_ADC_DAC_SPI_units();
  if (set_or_limit == SET) {
    float mv = SOURCE_DIAL.getMv(); // TODO: get current value from another place ?
    if (operationType == SOURCE_VOLTAGE) {
       if (SMU[0].fltSetCommitVoltageSource(mv, true)) printError(_PRINT_ERROR_VOLTAGE_SOURCE_SETTING);
    } else {

      // auto current range when sourcing current
      if (abs(mv) < 8.0) {  // TODO: move to 10.0 when DAC has full range (5v ref instead of 4.096)
        current_range = MILLIAMP10;
        SMU[0].setCurrentRange(current_range);
      } else {
        current_range = AMP1;
        SMU[0].setCurrentRange(current_range);
      }
      if (SMU[0].fltSetCommitCurrentSource(mv)) printError(_PRINT_ERROR_VOLTAGE_SOURCE_SETTING);
    }
  }
  if (set_or_limit == LIMIT) {
    Serial.println(LIMIT_DIAL.getMv());
    Serial.flush();
     if (SMU[0].fltSetCommitLimit(LIMIT_DIAL.getMv() / 1000.0, _SOURCE_AND_SINK)) printError(_PRINT_ERROR_CURRENT_SOURCE_SETTING);
  }
  GD.resume();
}

void printError(int16_t  errorNum)
{
  Serial.print(F("Error("));
  Serial.print(errorNum);
  Serial.println(")");
}
