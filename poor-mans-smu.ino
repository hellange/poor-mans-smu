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
#include "operations.h"
#include "Arduino.h"
#include "Wire.h"
#include "Mainmenu.h"
#include "SMU_HAL_dummy.h"
#include "SMU_HAL_717x.h"
#include "dial.h"
#include "FunctionPulse.h"

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

//uncomment below if you want to use real AD/DA 
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

int noOfWidgets = 6;
int activeWidget = 0;

CURRENT_RANGE current_range = AMP1; // TODO: get rid of global
int timeSinceLastChange = 0;  // TODO: get rid of global

float MAX_CURRENT_10mA_RANGE = 5.0; // current values set because the ADC limit is 6 volt now...
float MAX_CURRENT_1A_RANGE = 500.0;


OPERATION_TYPE operationType = SOURCE_VOLTAGE;


int functionType = SOURCE_DC;

OPERATION_TYPE getOperationType() {
  if (digitalRead(3) == HIGH) {
    return SOURCE_VOLTAGE;
  } else {
    return SOURCE_CURRENT;
  }
}


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

    pinMode(4,OUTPUT); // current range io pin for switching on/off 100ohm shunt
    digitalWrite(4, HIGH);

    pinMode(3,INPUT); // current range selector

    
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

   operationType = getOperationType();

   if (operationType == SOURCE_VOLTAGE) {
     SMU[0].fltSetCommitVoltageSource(0.0);
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
   
   V_FILTERS.init();
   C_FILTERS.init();
   
   V_CALIBRATION.init();
   C_CALIBRATION.init();

   SOURCE_DIAL.init();
   LIMIT_DIAL.init();

   FUNCTION_PULSE.init(SMU[0]);

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

void sourcePulsePanel(int x, int y) {
  FUNCTION_PULSE.render(x,y);
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


void renderUpperDisplay(int operationType, int functionType) {

  int x = 0;
  int y = 2;
 
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
  if (functionType == SOURCE_DC) {
    if (operationType == SOURCE_VOLTAGE) {
        sourceVoltagePanel(x,y);
    } else {
        sourceCurrentPanel(x,y);
    }
    renderStatusIndicators(x,y);
   
    if(V_STATS.rawValue > 10.0) {
      GD.ColorRGB(0xdddddd);
      GD.cmd_number(600, 0, 27, 6, (int)(V_FILTERS.mean / C_FILTERS.mean));
      GD.cmd_text(670, 0,  27, 0, "ohm load");
    }
   
  } else if (functionType == SOURCE_PULSE) {
    sourcePulsePanel(x,y);
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
    current_range = MILLIAMP10;
    SMU[0].setCurrentRange(current_range);
    if (operationType == SOURCE_VOLTAGE) {
      SMU[0].fltSetCommitVoltageSource(0.0);
    } else {
      SMU[0].fltSetCommitCurrentSource(0.0);
    }
  }
  
  if (!startupCalibrationDone1 && timeAtStartup + timeBeforeAutoNull + 1000 < millis()) {
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

  if (!startupCalibrationDone2 && timeAtStartup + timeBeforeAutoNull + 1100 < millis()) {
    current_range = AMP1;
    SMU[0].setCurrentRange(current_range);
    if (operationType == SOURCE_VOLTAGE) {
      SMU[0].fltSetCommitVoltageSource(0.0);
    } else {
      SMU[0].fltSetCommitCurrentSource(0.0);
    }
  }
  if (!startupCalibrationDone2 && timeAtStartup + timeBeforeAutoNull + 2000 < millis()) {
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

void notification(char *text) {
   GD.Begin(RECTS);
    GD.ColorA(230);  // already opaque, why ?
    GD.ColorRGB(0x222222);
    GD.Vertex2ii(180, 160);
    GD.Vertex2ii(620, 280);
    GD.ColorRGB(0xffffff);
    GD.cmd_text(250, 200 , 29, 0, text);
}



 
void loop()
{
  handleAutoNullAtStartup();
  operationType = getOperationType();

  // TODO: Moved pulse/sweep out as separate operationType
  if (startupCalibrationDone2) {
    //SMU[0].pulse(-4000.0, 4000.0, 5000);
    //SMU[0].sweep(5.00, -5.00, 0.1, 5000);
  }
//    if (startupCalibrationDone1 && startupCalibrationDone2) {
//      float milliAmpere = C_STATS.rawValue;
//      Serial.print(milliAmpere,5);
//      Serial.print("mA, current range:");
//      Serial.println(current_range);

//      // auto current range switch. TODO: Move to hardware ? Note that range switch also requires change in limit
//      float hysteresis = 0.5;
//      float switchAt = MAX_CURRENT_10mA_RANGE;
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
//    }
  
  GD.__end();

  int dataR = SMU[0].dataReady();
      //Serial.print("DataReady:");  
      //Serial.println(dataR, HEX);  

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
    V_FILTERS.updateMean(Vout, false);
  }
  disable_ADC_DAC_SPI_units();
  GD.resume();

  if (!gestureDetected) {
    int tag = GD.inputs.tag;

    if (tag == BUTTON_SOURCE_SET) {
      Serial.println("Source set");
      SOURCE_DIAL.open(operationType, SET,  closeDialCallback, SMU[0].getSetValuemV());
    } else if (tag == BUTTON_LIM_SET) {
      Serial.println("Limit set");
      LIMIT_DIAL.open(operationType, LIMIT, closeDialCallback, SMU[0].getLimitValue());
    } else if (tag == BUTTON_NULL) {
      Serial.println("Null set");
      V_CALIBRATION.toggleNullValue(V_STATS.rawValue, current_range);
      C_CALIBRATION.toggleNullValue(C_STATS.rawValue, current_range);
    } else if (tag == BUTTON_UNCAL) {
      Serial.println("Uncal set");
      V_CALIBRATION.toggleCalibratedValues();
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

      }
    } 
    // TODO: don't need to check buttons for inactive menus or functions...
    MAINMENU.handleButtonAction(tag);
    FUNCTION_PULSE.handleButtonAction(tag);
    
    
  }

  GD.Clear();
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

  if (!startupCalibrationDone2) {
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


void closeMainMenuCallback(int functionType_) {
  Serial.println("Closed main menu callback");
  Serial.flush();
  functionType = functionType_;
}
void closeDialCallback(int vol_cur_type, int set_or_limit, bool cancel) {
  Serial.print("vol or cur:");
  Serial.println(vol_cur_type);
  Serial.print("set or limit:");
  Serial.println(set_or_limit);
  Serial.flush();
  if (cancel) {
    return;
  }
  GD.__end();
  disable_ADC_DAC_SPI_units();
  if (set_or_limit == SET) {
    float mv = SOURCE_DIAL.getMv();
    if (operationType == SOURCE_VOLTAGE) {
       if (SMU[0].fltSetCommitVoltageSource(mv)) printError(_PRINT_ERROR_VOLTAGE_SOURCE_SETTING);
    } else {
      // auto current range when sourcing current
      if (mv < 3.0) {
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
