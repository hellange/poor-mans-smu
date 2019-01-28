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
#include "Filters.h"
#include "digit_util.h"
#include "tags.h"

#include "Arduino.h"
#include "Wire.h"
#include "SMU_HAL_dummy.h"

#include "Dac.h"


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
 
SMU_HAL_dummy SMU[1] = {
  SMU_HAL_dummy()
};



int scroll = 0;
int scrollDir = 0;
int scrollMainMenu = 0;
int scrollMainMenuDir = 0;

StatsClass V_STATS;
StatsClass C_STATS;
FiltersClass V_FILTERS;

float rawMa_glob; // TODO: store in stats for analysis just as voltage

float DACVout;  // TODO: Dont use global

float setMv;
float setMa;

int noOfWidgets = 4;


#include "dial.h"

void setup()
{
  Serial.begin(9600);
  Serial.println("Initializing DAC...");
  disableSPIunits();
  DAC.init();
  Serial.println("Done!");

  Serial.println("Initializing WeatherNG graphics controller FT81x...");
  disableSPIunits();
  GD.begin(0);
  GD.cmd_romfont(1, 34); // put FT81x font 34 in slot 1
  Serial.println("Done!");
    
  setMv = 0.0;
  setMa = 10.0;
  SMU[0].fltSetCommitCurrentSource(setMa / 1000.0, _SOURCE_AND_SINK); 
  SMU[0].fltSetCommitVoltageSource(setMv / 1000.0);
  
  V_STATS.init(DigitUtilClass::typeVoltage);
  C_STATS.init(DigitUtilClass::typeCurrent);
  V_FILTERS.init();
}

void disableSPIunits(){
  pinMode(9, OUTPUT);
  digitalWrite(9, HIGH);
  pinMode(10, OUTPUT);
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
  GD.cmd_text(x+20, y + 2 ,   29, 0, "SOURCE VOLTAGE");

  // primary
  //VOLT_DISPLAY.renderMeasured(x + 17,y + 26 , V_STATS.rawValue);
  VOLT_DISPLAY.renderMeasured(x + 17,y + 26, V_FILTERS.mean);

  // secondary
  GD.ColorRGB(COLOR_VOLT_LIGHT);
  DIGIT_UTIL.renderValue(x + 320,  y-4 , V_STATS.rawValue, 4, DigitUtilClass::typeVoltage); 

  GD.ColorRGB(COLOR_VOLT);
  VOLT_DISPLAY.renderSet(x + 120, y + 26 + 105, setMv);

  GD.ColorRGB(0,0,0);

  GD.cmd_fgcolor(0xaaaa90);  
  GD.Tag(BUTTON_VOLT_SET);
  GD.cmd_button(x + 20,y + 132,95,50,30,0,"SET");
  GD.Tag(BUTTON_VOLT_AUTO);
  GD.cmd_button(x + 350,y + 132,95,50,30,0,"AUTO");
  
  renderDeviation(x + 667,y + 130, V_STATS.rawValue, setMv, false);

  showStatusIndicator(x+630, y+5, "FILTER", true, false);
  showStatusIndicator(x+720, y+5, "NULL", false, false);
  showStatusIndicator(x+630, y+45, "50Hz", false, false);
  showStatusIndicator(x+720, y+45, "4 1/2", false, false);
  showStatusIndicator(x+630, y+85, "COMP", true, true);
}

void renderDeviation(int x, int y, float rawM, float setM, bool cur) {
  if (cur) {
     //Special handling: set current must currently be positive even if sink/negative.
     //                  This give error when comparing negative measured and positive set.
     //                  Use absolute values to give "correct" comparision...
     setM = abs(setM);
     rawM = abs(rawM);
  }
  float devPercent = abs(100.0 * ((setM - rawM) / setM));

  GD.ColorRGB(200,255,200);
  GD.cmd_text(x, y, 27, 0, "Deviation");
  if (cur) {
    GD.ColorRGB(COLOR_CURRENT);
  } else {
    GD.ColorRGB(COLOR_VOLT);
  }
  
  y=y+16;
  
  if (setM != 0.0) {
    if (devPercent < 1.0) {
      GD.cmd_text(x, y, 30, 0, "0.");
      GD.cmd_number(x+30, y, 30, 3, devPercent * 1000.0);
      GD.cmd_text(x+30+50, y, 30, 0, "%");
    } else if (devPercent >= 10.0){
      if (!cur) {
        GD.ColorRGB(255,0,0); // RED
      }
      GD.cmd_text(x, y, 30, 0, ">=10%");
    } else if (devPercent >= 1.0 && devPercent <10.0){
      int whole = (int)devPercent;
      //GD.cmd_text(x, y, 30, 0, "0.");
      GD.cmd_number(x, y, 30, 1, whole );
      GD.cmd_text(x+20, y, 30, 0, ".");
      GD.cmd_number(x+30, y, 30, 2, (devPercent - (float)whole) * 100.0);
      GD.cmd_text(x+65, y, 30, 0, "%");
    }
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
  GD.Begin(LINE_STRIP);
  GD.LineWidth(32);
  GD.ColorRGB(50,50,0);
 
  GD.ColorRGB(0xbbbbbb);

  CURRENT_DISPLAY.renderMeasured(x + 17, y+30, C_STATS.rawValue, overflow);
  CURRENT_DISPLAY.renderSet(x+120, y+135, setMa);
  
  GD.ColorRGB(0,0,0);

  GD.cmd_fgcolor(0xaaaa90);  
  GD.Tag(BUTTON_CUR_SET);
  GD.cmd_button(x+20,y+135,95,50,30,0,"LIM");
  GD.Tag(BUTTON_CUR_AUTO);
  GD.cmd_button(x+350,y+135,95,50,30,0,"AUTO");
  
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

void widgetHeaderTab(int y, int activeWidget) {
  y=y+10;
  GD.Begin(RECTS);

  GD.ColorA(255);
  GD.LineWidth(350);
  //GD.ColorRGB(0x444477);
  GD.ColorRGB(0x333350);
  GD.Vertex2ii(30,y);
  GD.Vertex2ii(770, 480);

  GD.Begin(RECTS);
  GD.ColorA(255);
  GD.LineWidth(10);
  GD.ColorRGB(0x000000);
  GD.Vertex2ii(0,y+15);
  GD.Vertex2ii(800, 480);

  y=y-5;
  int x = 400 - 30 * noOfWidgets/2;
  for (int i = 0; i < noOfWidgets; i++) {
    drawBall(x+ i*30,y,activeWidget == i);
  }
}


void showWidget(int y, int widgetNo, int scroll) {
  int yPos = y;
  if (widgetNo ==0) {
     if (scroll ==0){
       GD.ColorRGB(0xbbbbbb);
       GD.cmd_text(20, yPos+2, 29, 0, "MEASURE CURRENT");
     }
     currentPanel(scroll, yPos, SMU[0].Overflow());
  } else if (widgetNo == 1) {
    if (!DIAL.isDialogOpen()){
       if (scroll ==0){
         GD.ColorRGB(0xbbbbbb);
         GD.cmd_text(20, yPos+2, 29, 0, "CURRENT TREND");
       }
       renderCurrentGraph(scroll, yPos, scrollDir != 0);
    }
  } else if (widgetNo == 2) {
      if (!DIAL.isDialogOpen()){
        if (scroll ==0){
          GD.ColorRGB(0xbbbbbb);
          GD.cmd_text(20, yPos+2, 29, 0, "VOLTAGE TREND");
        }
        renderVoltageGraph(scroll, yPos, scrollDir != 0);
      }
  } else if (widgetNo == 3) {
      if (scroll ==0){
        GD.ColorRGB(0xbbbbbb);
        GD.cmd_text(20, yPos+2, 29, 0, "VOLTAGE HISTOGRAM");
      }
      renderHistogram(scroll, yPos, scrollDir !=0);
  }

}


int gestureDetected = GEST_NONE;
int activeWidget = 0;
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

boolean mainMenuActive = false;
void handleMenuScrolldown(){

  if (gestureDetected == GEST_MOVE_DOWN && mainMenuActive == false) {
    mainMenuActive = true;
    scrollMainMenuDir = 1;
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
    GD.ColorRGB(0xffffff);
    GD.Vertex2ii(50,0);
    GD.Vertex2ii(750, scrollMainMenu+40);

    GD.Begin(RECTS);
    GD.LineWidth(180);
    GD.ColorA(200);
    GD.ColorRGB(0x000000);
    GD.Vertex2ii(50,0);
    GD.Vertex2ii(750, scrollMainMenu+40);
    GD.ColorRGB(0xffffff);

    GD.ColorA(255);

    GD.Tag(MAIN_MENU_CLOSE);
    GD.cmd_button(360,scrollMainMenu-20,80,50,28,0,"CLOSE");
   
    GD.get_inputs();

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
  int y = 25;


  // register screen for gestures on top half
  GD.Tag(GESTURE_AREA_HIGH);
  GD.Begin(RECTS);
  GD.ColorA(200);
  GD.ColorRGB(0x000000);
  GD.Vertex2ii(0,0);
  GD.Vertex2ii(800, LOWER_WIDGET_Y_POS);
  voltagePanel(x,y);

  // register screen for gestures on lower half
  GD.Tag(GESTURE_AREA_LOW);
  GD.Begin(RECTS);
  //GD.ColorA(200);
  GD.ColorRGB(0x000000);
  GD.Vertex2ii(0,LOWER_WIDGET_Y_POS);
  GD.Vertex2ii(800, 480);
  
  handleWidgetScrollPosition();

  widgetHeaderTab(260, activeWidget);

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
    if (touchX > 0 && touchY > LOWER_WIDGET_Y_POS && gestDistanceX < -10 && scrollDir == 0) {
      if (++gestDurationX >= 2) {
        Serial.println("move left");
        Serial.flush();
        gestureDetected = GEST_MOVE_LEFT;
        gestDurationX = 0;
      }
    }
    else if (touchX > 0 && touchY > LOWER_WIDGET_Y_POS && gestDistanceX > 10 && scrollDir == 0) {
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
  if (millis() > startupMillis + 2000) {
    return true;
  } else {
    return false;
  }
}

unsigned long previousMillis = 0; 
unsigned long previousMillisSlow = 0; 
const long interval = 1; // should it account for time taken to perform ADC sample ?
float Vout = 0.0;
void loop()
{
  GD.__end();
  disableSPIunits();

  SPI.beginTransaction (SPISettings (1000000, MSBFIRST, SPI_MODE1));
  digitalWrite(10, LOW);
  if(DAC.dataReady() == true) {
    Vout = DAC.MeasureVoltage();
    Serial.print("Measured raw:");  
    Serial.print(Vout, 3);
    Serial.println(" mV");  
    Serial.flush();
  }
  disableSPIunits();

  GD.resume();
  SPI.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE0));
  if (!gestureDetected) {
    if (GD.inputs.tag == BUTTON_VOLT_SET) {
      DIAL.open(BUTTON_VOLT_SET, closeCallback);
    } else if (GD.inputs.tag == BUTTON_CUR_SET) {
      DIAL.open(BUTTON_CUR_SET, closeCallback);
    }
  }

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    GD.get_inputs();
    detectGestures();

    GD.Clear();
    if (readyToDoStableMeasurements()) {
      // Dont sample voltage and current while scrolling because polling is slow.
      // TODO: Remove this limitation when sampling is based on interrupts.
      //if (scrollDir == 0) {
         //V_STATS.addSample(SMU[0].MeasureVoltage() * 1000.0);
         V_STATS.addSample(Vout);
         V_FILTERS.updateMean(Vout);
         C_STATS.addSample(SMU[0].MeasureCurrent() * 1000.0);
      //}
    }
    renderDisplay();
    
    if (DIAL.isDialogOpen()) {
      bluredBackground();
      DIAL.checkKeypress();
      DIAL.handleKeypadDialog();
    }

    GD.swap();    
    GD.__end();
  }
}



void closeCallback(int vol_cur_type, bool cancel) {
   Serial.print("SET type:");
   Serial.println(vol_cur_type);
   Serial.println(DIAL.type() );
  if (cancel) {
    return;
  }
  if (DIAL.type() == BUTTON_VOLT_SET) {
     if (SMU[0].fltSetCommitVoltageSource(DIAL.getMv() / 1000.0)) printError(_PRINT_ERROR_VOLTAGE_SOURCE_SETTING);
     setMv = DIAL.getMv();
  }
  if (DIAL.type() == BUTTON_CUR_SET) {
     if (SMU[0].fltSetCommitCurrentSource(DIAL.getMv() / 1000.0, _SOURCE_AND_SINK)) printError(_PRINT_ERROR_CURRENT_SOURCE_SETTING);
     setMa = DIAL.getMv();
  }
     
}




void printError(int16_t  errorNum)
{
  Serial.print(F("Error("));
  Serial.print(errorNum);
  Serial.println(")");
}







