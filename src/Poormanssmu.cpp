/****************************************************************
 *  
 *  Initial prototype for
 *  P O O R  M A N ' s  S M U
 *  
 *  by Helge Langehaug (2018 - 2023)
 * 
 * 
 * Ref also:
 *   https://poormanssmu.wordpress.com/
 *   https://poormanssmu.wordpress.com/2020/06/05/first-prototype/
 *****************************************************************/

//Uncomment the line below if you don't have the analog hardware, only processor and screen...
//The analog circuitry will then be simulated.
//Don't exect everything to work... It's not tested very often...
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
#include "RotaryEncoder.h"
#include "PushButtons.h"
#include "TrendGraph.h"
#include "Logger.h"

#include "Settings.h"
#include "SimpleStats.h"

#include "utils.h"
#include "analogGauge.h"
#ifndef ARDUINO_TEENSY31
#include "Network2.h"
#endif
#include "Digitizer.h"
#include "ZeroCalibration.h"

#include "Ada4254.h"
#include "Vrekrer_scpi_parser.h" // Ref: https://github.com/Vrekrer/Vrekrer_scpi_parser
#include "Debug.h"
#include "Gest.h"
#include "Widgets.h"
#include "SMU_HAL.H"


#define _PRINT_ERROR_VOLTAGE_SOURCE_SETTING 0
#define _PRINT_ERROR_CURRENT_SOURCE_SETTING 1
#define _PRINT_ERROR_SERIAL_COMMAND 2
#define _PRINT_ERROR_NO_CALIBRATION_DATA_FOUND 3
#define _PRINT_ERROR_FACTORY_CALIBRATION_RESTORE_FAILED_CATASTROPHICALLY


#define LOWER_WIDGET_Y_POS 250

#define SAMPLING_BY_INTERRUPT

#define VERSION_NUMBER "0.2.81"

SimpleStatsClass SIMPLE_STATS;
LoggerClass LOGGER;

PushbuttonsClass PUSHBUTTONS;
PushbuttonsClass PUSHBUTTON_ENC;

ZeroCalibrationlass ZEROCALIBRATION;

const char compile_date[] = __DATE__ " " __TIME__ ", (sketch:" __FILE__ ")";

bool anyDialogOpen();
void openMainMenu();
//void renderAnalogGauge(int x, int y, int size, float degrees, float value, const char *title);
void loopMain();
void loopDigitize();
static void handleSampling();
void closeMainMenuCallback(FUNCTION_TYPE functionType_);
void showFanSpeed(int x, int y);
void fltCommitCurrentSourceAutoRange(float mv, bool autoRange);
void closeSourceDCCallback(int set_or_limit, bool cancel);
void scpi_setup();
void useVoltageFeedback();
void useCurrentFeedback();


#ifndef USE_SIMULATOR
  ADCClass SMU[1] = {
    ADCClass()
  };
#else
  SMU_HAL_dummy SMU[1] = {
    SMU_HAL_dummy()
  };
#endif

char SCPI_CommandBuffer[100];

unsigned long timeAtStartup;

// Used to make sure buttons are not triggered multiple times by a single touch
uint32_t timeSinceLastChange = 0;  // TODO: get rid of global

OPERATION_TYPE operationType = SOURCE_VOLTAGE;
FUNCTION_TYPE functionType = SOURCE_DC_VOLTAGE;

void printError(int16_t  errorNum)
{
  DEBUG.printError(errorNum);
}

OPERATION_TYPE getOperationType() {
  OPERATION_TYPE ot;
  //TODO:  This is messy! Based on older concept where function and operation were different stuff... this has slightly changed....
  if (functionType == SOURCE_DC_CURRENT) {
    ot = SOURCE_CURRENT;
  } else {
    ot = SOURCE_VOLTAGE;
  }
  return ot;
}
IntervalTimer normalSamplingTimer;

float changeDigit = 0; 
uint32_t changeDigitTimeout;

void rotaryChangedVoltCurrentFn(float changeVal) {
  changeDigitTimeout = millis();

    // changeDigit will be 0 if rotary know has not been pushed to select decade.
    // If so and dialog is not open, then ignore rotation !
    if (changeDigit == 0 && !SOURCE_DIAL.isDialogOpen()) {
       DEBUG.println("change digit not set. Ignoring rotary input.");
       return;
    }

    // Dialog is open. Adhere to rotary knob independent on changeDigit
    if (operationType == SOURCE_VOLTAGE) {
      DEBUG.print("rotary changeval:");
      DEBUG.print(changeVal);
      //DEBUG.print(",rotary changedigit:");
      //DEBUG.print(changeDigit);

      DEBUG.println();
      if(LIMIT_DIAL.isDialogOpen()) {
        DEBUG.println("Rotaty change not implemented for current limit yet");
      } else if(SOURCE_DIAL.isDialogOpen()) {
        //TODO: Fix problem with resolution and selected digit to change !!!
        float mv = SOURCE_DIAL.getMv();
        int64_t change_uV =  changeVal*1000;
        int64_t new_uV = mv*1000 + change_uV;
         SOURCE_DIAL.setMv(new_uV /1000.0);
      } else {
        DEBUG.print("from smu setvalue mv=");
        DIGIT_UTIL.print_uint64_t(SMU[0].getSetValue_micro());
        DEBUG.println();

        int64_t change_uV =  changeVal*changeDigit;
        DEBUG.print("change value in uV=");
        DIGIT_UTIL.print_uint64_t(change_uV);
        DEBUG.println();

        int64_t new_uV = SMU[0].getSetValue_micro() + change_uV;
        DEBUG.print("new uV=");
        DIGIT_UTIL.print_uint64_t(new_uV);
        DEBUG.println();
        /*
        float newVoltage_mV = new_uV / 1000.0;
        DEBUG.print("new mv=");
        DEBUG.println(newVoltage_mV,5);
        */
        if (SMU[0].fltSetCommitVoltageSource(new_uV, true)) printError(_PRINT_ERROR_VOLTAGE_SOURCE_SETTING);
      }
    } else {
      // in source current
      if (SMU[0].getCurrentRange() == MILLIAMP10) {
        changeVal = changeVal / 100.0;
      }
      if(LIMIT_DIAL.isDialogOpen()) {
        DEBUG.println("Rotaty change not implemented for voltage limit yet");
      } else if(SOURCE_DIAL.isDialogOpen()) {
        float mv = SOURCE_DIAL.getMv();
        int64_t change_uV =  changeVal*1000;
        int64_t new_uV = mv*1000 + change_uV;
        SOURCE_DIAL.setMv(new_uV /1000.0);
      } else {
        DEBUG.print("from smu setvalue mv=");
        DIGIT_UTIL.print_uint64_t(SMU[0].getSetValue_micro());
        DEBUG.println();

        int64_t change_uV =  changeVal*changeDigit; 
        DEBUG.print("change value in uV=");
        DIGIT_UTIL.print_uint64_t(change_uV);
        DEBUG.println();

        int64_t new_uV = SMU[0].getSetValue_micro() + change_uV;
        DEBUG.print("new uV=");
        DIGIT_UTIL.print_uint64_t(new_uV);
        DEBUG.println();
        fltCommitCurrentSourceAutoRange(new_uV, false);
      }
   }

}

bool showSettings = false;

void pushButtonEncInterrupt(int key, bool quickPress, bool holdAfterLongPress, bool releaseAfterLongPress) {
  DEBUG.print("Key pressed:");
  DEBUG.print(key);
  DEBUG.print(" ");
  DEBUG.println(quickPress==true?"QUICK" : "");
  DEBUG.println(holdAfterLongPress==true?"HOLDING" : "");
  DEBUG.println(releaseAfterLongPress==true?"RELEASED AFTER HOLDING" : "");
  changeDigitTimeout = millis();
  ROTARY_ENCODER.stepless_dynamic = true;
  if (changeDigit == 0) {
    changeDigit = 1000000;
    ROTARY_ENCODER.stepless_dynamic = false;
  } else if (changeDigit == 10000000){
    changeDigit = 1000000;
    ROTARY_ENCODER.stepless_dynamic = false;
  } else if (changeDigit == 1000000){
    changeDigit = 100000;
  }  else if (changeDigit == 100000){
    changeDigit = 10000;
  }  else if (changeDigit == 10000){
    changeDigit = 1000;
  }  else if (changeDigit == 1000){
    changeDigit = 0;
  }

  // For sourcing current. hardcode to least digit
  // until marking works in current display
  // TODO: Remove this override !!!!
  if (operationType == SOURCE_CURRENT || SMU[0].getCurrentRange() == MILLIAMP10) {
    //changeDigit / 10000; 
  }


  DEBUG.print("changeDigit:");
  DEBUG.println(changeDigit);


}


void pushButtonInterrupt(int key, bool quickPress, bool holdAfterLongPress, bool releaseAfterLongPress) {
  DEBUG.print("Key pressed:");
  DEBUG.print(key);
  DEBUG.print(" ");
  DEBUG.println(quickPress==true?"QUICK" : "");
  DEBUG.println(holdAfterLongPress==true?"HOLDING" : "");
  DEBUG.println(releaseAfterLongPress==true?"RELEASED AFTER HOLDING" : "");

  if (quickPress && key ==1 && MAINMENU.active == false) {
    openMainMenu();
  }

  else if (quickPress && key ==4 && showSettings == false) {
    showSettings = true;
  } else if (quickPress && key ==4 && showSettings == true) {
    showSettings = false;
  }

}

bool reduceDetails() {
  return WIDGETS.isScrolling() || MAINMENU.active == true || anyDialogOpen();
}

void  disable_ADC_DAC_SPI_units() {
  SMU[0].disable_ADC_DAC_SPI_units();
}


void initDefaultSamplingIfByInterrupt() {
#ifdef SAMPLING_BY_INTERRUPT
  normalSamplingTimer.begin(handleSampling, 10); // in microseconds.Too low value gives problems... 
  SPI.usingInterrupt(normalSamplingTimer);
#endif
}


void setup()
{
  WIDGETS.init(SMU[0]);
  ZEROCALIBRATION.init(SMU[0]);

	//while (!Serial) ; // wait
  //delay(5000);
  LOGGER.init();

  SIMPLE_STATS.init();
  
   disable_ADC_DAC_SPI_units();
   delay(50);
   //TODO: Organise pin numbers into definitions ?
   pinMode(6,OUTPUT); // LCD powerdown pin?
   //digitalWrite(6, LOW);
   pinMode(5,OUTPUT); // RAM

    pinMode(7,OUTPUT);
    pinMode(8,OUTPUT);
    pinMode(9,OUTPUT);
    pinMode(10,OUTPUT);

    pinMode(4,OUTPUT); // current range io pin for switching on/off 100ohm shunt
    digitalWrite(4, LOW);


    // Changed in later revisions to be inpur for limits
    // TODO: Fix this mess
    pinMode(3,INPUT); // (NO!) current range selector
    pinMode(2,INPUT); // (NO!) Fan speed feedback


    PUSHBUTTONS.init(3, 1000); // bushbuttons based on analog pin 3, and holding 1000ms
    PUSHBUTTONS.setCallback(pushButtonInterrupt);
    PUSHBUTTON_ENC.init(16, 1000); 
    PUSHBUTTON_ENC.setCallback(pushButtonEncInterrupt);
    
    FAN.init();
    
    //pinMode(11,OUTPUT);
    //pinMode(12,INPUT);
    //pinMode(13,OUTPUT);

    DEBUG.println("Initializing graphics controller FT81x...");
    DEBUG.flush();

    // bootup FT8xx
    // Drive the PD_N pin high
    // brief reset on the LCD clear pin
    //TODO: Does not work yet.... why ?  Need to reconnect USB (power cycle)...
    for (int i=0;i<2;i++) {
    digitalWrite(6, LOW);
    delay(200);
    digitalWrite(6, HIGH);
    delay(200);
    }

    DEBUG.println("Build info");
    DEBUG.println(compile_date);

    DEBUG.flush();
   GD.begin(0);
   
   delay(100);

   DEBUG.println("Initializing graphics...");
   DEBUG.flush();
   GD.cmd_romfont(1, 34); // put FT81x font 34 in slot 1
   GD.Clear();
   GD.ColorRGB(0xaaaaff);
   GD.ColorA(200);
   GD.cmd_text(250, 200 ,   31, 0, "Poor man's SMU");
   GD.ColorRGB(0xaaaaaa);
   GD.cmd_text(250, 245 ,   28, 0, "Designed    by    Helge Langehaug");
   GD.cmd_text(250, 270 ,   28, 1, "Version:");
   GD.cmd_text(340, 270 ,   28, 1, VERSION_NUMBER);

   GD.cmd_text(0, 430 ,   27, 1, "Build date:");
   GD.cmd_text(0, 450 ,   27, 1, compile_date);



   GD.swap();


   //delay(501);

   GD.__end();
   DEBUG.println("Graphics initialized.");
   DEBUG.flush();

      delay(2000); // Delay a bit to show splash screen...


   // TODO: Fix this experimental mess !!!
   DEBUG.println("====== ADA4254 =====");
   ADA4254.reset();
   ADA4254.printId();
   ADA4254.ada4254_4();
   delay(100);
   ADA4254.ada4254_5_gain();
   delay(100);
   //ADA4254.ada4254_clear_analog_error();
   delay(100);
   ADA4254.ada4254_clear_analog_error();
   //ADA4254.ada4254_5_gainx1d25();
   DEBUG.println("====== ADA4254 done =====");

   disable_ADC_DAC_SPI_units();
   delay(100);
   DEBUG.println("Initializing SMU...");
   SMU[0].init();
   SETTINGS.init(SMU[0]);

   SMU[0].setSamplingRate(20);
   operationType = getOperationType();

   V_CALIBRATION.init(SMU[0], SOURCE_VOLTAGE);
   C_CALIBRATION.init(SMU[0], SOURCE_CURRENT);

   V_CALIBRATION.printNonlinearValues();
   C_CALIBRATION.printNonlinearValues();

   DEBUG.println("SMU initialized");
   DEBUG.println("");
   DEBUG.print("Found DAC type:");
   // based on datasheet...
   if ((SMU[0].deviceTypeId & 0xfff0) == 0x0C90) { //0x0C94 ?
     DEBUG.print("AD7176-2");
   } else if ((SMU[0].deviceTypeId & 0xfff0) == 0x00D0) { 
     DEBUG.print("AD7172-2"); 
   } else if ((SMU[0].deviceTypeId & 0xfff0) == 0x0CD0) {
     DEBUG.print("AD7175-2"); //0x0CDX in datasheet
   } else if ((SMU[0].deviceTypeId & 0xfff0) == 0x4FD0) {
     DEBUG.print("AD7177-2");
   } else if (SMU[0].deviceTypeId == SMU_HAL_DUMMY_DEVICE_TYPE_ID) {
     DEBUG.print("Sim");
   } else {
     DEBUG.print("Unknown");
   }

     DEBUG.print("    hex:");
     DEBUG.println(SMU[0].deviceTypeId, HEX);
   
   DEBUG.println("--------");
   DEBUG.println("");

   DEBUG.flush();

   if (operationType == SOURCE_VOLTAGE) {
     SMU[0].fltSetCommitVoltageSource(SETTINGS.setMilliVoltage*1000, true);
     DEBUG.println("Source voltage");
     //current_range = AMP1;
     SMU[0].setCurrentRange(AMP1, operationType);
     SMU[0].fltSetCommitCurrentLimit(SETTINGS.setCurrentLimit*1000); 
   } 
   DEBUG.print("Default source voltage ");
   DEBUG.println(SETTINGS.setMilliVoltage);
   DEBUG.println(" mV");
   DEBUG.print("Default current limit ");
   DEBUG.println(SETTINGS.setCurrentLimit);
   DEBUG.println(" mA");
   DEBUG.flush();

   V_STATS.init(DigitUtilClass::typeVoltage);
   C_STATS.init(DigitUtilClass::typeCurrent);
   
   V_FILTERS.init(1234);
   C_FILTERS.init(5678);
   
   SOURCE_DIAL.init();
   LIMIT_DIAL.init();

// TODO: Resolve problems with static..
   FUNCTION_PULSE.init(/*SMU[0]*/); // TODO: Resolve problems with static...
   // TODO: Resolve problems with static...


   FUNCTION_SWEEP.init(SMU[0]);

   RAM.init();
   RAM.startLog(); // TODO: Start it after things have "warmed up" a bit ?

   timeAtStartup = millis();

   // SPI.usingInterrupt(2);
   // pinMode(2,INPUT);
   // attachInterrupt(2, handleSampling, CHANGE);

   initDefaultSamplingIfByInterrupt();


   ROTARY_ENCODER.init(rotaryChangedVoltCurrentFn);
 
   //TC74 
   Wire.begin();

   DIGITIZER.init(SMU[0], getOperationType());

   scpi_setup();

#ifndef ARDUINO_TEENSY31
   ETHERNET2_UTIL.setup(); 
  //  DEBUG.print("Ethernet initialization ended ");
#endif
} 

void markSetDigitCur(int x, int y) {
  if (changeDigit != 0) {

    GD.Begin(LINE_STRIP);
    if ( ((millis() - changeDigitTimeout) /750) & 1) {
      GD.ColorA(150);
    } else {
      GD.ColorA(255);
    }
    GD.LineWidth(25);
    GD.ColorRGB(0xff5555);
    int length = 20;
    int position = 0;

    if (SMU[0].getCurrentRange() == MILLIAMP10) {
      if (changeDigit == 1000) {
        position = 130;
      } else if (changeDigit == 10000) {
        position = 105;
      } else if (changeDigit == 100000) {
        position = 82;
      } else if (changeDigit == 1000000) {
        position = 52;
      } 

    } else {
      if (changeDigit == 1000) {
        position = 142;
      } else if (changeDigit == 10000) {
        position = 111;
      } else if (changeDigit == 100000) {
        position = 87;
      } else if (changeDigit == 1000000) {
        position = 63;
      } 

    }
 


    GD.Vertex2ii(x + position ,y);
    GD.Vertex2ii(x + position + length ,y);
    GD.ColorA(255);
  }
}


void markSetDigitVol(int x, int y) {
  
  if (changeDigit != 0) {
    GD.Begin(LINE_STRIP);
    if ( ((millis() - changeDigitTimeout) /750) & 1) {
      GD.ColorA(150);
    } else {
      GD.ColorA(255);
    }
    GD.LineWidth(25);
    GD.ColorRGB(0xff5555);
    int length = 20;
    int position = 0;
    if (changeDigit == 1000) {
      position = 167;
    } else if (changeDigit == 10000) {
      position = 134;
    } else if (changeDigit == 100000) {
      position = 112;
    } else if (changeDigit == 1000000) {
      position = 88;
    } else if (changeDigit == 10000000) {
      position = 52;
    } 
    GD.Vertex2ii(x + position ,y);
    GD.Vertex2ii(x + position + length ,y);
    GD.ColorA(255);
  }
}


void showStatusIndicator(int x,int y,const char* text, bool enable, bool warn) {
  if (MAINMENU.active == true) {
    return;
  }
  GD.Begin(RECTS);
  GD.ColorRGB(warn ? 0xFF0000 : COLOR_VOLT);
  GD.ColorA(enable ? 150 : 25);
  
  GD.LineWidth(150);
  GD.Vertex2ii(x, 15 + y);
  GD.Vertex2ii(x + 55, y + 24);
  GD.ColorRGB(0xffffff);
  GD.ColorA(enable ? 200 : 25);
  GD.cmd_text(x+ 2, y + 10, 27, 0, text);
  GD.ColorA(255);
}

void sourcePulsePanel(int x, int y) {
  FUNCTION_PULSE.render(x, y, reduceDetails());
}

void sourceSweepPanel(int x, int y) {
  FUNCTION_SWEEP.render(x, y, MAINMENU.active == true);
}

//int settingsCurrentAutoRange = 0;


void sourceCurrentPanel(int x, int y) {

  // heading
  GD.ColorRGB(COLOR_CURRENT);
  GD.ColorA(120);
  GD.cmd_text(x+20, y + 2 ,   29, 0, "SOURCE CURRENT");
  GD.cmd_text(x+20 + 1, y + 2 + 1 ,   29, 0, "SOURCE CURRENT");
  
  // primary
  bool shownA = (SMU[0].getCurrentRange() == MILLIAMP10);
  shownA = false; // Override. Dont show nA
  
  GD.ColorA(255);
  CURRENT_DISPLAY.renderMeasured(x /*+ 17*/,y + 26, C_FILTERS.mean, false, shownA, SMU[0].getCurrentRange(), reduceDetails());

  // secondary
  GD.ColorRGB(COLOR_CURRENT);
  GD.ColorA(180); // a bit lighter
  DIGIT_UTIL.renderValue(x + 290,  y-4 , C_STATS.rawValue, 4, DigitUtilClass::typeCurrent); 

  GD.ColorA(255);
  if (changeDigit > 0) {
    GD.ColorRGB(0xdddddd);
  } else {
    GD.ColorRGB(COLOR_CURRENT);
  }
  CURRENT_DISPLAY.renderSet(x + 120, y + 131, SMU[0].getSetValue_micro(), SMU[0].getCurrentRange());
  markSetDigitCur(x+120, y+131 + 42);
  GD.ColorRGB(COLOR_CURRENT);

  GD.ColorRGB(0,0,0);
  GD.cmd_fgcolor(0xaaaa90);  
  
  GD.Tag(BUTTON_SOURCE_SET);
  GD.cmd_button(x + 20,y + 132,95,50,29,OPT_NOTEAR,"SET");
  
  GD.Tag(BUTTON_CUR_AUTO);
  GD.cmd_button(x+380,y+132,95,50,29,0,SMU[0].getCurrentRange()==AMP1 ? "1A" : "10mA");
  GD.Tag(0); // Note: Prevents button in some cases to react also when touching other places in UI. Why ?
}



void houseKeeping() {
  // various stuff that shall be checked and updated "in the background"...

  if (changeDigit > 0 && changeDigitTimeout + 5000 < millis()) {
     changeDigit = 0;  // turn off digit change mode after 5 seconds
  }


}

void sourceVoltagePanel(int x, int y) {

  // heading

  GD.ColorRGB(COLOR_VOLT);
  GD.ColorA(120);
  GD.cmd_text(x+20, y + 2 ,   29, 0, "SOURCE VOLTAGE");
  GD.cmd_text(x+20 + 1, y + 2 + 1 ,   29, 0, "SOURCE VOLTAGE");
  
  // primary
  VOLT_DISPLAY.renderMeasured(x /*+ 17*/,y + 26, V_FILTERS.mean, false, reduceDetails());

  if (!reduceDetails()) {
    // secondary
    GD.ColorRGB(COLOR_VOLT);
    GD.ColorA(180); // a bit lighter
    DIGIT_UTIL.renderValue(x + 300,  y-4 , V_STATS.rawValue, 4, DigitUtilClass::typeVoltage); 
  }
  GD.ColorA(255);
  if (changeDigit > 0) {
    GD.ColorRGB(0xdddddd);
  } else {
    GD.ColorRGB(COLOR_VOLT);
  }
  VOLT_DISPLAY.renderSet(x + 120, y + 131, SMU[0].getSetValue_micro());
  markSetDigitVol(x+120, y+131 + 42);
  GD.ColorRGB(COLOR_VOLT);

  float uVstep = 1000000.0/ powf(2.0,18.0); // for 1V 18bit DAC

  if (!reduceDetails()) {
    y=y+5;
    if (abs(SMU[0].DAC_RANGE_LOW)== 5.0 && abs(SMU[0].DAC_RANGE_HIGH) == 5.0) {
      uVstep = uVstep *20.0;
        GD.cmd_text(x+120,y+170, 27, 0, "+/-10V [step:");
        DIGIT_UTIL.renderValue(x + 205,  y+173 , uVstep, 0, -1 /*DigitUtilClass::typeVoltage*/); 
        GD.cmd_text(x+270,y+170, 27, 0, "uV]");
    }
    else if (abs(SMU[0].DAC_RANGE_LOW) == 2.5 && abs(SMU[0].DAC_RANGE_HIGH) == 2.5) {
        uVstep = uVstep *10.0;
        GD.cmd_text(x+120,y+170, 27, 0, "+/- 5V  [step:");
        DIGIT_UTIL.renderValue(x + 205,  y+173 , uVstep, 0, -1/*DigitUtilClass::typeVoltage*/);
        GD.cmd_text(x+270,y+170, 27, 0, "uV]");
    } else if (abs(SMU[0].DAC_RANGE_LOW) == 10.0&& abs(SMU[0].DAC_RANGE_HIGH) == 10.0){
        uVstep = uVstep *40.0;
        GD.cmd_text(x+120,y+170, 27, 0, "+/- 20V [step:");
        DIGIT_UTIL.renderValue(x + 205 + 15,  y+173 , uVstep, 0, -1/*DigitUtilClass::typeVoltage*/); 
        GD.cmd_text(x+270+ 15,y+170, 27, 0, "uV]");
    } else {
         GD.cmd_text(x+120,y+170, 27, 0, " ?");
    }

    y=y-5;
  }
  GD.ColorRGB(0,0,0);
  GD.cmd_fgcolor(0xaaaa90);  

  
/*
  GD.ColorRGB(COLOR_VOLT);
  GD.cmd_text(x+0,y+132+10+5, 28, 0, "SOURCE");
  GD.ColorRGB(0);
  GD.Tag(BUTTON_SOURCE_SET);
  GD.cmd_button(x + 20+100-30-5,y + 132,95+100,50,29,OPT_NOTEAR,""); // SET
    //VOLT_DISPLAY.renderSet(x + 120-100, y + 131, SMU[0].getSetValuemV());
  DIGIT_UTIL.renderValue(x + 120-50+5,  y+131+10-5+3 , SMU[0].getSetValuemV(), 3, DigitUtilClass::typeVoltage); 
  //GD.cmd_text(x+ + 120-50+100+50+30+5,  y+131+10-5+3, 30, 0, "V");
*/

  GD.Tag(BUTTON_SOURCE_SET);
  GD.cmd_button(x + 20, y + 132, 95, 50, 29, OPT_NOTEAR, "SET");

  GD.Tag(BUTTON_VOLT_AUTO);
  GD.cmd_button(x + 350 + 20, y + 132, 95 + 0, 50, 29, 0, "AUTO");

  GD.Tag(0);

  // TODO: Fix. adding details caused some crashes...
  if (1 == 0 && !reduceDetails()) {
    // Add some minor statistics on screen. Nice while looking at long time stability...
    GD.ColorRGB(0xaaaaaa);

    x=x+ 115;
    y=y-5;
    GD.cmd_text(x + 350+20+150-10,y + 132+2, 27, 0, "Min");
    DIGIT_UTIL.renderValue(x + 350+20+170,y + 132, SIMPLE_STATS.minimum, 1, DigitUtilClass::typeVoltage); 

    GD.cmd_text(x + 350+20+150-10,y + 132+ 20+2, 27, 0, "Max");
    DIGIT_UTIL.renderValue(x + 350+20+170,y + 132+ 20, SIMPLE_STATS.maximum, 1, DigitUtilClass::typeVoltage); 

    GD.cmd_text(x + 350+20+150-10,y + 132+ 40+2, 27, 0, "Samples");
    //DIGIT_UTIL.renderValue(x + 350+40+170+20,y + 132+ 40, SIMPLE_STATS.samples / 1000.0, 1, -1); 
    GD.cmd_number(x + 350+40+170+35,y + 132+ 40, 28, 6, SIMPLE_STATS.samples);
  }
  



}

void renderStatusIndicators(int x, int y) {
  x=x+20;
  y=y+10;
  showStatusIndicator(x+630, y+5, "FILTER", V_FILTERS.filterSize>1, false);
  if (operationType == SOURCE_VOLTAGE) {
    showStatusIndicator(x+710, y+5, "NULLv", V_CALIBRATION.nullValueIsSet(SMU[0].getCurrentRange()), false);
    showStatusIndicator(x+710, y+45, "RELv", V_CALIBRATION.relativeValueIsSet(SMU[0].getCurrentRange()), false);

  } else {
    showStatusIndicator(x+710, y+5, "NULLc", C_CALIBRATION.nullValueIsSet(SMU[0].getCurrentRange()), false);
    showStatusIndicator(x+710, y+45, "RELc", C_CALIBRATION.relativeValueIsSet(SMU[0].getCurrentRange()), false);

  }
  showStatusIndicator(x+630, y+45, "50Hz", false, false);
  showStatusIndicator(x+630, y+85, "COMP", SMU[0].hasCompliance(), true);
    if (operationType == SOURCE_VOLTAGE) {
      showStatusIndicator(x+710, y+85, "UNCAL", !V_CALIBRATION.useCalibratedValues, true);
    } else {
      showStatusIndicator(x+710, y+85, "UNCALc", !C_CALIBRATION.useCalibratedValues, true);
    }
  
    showStatusIndicator(x+630, y+85+40, "1AMP", SMU[0].getCurrentRange() == AMP1, false);
    showStatusIndicator(x+710, y+85+40, "10mA", SMU[0].getCurrentRange() == MILLIAMP10, false);
  

}


bool anyDialogOpen() {
  // make sure buttons below the dialog do not reach if finger is not removed from screen
  // when dialog disappears. Use a timer for now...
  return SOURCE_DIAL.isDialogOpen() or LIMIT_DIAL.isDialogOpen();
}

void closeAllOpenDialogs() {
  if ( SOURCE_DIAL.isDialogOpen() ) {
    SOURCE_DIAL.close();
  }
  if (LIMIT_DIAL.isDialogOpen() ) {
    LIMIT_DIAL.close();
  }
}


int gestureDetected = GEST_NONE;
void handleWidgetScrollPosition() {
  WIDGETS.handleWidgetScrollPosition(gestureDetected);
}

void displayWidget() {
  WIDGETS.displayWidget(functionType, operationType, SMU[0].hasCompliance(), reduceDetails(), SMU[0].getSetValue_micro(), SMU[0].getLimitValue_micro(), SMU[0].getCurrentRange(), anyDialogOpen(), LOWER_WIDGET_Y_POS);
}

void bluredBackground() {
    GD.Begin(RECTS);
    GD.ColorA(150);
    GD.ColorRGB(0x000000);
    GD.Vertex2ii(0,0);
    GD.Vertex2ii(800, 480);
}

void openMainMenu() {

   if (anyDialogOpen()) {
      closeAllOpenDialogs();
    }
    MAINMENU.open(closeMainMenuCallback);
}

void handleMenuScrolldown(){

  if (gestureDetected == GEST_MOVE_DOWN && MAINMENU.active == false) {
    openMainMenu();
    return; // start the animation etc. next time, so UI that needs to reduce details have time to reach.
  }  
  
  // main menu
  if (MAINMENU.active) {
    MAINMENU.render();
  }
 
}

//TODO: Investigate this
char* deviceIdToText(int deviceTypeId) {
  //char *a = "fisk\0"

// AD7175-2 is 0x0CDX from datasheet
// AD7176-2 is 0x0C90 ?
// AD7172-2 is 0x00D0 ?
// AD7177-2 is 0x4FD0 ?

  char* ch = new char[8] /* 11 = len of Hello Heap + 1 char for \0*/;

// TODO: Avoid duplicate logic of id to string in this class !
   if ((deviceTypeId & 0xfff0) == 0x0C90) {
     strcpy(ch, "AD7176");
       return ch;

   } else if ((deviceTypeId & 0xfff0) == 0x00D0) {
     strcpy(ch, "AD7172");
       return ch;

   } else if ((deviceTypeId & 0xfff0) == 0x0CD0) {
     strcpy(ch, "AD7175");
       return ch;

   } else if ((deviceTypeId & 0xfff0) == 0x4FD0) {
     strcpy(ch, "AD7177");
       return ch;

   } else if (deviceTypeId == SMU_HAL_DUMMY_DEVICE_TYPE_ID) {
     strcpy(ch, "SIM");
       return ch;

   } 

  itoa(deviceTypeId, ch, 10);

  return ch;
}

void renderMainHeader() {

  if (MAINMENU.active == true or anyDialogOpen()) {
    return;
  }
  // top header
  GD.Begin(RECTS);
  GD.ColorA(255);
  GD.ColorRGB(0x181818);
  GD.Vertex2ii(0,0);
  GD.Vertex2ii(800, 22);
  GD.ColorA(255);

  // TODO: Use other that deviceTypeId to "detect" analog board problem
  if (SMU[0].deviceTypeId == 0) {
      GD.ColorRGB(0xff0000);
      GD.cmd_text(0, 0, 27, 0, "Analog Error?");
  } else {
      GD.ColorRGB(0xaaaaaa);
      GD.cmd_text(0, 0, 27, 0, "ADC:");
      //GD.cmd_text(40, 0, 27, 0, (SMU[0].deviceTypeId & 0xfff0) == 0x00D0 ? "AD7172-2" : "?");
      
      GD.cmd_number(40,0,27,0,SMU[0].deviceTypeId);
      // String handling causing restarts after a while ????
      //GD.cmd_text(40, 0, 27, 0, deviceIdToText(SMU[0].deviceTypeId));

  }

  //showFanSpeed(220, 0);
  //GD.cmd_number(50,0,27,2,LOGGER.percentageFull);

  // debugging pushbutton values...
  //GD.ColorRGB(0x444444);
  //GD.cmd_number(0,0,27,3,analogRead(3)); // pushbutton analog value
  //GD.cmd_number(30,0,27,3,analogRead(16)); // encoder button value ?

   // uptime
  
  GD.ColorRGB(0xaaaaaa);
  GD.cmd_text(120, 0, 27, 0, "Uptime:");
  DIGIT_UTIL.displayTime(millis(), 190, 0);
#ifdef ARDUINO_TEENSY41
  if (ETHERNET2_UTIL.linkState) {
    GD.ColorA(100);
    GD.ColorRGB(0x00ff00);
  } else {
    GD.ColorA(200);
    GD.ColorRGB(0x888888);
  }
  if (ETHERNET2_UTIL.hasIpx) {
    GD.cmd_text(665, 0, 27, 0, ETHERNET2_UTIL.ipAddressString);
    // Uncomment below if you want to see command buffer info SCPI
    GD.ColorRGB(0xaaaaaa);
    GD.cmd_number(584,27,27,5, ETHERNET2_UTIL.receivedMessages);
    GD.cmd_text(640, 27, 27, 0, SCPI_CommandBuffer);
  } else {
     GD.cmd_text(665, 0, 27, 0, "No IP Address");
  }
#endif
  //GD.cmd_text(760, 0, 27, 0, ETHERNET2_UTIL.linkState?"ON":"OFF");
  GD.ColorA(255);
  GD.ColorRGB(0xaaaadd);

  int temp = UTILS.TC74_getTemperature();
  FAN.setAutoSpeedBasedOnTemperature(temp);
  
  GD.ColorRGB(0xdddddd);
  GD.cmd_text(410,0,27,0,"Fin:");
  if (temp > SETTINGS.getMaxTempAllowed()) {
    GD.ColorRGB(0xff0000);
    GD.Begin(RECTS);
    GD.Vertex2ii(440-3, 0);
    GD.Vertex2ii(480+5, 20);
    GD.ColorRGB(0xffffff);
  } else {
    GD.ColorRGB(0x00ff00);
  }
  GD.cmd_number(440,0,27,3, temp);
  GD.cmd_text(470,0,27,0,"C");

  GD.ColorRGB(0xaaaaaa);
  GD.cmd_text(500-10,0,27,0,"Fan:");
  GD.cmd_number(535-10,0,27,3,FAN.getSpeed());
  GD.cmd_text(555,0,27,0,"%");

  GD.ColorRGB(0x888888);
  GD.cmd_text(580,0,27,0,"Int:");
  float temp2 = UTILS.LM60_getTemperature(6); // requires LM60 onboard temp sensor...
  // convert to one decimal and show
  int num = (int)temp2;
  int dec = (temp2 - num) * 10.0;
  GD.cmd_number(610,0,27,0,num);
  GD.cmd_text(630,0,27,0,".");
  GD.cmd_number(635,0,27,0,dec);
  GD.cmd_text(645,0,27,0,"C");
  
  GD.ColorRGB(0xdddddd);
  // Show log info
  //int logEntries = RAM.getCurrentLogAddress();
  //GD.cmd_text(300,0,27,0,"Log:");
  //GD.cmd_number(340,0,27,0,logEntries);
  float percentage = LOGGER.percentageFull;
  if (percentage>=100.0) {
    GD.cmd_text(300,0,27,0,"Log:      %");
  } else if (percentage>=10.0) {
    GD.cmd_text(300,0,27,0,"Log:     %");
  } else {
    GD.cmd_text(300,0,27,0,"Log:    %");
  }
  GD.cmd_number(335,1,27,0, LOGGER.percentageFull);

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
//   GD.cmd_text(x,y,27,0, "Fan:");
//  GD.cmd_number(x+30,y,27,5, FAN.getRPMValueFiltered());
//  GD.cmd_text(x+80,y,27,0, "RPM");
//  DEBUG.print(FAN.getPWMFanRPM());
//  DEBUG.print("(");
//  DEBUG.print(FAN.getFanWidth());
//  DEBUG.flush();
}

void renderUpperDisplay(OPERATION_TYPE operationType, FUNCTION_TYPE functionType) {

  int x = 0;
  int y = 32;
 GD.Tag(0); // added to avoid wrong button detections when touching upper area...
  // show upper panel
  if (functionType == SOURCE_DC_VOLTAGE || functionType == SOURCE_DC_CURRENT) {
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
  } else if (functionType == MEASURE_RESISTANCE) {
    sourceVoltagePanel(x,y);
  }
  
}

int gestOldX = 0;
int gestOldY = 0;
int gestDurationX = 0;
int gestDurationY = 0;
uint32_t trackingDetectedTimer = millis();
bool ignoreGesture(int t){
  if (trackingDetectedTimer + 50 > millis()) {
    return true;
  }
  t = t & 0xff;
  // TODO: Make more generic.
  //       The idea is to ignore swipe especially when controlling sliders !
  for (int i=0;i<2;i++) {
    if (t == WIDGETS.sliderTags[i]) {
      trackingDetectedTimer = millis();
      WIDGETS.trackingOngoing = t;

      return true;
    }
  }
  WIDGETS.trackingOngoing = 0;
  return false;
}

int detectGestures() {
  GD.get_inputs();
  if (ignoreGesture(GD.inputs.track_tag)){  
    return GEST_NONE;
  }
  
  //DEBUG.println(GD.inputs.tag);
  int touchX = GD.inputs.x;
  int touchY = GD.inputs.y;
  int gestDistanceX = touchX - gestOldX;
  int gestDistanceY = touchY - gestOldY;

  if (touchX == -32768 && touchX == -32768) { // TODO: Why negative values when no touch ? Use unsigned ?
    gestDurationX = 0;
    gestDurationY = 0;
    gestDistanceX = 0;
    gestDistanceY = 0;
    gestOldX = 0;
    gestOldY = 0;
  }

 // if ((GD.inputs.tag == GESTURE_AREA_LOW || GD.inputs.tag == GESTURE_AREA_HIGH) && gestureDetected ==EST_NONE) {
  if (gestureDetected == GEST_NONE) {
    if (MAINMENU.active || anyDialogOpen()) {
      // dont detect swipes in lower part if dialog open
    } else {
    if (touchX > 0 && touchY > LOWER_WIDGET_Y_POS && gestDistanceX < -20 && !WIDGETS.isScrolling()) {
      if (++gestDurationX >= 3) {
        DEBUG.println("gesture = move left");
        DEBUG.flush();
        gestureDetected = GEST_MOVE_LEFT;
        gestDurationX = 0;
      }
    }
    else if (touchX > 0 && touchY > LOWER_WIDGET_Y_POS && gestDistanceX > 20 && !WIDGETS.isScrolling()) {
      if (++gestDurationX >= 3) {
        DEBUG.println("gesture = move right");
        DEBUG.flush();
        gestureDetected = GEST_MOVE_RIGHT;
        gestDurationX = 0;
      }
    } 
  }
    if (touchY > 0 && touchY<150 && gestDistanceY > 10 && !WIDGETS.isScrolling() && gestDistanceX < 20) {
       if (++gestDurationY >= 2) {
        DEBUG.println("gesture = move down from upper");
        DEBUG.flush();
        gestureDetected = GEST_MOVE_DOWN;
        gestDurationY = 0;
      }
    } 
    
  } else {
    gestureDetected = GEST_NONE;
    gestDurationX = 0;
    gestDurationY = 0;
  }
  gestOldX = touchX;
  gestOldY = touchY;
  return gestureDetected;
}




void notification(const char *text, int progressPercent) {
   GD.Begin(RECTS);
    GD.ColorA(230);  // already opaque, why ?
    GD.ColorRGB(0x222222);
    GD.Vertex2ii(180, 160);
    GD.Vertex2ii(620, 280);
    GD.ColorRGB(0xffffff);
    GD.cmd_text(250, 200 , 29, 0, text);

    if (progressPercent>=0) {
  
  
  GD.Begin(RECTS);
  GD.ColorA(255);
  GD.ColorRGB(0x00ff00);
  GD.Vertex2ii(250,230);
  GD.Vertex2ii(250+progressPercent*3, 260);

  GD.ColorRGB(0x000000);

      GD.cmd_number(260,233, 28, 3, progressPercent);
      GD.cmd_text(305, 233 , 28, 0, "%");
    }

}

int count = 1;
float sampleVolt = 10.0;
int logTimer = millis();
int msDigit = 0;
float simulatedWaveform;

unsigned long lastSimMs = millis();
bool simVolt = true;

static void handleSampling() {
  if (timeAtStartup +5000 < millis());
  int dataR = -1;
  // TODO: Use something else that deviceTypeId to "detect" problems with analog board...
  if (SMU[0].deviceTypeId != 0) {
    dataR = SMU[0].dataReady();
  } else {
    // TODO: Add indicator that power is probably of on analog board
    // Just give some simulated volt values with a litle noise.
    // Makes it possible to test without power on the hardware.
    // Is not the same as using USE_SIMULATOR
    if (lastSimMs + 100 < millis()) {
      lastSimMs = millis();
      dataR = simVolt ? 100 : 101; // swap between simulated voltage and current
      simVolt = !simVolt;
    }
  }

   if (DIGITIZER.digitize == true) {
     if (dataR == 0 or dataR == 1) {
       DIGITIZER.handleSamplingForDigitizer(dataR);
     } 
    //  else if (dataR == 100 or dataR == 101) { 
    //    //simulate
    //    DIGITIZER.handleSamplingForDigitizer(dataR-100);
    //  }
     return;
   }
 
  if (dataR == -1) {
    return;
  }
  if (dataR == -99) {
    DEBUG.println("DONT USE SAMPLE!");  
  } 
  else if (dataR == -98) {
    //DEBUG.println("OVERFLOW"); // haven't been able to get this to work...
  }
  
  else if (dataR == 1) {
    float Cout = SMU[0].measureCurrent(SMU[0].getCurrentRange());
    //TODO: DIffer between constant current and constant voltage nulling
    //Cout = Cout - V_CALIBRATION.nullValueCur[current_range];
    Cout = Cout - C_CALIBRATION.relativeValue[SMU[0].getCurrentRange()];
    C_STATS.addSample(Cout);
    C_FILTERS.updateMean(Cout, true); // moving average
  }
  else if(dataR == 0) {
    float Vout = SMU[0].measureMilliVoltage();
    Vout = Vout - V_CALIBRATION.relativeValue[SMU[0].getCurrentRange()];
    V_STATS.addSample(Vout);    
    V_FILTERS.updateMean(Vout, true);// moving average
    SIMPLE_STATS.registerValue(V_FILTERS.mean);
    LOGGER.registerValue2(Vout);
    // store now and then
    if (logTimer + 1000 < (int)millis()) {
     logTimer = millis();
     //RAM.logData(V_FILTERS.mean);
     RAM.logDataCalculateMean(V_FILTERS.mean, 1);
    }
  } 
  else if (dataR >= 100){ // Probably no analog board (no power?)
  // Just simulate voltage/current with some noise.
  // This is NOT the same as USE_SIMULATOR !
  // Just to make it possible to test the software even if there is no hardware...
  if (dataR == 100) {
    float v;
    v = SMU[0].getSetValue_micro();

    int noise = 20; //20uV simulated noise
    //simulate not calibrated
    if (abs(SMU[0].getSetValue_micro())>10000000) {
          v = v*1.001; //simulate not calibrated
          noise = 40;
    } else if (abs(SMU[0].getSetValue_micro())>5000000) {
          v = v*1.0005; //simulate not calibrated
          noise = 30;
    } else{
          v = v*1.0003; 
    }
    v = V_CALIBRATION.adc_nonlinear_compensation(v/1000.0) * 1000.0;
    v = v + random(0, noise);
    v = v - noise / 2.0;
    v = v / 1000.0;

    V_STATS.addSample(v);    
    V_FILTERS.updateMean(v, true);
    SIMPLE_STATS.registerValue(V_FILTERS.mean);
    if (logTimer + 1000 < (int)millis()) {
      logTimer = millis();
      //RAM.logData(V_FILTERS.mean);
      RAM.logDataCalculateMean(V_FILTERS.mean, 1);
    }

  } else {
    float i = 0;
    i = i + random(0, 20); // add 20uA simulated noise
    i=i/1000.0;
    C_STATS.addSample(i);    
    C_FILTERS.updateMean(i, true);
    SIMPLE_STATS.registerValue(C_FILTERS.mean);
  }

  }
}

void handleAutoCurrentRange() {
  if (WIDGETS.settingsCurrentAutoRange < 10000) { 
    return;
  }
     if (!ZEROCALIBRATION.autoNullStarted && !V_CALIBRATION.autoCalInProgress && !C_CALIBRATION.autoCalInProgress) {
      float milliAmpere = C_STATS.rawValue;
//      DEBUG.print(milliAmpere,5);
//      DEBUG.print("mA, current range:");
//      DEBUG.println(current_range);

      // auto current range switch. TODO: Move to hardware ? Note that range switch also requires change in limit
      float hysteresis = 0.5;
      float switchAt = SETTINGS.max_current_10mA_range();
      
        if (SMU[0].getCurrentRange() == AMP1 && abs(milliAmpere) < switchAt - hysteresis) {
          //current_range = MILLIAMP10;
          SMU[0].setCurrentRange(MILLIAMP10,operationType);
          DEBUG.println("switching to range 1");
           //TODO: Use getLimitValue from SMU instead of LIMIT_DIAL ?
          if (operationType == SOURCE_VOLTAGE){
            if (SMU[0].fltSetCommitCurrentLimit(SMU[0].getLimitValue_micro())) printError(_PRINT_ERROR_VOLTAGE_SOURCE_SETTING);
          } 

        }
        // TODO: Make separate function to calculate current based on shunt and voltage!
//        DEBUG.print("Check 10mA range and if it should switch to 1A... ma=");
//        DEBUG.print(milliAmpere);
//        DEBUG.print(" ");
//        DEBUG.println(switchAt);
        if (SMU[0].getCurrentRange() == MILLIAMP10 && abs(milliAmpere) > switchAt) {
          //current_range = AMP1;
          SMU[0].setCurrentRange(AMP1,operationType);
          DEBUG.println("switching to range 0");
           //TODO: Use getLimitValue from SMU instead of LIMIT_DIAL ?
          if (operationType == SOURCE_VOLTAGE){
            if (SMU[0].fltSetCommitCurrentLimit(SMU[0].getLimitValue_micro())) printError(_PRINT_ERROR_VOLTAGE_SOURCE_SETTING);
          } 

          
        }
    }
}

int displayUpdateTimer = millis();
int samplesUpdateTimer = millis();

int loopUpdateTimer = millis();


SCPI_Parser my_instrument;
void Identify(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  interface.print("Langehaug Consultancy, Poormanssmu,123,");
  interface.println(VERSION_NUMBER);
  interface.flush();
}
void sourceVoltageSCPI(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  DEBUG.println("SCPI Source voltage to given value (mV). MEAS:VOLT gives mV.");
  if (parameters.Size() > 0) {
    int mv = (String(parameters[0]).toInt());
    GD.__end();
    useVoltageFeedback();    
    SMU[0].fltSetCommitVoltageSource(mv*1000, true);
    //if (SMU[0].fltSetCommitCurrentLimit(SETTINGS.setCurrentLimit*1000)) printError(_PRINT_ERROR_VOLTAGE_SOURCE_SETTING);
    GD.resume();
    interface.println("OK");
  } else {
    interface.println("ERROR");
  }
  interface.flush();
  functionType = SOURCE_DC_VOLTAGE; // this is used to rended correct UI
}

void sourceCurrentSCPI(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  DEBUG.println("SCPI Source current to given value (uA). MEAS:CURR gives mA.");
  if (parameters.Size() > 0) {
    int uA = (String(parameters[0]).toInt());
    useCurrentFeedback();
    GD.__end();
    fltCommitCurrentSourceAutoRange(uA * 1000.0, true);
    GD.resume();
    //SMU[0].fltSetCommitVoltageLimit(SETTINGS.setVoltageLimit*1000, true);
    interface.println("OK");
  } else {
    interface.println("ERROR");
  }
  interface.flush();
  functionType = SOURCE_DC_CURRENT; // this is used to rended correct UI
}

void measureCurrentSCPI(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  DEBUG.println("SCPI Measure current");
  float milliAmp = C_FILTERS.mean;
    interface.println(milliAmp,3);
    interface.flush();
}

void measureVoltageSCPI(SCPI_C commands, SCPI_P parameters, Stream& interface) {
    DEBUG.println("SCPI Measure voltage");
    float millivolt = V_FILTERS.mean;
    interface.println(millivolt,3);
    interface.flush();
}

void systemErrSCPI(SCPI_C commands, SCPI_P parameters, Stream& interface) {
    DEBUG.println("SCPI Called systemErrSCPI, return 0");
    // Hardcoded to 0. Required to make it work properly with various SCPI software tools ???
    interface.println(0);
    interface.flush();
}

void systemTemperatureSCPI(SCPI_C commands, SCPI_P parameters, Stream& interface) {
    DEBUG.println("SCPI Measure system temperature");
    float temp = UTILS.LM60_getTemperature(6);
    interface.println(temp,1);
    interface.flush();
}

void setGainSCPI(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  DEBUG.println("SCPI set gain to given value.");
      DEBUG.println("");
    DEBUG.println("");

  if (parameters.Size() > 0) {
    double gain = (String(parameters[0]).toInt());
      DEBUG.println(gain);

    V_CALIBRATION.saveGain(gain);
    interface.println("OK");
  } else {
    interface.println("ERROR");
  }
  interface.flush();
 }

 void getGainSCPI(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  DEBUG.println("SCPI get gain.");
    DEBUG.println("");
    DEBUG.println("");

  CalibrationClass::config_gain gain =V_CALIBRATION.readGain();
  //V_CALIBRATION.readGain(gain);
   DEBUG.println(gain.gain_negative);
   DEBUG.println(gain.gain_positive);

    //interface.println(gain.gain_negative,1);
        interface.println(gain.gain_positive);

 //float dummy=5.5555; 
  //    interface.println(dummy,3);
          interface.flush();


 }


void finTemperatureSCPI(SCPI_C commands, SCPI_P parameters, Stream& interface) {
    DEBUG.println("SCPI Measure (cooler) fin temperature");
    float temp = UTILS.TC74_getTemperature();
    interface.println(temp,1);
    interface.flush();
}

void sourceVoltageILimitSCPI(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  DEBUG.println("SCPI Set current limit for voltage source (uA).");
  if (parameters.Size() > 0) {
    int uA = (String(parameters[0]).toInt());
    SMU[0].fltSetCommitCurrentLimit(uA);
    interface.println("OK");
  } else {
    interface.println("ERROR");
  }
  interface.flush();
}

void sourceCurrentVLimitSCPI(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  DEBUG.println("SCPI Set voltage limit for current source (mV).");
  if (parameters.Size() > 0) {
    int mV = (String(parameters[0]).toInt());
    SMU[0].fltSetCommitVoltageLimit(mV);
    interface.println("OK");
  } else {
    interface.println("ERROR");
  }
  interface.flush();

}


// scpi_setup is initially based on default Vrekrer_scpi example...
void scpi_setup()
{
  //We change the `hash_magic_number` variable before registering the commands
  my_instrument.hash_magic_number = 16; //16 will generate hash crashes
  //The default value is 37 and good values are prime numbers (up to 113)

  // Important commands to identfy instruments
  // SYST:ERR reponse seems to be important
  // to support in order to use various SCPI software, such as Keysight Command Expert ?
  my_instrument.RegisterCommand(F("*IDN?"), &Identify);
  my_instrument.RegisterCommand(F("SYST:ERR?"),  &systemErrSCPI);

  // special
  my_instrument.SetCommandTreeBase(F("SYSTem:"));
  //my_instrument.RegisterCommand(F(":TEMPerature?"),  &systemTemperatureSCPI);
  my_instrument.RegisterCommand(F(":TEMPerature:FIN?"),  &finTemperatureSCPI);
  my_instrument.RegisterCommand(F(":TEMPerature:SYS?"),  &systemTemperatureSCPI);

  // Various supported commands
  my_instrument.SetCommandTreeBase(F("SOURce:"));
  my_instrument.RegisterCommand(F(":VOLTage"), &sourceVoltageSCPI);
  my_instrument.RegisterCommand(F(":CURRent"), &sourceCurrentSCPI);
  my_instrument.RegisterCommand(F(":CURRent:VLIMit"), &sourceCurrentVLimitSCPI);
  my_instrument.RegisterCommand(F(":VOLTage:ILIMit"), &sourceVoltageILimitSCPI);
    my_instrument.RegisterCommand(F(":SETGain"), &setGainSCPI);


  my_instrument.SetCommandTreeBase(F("MEASure:"));
  my_instrument.RegisterCommand(F(":VOLTage?"), &measureVoltageSCPI);
  my_instrument.RegisterCommand(F(":CURRent?"), &measureCurrentSCPI);
    my_instrument.RegisterCommand(F(":GETGain?"), &getGainSCPI);

  // Override AC to actually return temp... TODO: Find a proper SCPI command...
  //my_instrument.RegisterCommand(F(":VOLTage:AC?"), &systemTemperatureSCPI);
  //my_instrument.RegisterCommand(F(":CURRent:AC?"), &finTemperatureSCPI);

  //my_instrument.SetCommandTreeBase(F("CALIbration:"));
  //my_instrument.RegisterCommand(F(":GETGagin?"), &getGainSCPI);
  //my_instrument.RegisterCommand(F(":SETGain"), &setGainSCPI);


  //`PrintDebugInfo` will print the registered tokens and 
  //command hashes to the serial interface.
  my_instrument.PrintDebugInfo();
  //See the result in the serial monitor and verify that
  //there are no duplicated hashes or hashes equal to zero.
  //Change the hash_magic_number to solve any problem.
}

#ifndef ARDUINO_TEENSY31
void scpiCommandDetectionLoop() {
  ETHERNET2_UTIL.loop();
   if ( ETHERNET2_UTIL.linkState ) {
      if (ETHERNET2_UTIL.newMessageReady()) {
        //Serial.print("New message ready:");
        Serial.println(ETHERNET2_UTIL.GetEthMsg());

        strncpy(SCPI_CommandBuffer, ETHERNET2_UTIL.GetEthMsg(), 40); // TODO: don't hardcode command buffer limit
      
        ETHERNET2_UTIL.clearBuffer();
        //my_instrument.Execute(commandBuffer, Serial);

      }
   }
}
#endif
void loop() {
#ifndef ARDUINO_TEENSY31
   scpiCommandDetectionLoop();
#endif

  if (V_CALIBRATION.autoCalInProgress) {
    V_CALIBRATION.autoCalADCfromDAC();
  } 
  if (C_CALIBRATION.autoCalInProgress) {
    C_CALIBRATION.autoCalADCfromDAC();
  } 
  

  // No need to update display more often that the eye can detect.
  // Too often will cause jitter in the sampling because display and DAC/ADC share same SPI port.
  // Will that be improved by Teensy 4 where there are more that one SPI port ?
  //
  // Note that the scrolling speed and gesture detection speed will be affected.
  // 
  bool reduceResolution = !SMU[0].use100uVSetResolution() && changeDigit == 1000;
  ROTARY_ENCODER.handle(reduceResolution);

  //TODO: Not sure if this should be here.... this is more that "display time"... it also handles sampling of not driven by timer interrupt...
  if (displayUpdateTimer + 20 > (int)millis()) {
    //return; 
  }
 
  displayUpdateTimer = millis();
  if (functionType == DIGITIZE) {
   #ifndef SAMPLING_BY_INTERRUPT 
    handleSampling(); 
   #endif
    DIGITIZER.loopDigitize(reduceDetails());
    if ( (!DIGITIZER.digitize || !DIGITIZER.allowTrigger || !DIGITIZER.continuous) && ( (int)millis() > samplesUpdateTimer + 100  || MAINMENU.active)){
      DIGITIZER.rendering = true;
      samplesUpdateTimer = (int)millis();
      SMU[0].disable_ADC_DAC_SPI_units();
      GD.resume();
      GD.Clear();
      detectGestures();
      showStatusIndicator(700, 40, "COMP", SMU[0].hasCompliance(), true);

      if (MAINMENU.active) {
        int tag = GD.inputs.tag;
        // TODO: don't need to check buttons for inactive menus or functions...
        MAINMENU.handleButtonAction(tag);
      }

      PUSHBUTTONS.handle();
      PUSHBUTTON_ENC.handle();
      handleMenuScrolldown();
      renderMainHeader();
      DIGITIZER.renderGraph(reduceDetails());

      GD.swap();
      GD.__end();
      DIGITIZER.rendering = false;
    }

  } else if (functionType == GRAPH) {

//    if (loopUpdateTimer + 10 > millis() ) {
//      return;
//    }
//    loopUpdateTimer = millis();

    OPERATION_TYPE ot = getOperationType();
    disable_ADC_DAC_SPI_units();
    GD.resume();
    GD.Clear();
    detectGestures();
    renderMainHeader();
    TRENDGRAPH.loop(ot);

    handleMenuScrolldown();
    int tag = GD.inputs.tag;
    // TODO: don't need to check buttons for inactive menus or functions...
    MAINMENU.handleButtonAction(tag);
    PUSHBUTTONS.handle();
        PUSHBUTTON_ENC.handle();

    GD.swap();
    GD.__end();
    
  } else if (functionType == DATALOGGER) {

//    if (loopUpdateTimer + 10 > millis() ) {
//      return;
//    }
//    loopUpdateTimer = millis();

    disable_ADC_DAC_SPI_units();
    GD.resume();
    GD.Clear();
    detectGestures();
    renderMainHeader();
    LOGGER.loop(reduceDetails());

    handleMenuScrolldown();
    int tag = GD.inputs.tag;
    // TODO: don't need to check buttons for inactive menus or functions...
    MAINMENU.handleButtonAction(tag);
    PUSHBUTTONS.handle();
    PUSHBUTTON_ENC.handle();

    GD.swap();
    GD.__end();
    #ifndef SAMPLING_BY_INTERRUPT 
    handleSampling(); 
   #endif
    
  }
  else {
    loopMain();
  }
}



int prevTag = 0;
int tagTimer = millis();
bool buttonPressed=false;
int buttonPressedPeriod = 0;
int prevButton = 0;
int x[10];
int y[10];

int checkButtons() {
  int valueToReturnIfTooFast = 0;
  int nrOfChecks = 3;

    if (MAINMENU.active == true) {
      return 0;
    }
    
   int trackTag = GD.inputs.track_tag & 0xff;
   if (trackTag != 0 && WIDGETS.trackingOngoing) {
     return 0;
   }

   int tag;

    tag = GD.inputs.tag;



    if (tag>0 && tagTimer + 20 > (int)millis()) {
       return 0;
    } 
        tagTimer = millis();

    if (tag>0) {
      int touchX = GD.inputs.x;
      int touchY = GD.inputs.y;
      buttonPressed = true;
      if (prevButton == buttonPressed) {
        x[buttonPressedPeriod] = touchX;
        y[buttonPressedPeriod] = touchY;
        buttonPressedPeriod ++;
        //DEBUG.print("Button pressed:");
        //DEBUG.println(buttonPressedPeriod);
      }
      prevButton = buttonPressed;
    } else {
      prevButton = 0;
      buttonPressed = false;
      buttonPressedPeriod = 0;
    }

    if (buttonPressedPeriod<nrOfChecks) {
      return 0;
    }
    bool accidental = false;
    // check that all registred touches was within a certain area. 
    // If not, the touch registered was probably part of a gesture and should be ignored
    for (int i = 0; i<nrOfChecks-1;i++) {
      if (x[i] > x[i+1] + 10 or x[i] < x[i+1] - 10 or  y[i] > y[i+1] + 10 or  y[i] < y[i+1] - 10) {
        // at least one touch outside acceptable area !
        accidental = true;
      }
    }

    if (accidental) {
      //DEBUG.println("Gesture accidentally touched a button...");
      return 0;
    } else {
      //DEBUG.println("Button pressed :-)");
    }

    buttonPressedPeriod= 0;
    prevTag = tag;
    
    if (tag == BUTTON_SOURCE_SET) {
      DEBUG.print("open dial to set source, start with value ");
      DEBUG.println((float)SMU[0].getSetValue_micro()/1000.0);
      SOURCE_DIAL.open(operationType, SET,  closeSourceDCCallback, SMU[0].getSetValue_micro());
    } else if (tag == BUTTON_LIM_SET) {
      DEBUG.print("open dial to set limit, start with value ");
      //DIGIT_UTIL.print_uint64_t(SMU[0].getLimitValue_micro());
      DEBUG.println((float)SMU[0].getLimitValue_micro()/1000.0);
      LIMIT_DIAL.open(operationType, LIMIT, closeSourceDCCallback, SMU[0].getLimitValue_micro());
    } 
    // else if (tag == BUTTON_REL) {
    //   DEBUG.println("Set relative");
    //   V_CALIBRATION.toggleRelativeValue(V_STATS.rawValue, SMU[0].getCurrentRange());
    //   C_CALIBRATION.toggleRelativeValue(C_STATS.rawValue, SMU[0].getCurrentRange());
    // } else if (tag == BUTTON_UNCAL) {
    //   DEBUG.println("Uncal set");
    //   V_CALIBRATION.toggleCalibratedValues();
    //   C_CALIBRATION.toggleCalibratedValues();
    // } else if (tag == BUTTON_CLEAR_BUFFER) {
    //   DEBUG.println("clearbuffer set");

    //   SIMPLE_STATS.clear(); // TODO: Separate clearing for this ?
    //   V_STATS.clearBuffer();
    //   C_STATS.clearBuffer();
    //   DIGIT_UTIL.startIndicator(tag); 
    // } 
    else if (tag == BUTTON_CUR_AUTO) { //TODO: Change name
      if (timeSinceLastChange + 500 < millis()){
        DEBUG.println("current range set");
       
        // swap current range
        CURRENT_RANGE currentRange = SMU[0].getCurrentRange();
        if (currentRange == AMP1) {
          currentRange = MILLIAMP10;
        } else {
          currentRange = AMP1;
        }
        timeSinceLastChange = millis();
        
        GD.__end();
        disable_ADC_DAC_SPI_units(); 
        SMU[0].setCurrentRange(currentRange,operationType);
        GD.resume();

      }
    } else  {
      WIDGETS.checkButtons(tag);
    } 

    #ifndef USE_SIMULATOR // dont want to mess up calibration while in simulation mode...
    
    // TODO handleCalibrationButtons combines voltage and current.
    //      should be rewritten so that V_CALIBRATION and C_CALIBRATION handles volt and current
    //      respectivly. 
    //      Currently, the code is only moved to Calibration. Just use the V_CALIBRATION for now
    //      even though it also contains current calibration
    if (!V_CALIBRATION.handleCalibrationButtons(tag,operationType, SMU[0].getCurrentRange())) {
      return valueToReturnIfTooFast;
    }
    if (tag == BUTTON_DAC_ZERO_CALIBRATE) {
       if (timeSinceLastChange + 1000 > millis()){
        return valueToReturnIfTooFast;
       } 
       DEBUG.println("Start zero calibration of dac....");
       DIGIT_UTIL.startIndicator(tag);

       timeSinceLastChange = millis();
       ZEROCALIBRATION.startNullCalibration(operationType);
    }
    
    #endif  // USE_SIMULATOR
    return tag;
}

void handleFunctionSpecifcButtonAction(FUNCTION_TYPE functionType, int tag, int tag2) {
  if (functionType == SOURCE_PULSE) {
        FUNCTION_PULSE.handleButtonAction(tag);
      } else if (functionType == SOURCE_SWEEP) {
        // Use raw tags for sweep function. TODO: update checkBUttons to handle holding etc.
        FUNCTION_SWEEP.handleButtonAction(tag2); 
      }
}


int settingsShortAdc = 0;
//int settingsCurrentAutoRange = 65000;
int settingsInternalADCRef = 0;
int voltageMeasurementGainX2 = 0;
int voltageOnly = 0;

void loopMain()
{
  ZEROCALIBRATION.handleAutoNull();
  operationType = getOperationType();
  
  GD.__end();

  houseKeeping();
  ADA4254.indicateADA4254status();

  // Auto range current measurement while sourcing voltage. 
  // Note that this gives small glitches in voltage.
  // TODO: Find out how large glitches and if it's a real problem...
  if (operationType == SOURCE_VOLTAGE || operationType == SOURCE_CURRENT) {
      handleAutoCurrentRange();
  }

  #ifndef SAMPLING_BY_INTERRUPT 
    handleSampling(); 
  #endif
  // Dont use interrupt driven voltage measurement when in pulse mode
  // because pulse uses interrupt to create pulses.
  // Should work with two interrupts at the same time ???? Why do I have problems ?
  // TODO: Fix the above problem and see if this hack below can be removed 
  #ifdef SAMPLING_BY_INTERRUPT 
  if (functionType == SOURCE_PULSE) {
    handleSampling(); 
  }
  #endif

  if (functionType == SOURCE_SWEEP) {
    FUNCTION_SWEEP.handle();
  }
  
  disable_ADC_DAC_SPI_units();
  GD.resume();
  GD.Clear();

  if (showSettings == false) {
    renderUpperDisplay(operationType, functionType);  
    detectGestures();
    if (!gestureDetected) {
      int tag = checkButtons();
      MAINMENU.handleButtonAction(GD.inputs.tag);
      handleFunctionSpecifcButtonAction(functionType, tag, GD.inputs.tag);
    }
    handleWidgetScrollPosition();
    displayWidget();  
    handleMenuScrolldown();
    renderMainHeader();

  } else {
      renderMainHeader();

    // Special page without widgets etc...
    //renderUpperDisplay(operationType, functionType); 
   

//GD.cmd_track(360, 62, 80, 20, 42);


    int lineY = 60;
    GD.cmd_text(180, lineY - 7, 29, 0, "Short ADC input");
    GD.Tag(42);
    GD.cmd_toggle(50, lineY, 100, 29, OPT_FLAT, settingsShortAdc,
    "disabled" "\xff" "enabled");
    GD.cmd_track(50, lineY, 100, 40, 42);
    lineY+=50;
    
    GD.cmd_text(180, lineY - 7 , 29, 0, "Current auto range");
    GD.Tag(43);
    GD.cmd_toggle(50, lineY, 100, 29, OPT_FLAT, WIDGETS.settingsCurrentAutoRange, // TODO: Don't fetch from widgets
    "disabled" "\xff" "enabled");
    GD.cmd_track(50, lineY, 80, 40, 43);
    lineY+=50;

    GD.cmd_text(180, lineY - 7 , 29, 0, "Internal reference");
    GD.Tag(44);
    GD.cmd_toggle(50, lineY, 100, 29, OPT_FLAT, settingsInternalADCRef,
    "disabled" "\xff" "enabled");
    GD.cmd_track(50, lineY, 80, 40, 44);

    lineY+=50;

    GD.cmd_text(180, lineY - 7 , 29, 0, "Voltage measurement gain x2");
    GD.Tag(45);
    GD.cmd_toggle(50, lineY, 100, 29, OPT_FLAT, voltageMeasurementGainX2,
    "disabled" "\xff" "enabled");
    GD.cmd_track(50, lineY, 80, 40, 45);

   lineY+=50;

    GD.cmd_text(180, lineY - 7 , 29, 0, "Only voltage");
    GD.Tag(46);
    GD.cmd_toggle(50, lineY, 100, 29, OPT_FLAT, voltageOnly,
    "disabled" "\xff" "enabled");
    GD.cmd_track(50, lineY, 80, 40, 46);



    GD.Tag(244);
    GD.cmd_button(300,lineY +30,120,50,29,0,"CLOSE");


    GD.Tag(0);

    //45 max degree ?
    float degreeV = (V_FILTERS.mean) * 45.0/10000.0; 
    ANALOG_GAUGE.renderAnalogGaugeValue(25,300,350, degreeV, V_FILTERS.mean, "mV", "+/-10V");
    
    float degreeC = (C_FILTERS.mean) * 45.0/1000.0; 
    ANALOG_GAUGE.renderAnalogGaugeValue(425,300,350, degreeC, C_FILTERS.mean, "mA", "+/-1000mA");

    //detectGestures();
    GD.get_inputs();
    int tag = GD.inputs.tag;
    switch (tag & 0xff) {
      case 42:
        settingsShortAdc = GD.inputs.track_val;
        if (settingsShortAdc > 30000) {
          settingsShortAdc = 65535;
          SMU[0].shortAdcInput(true);
        }
        else if (settingsShortAdc <= 30000) {
          settingsShortAdc = 0;
          SMU[0].shortAdcInput(false);
        }
        break;
      case 43:
        WIDGETS.settingsCurrentAutoRange = GD.inputs.track_val;
        if (WIDGETS.settingsCurrentAutoRange > 30000) {
          WIDGETS.settingsCurrentAutoRange = 65535;
        }
        else if (WIDGETS.settingsCurrentAutoRange <= 30000) {
          WIDGETS.settingsCurrentAutoRange = 0;
        }
        break;
      case 44:
        settingsInternalADCRef = GD.inputs.track_val;
        if (settingsInternalADCRef > 30000) {
          settingsInternalADCRef = 65535;
          SMU[0].internalRefInput(true);
        }
        else if (settingsInternalADCRef <= 30000) {
          settingsInternalADCRef = 0;
          SMU[0].internalRefInput(false);
        }
        break;
      case 45:
        voltageMeasurementGainX2 = GD.inputs.track_val;
        if (voltageMeasurementGainX2 > 30000) {
          voltageMeasurementGainX2 = 65535;
          SMU[0].voltageMeasurementGainX2 = true;
        }
        else if (voltageMeasurementGainX2 <= 30000) {
          voltageMeasurementGainX2 = 0;
          SMU[0].voltageMeasurementGainX2 = false;
        }
        break;
      case 46:
        voltageOnly = GD.inputs.track_val;
        if (voltageOnly > 30000) {
          voltageOnly = 65535;
          //TODO: Avoid overwrite by other mechanism !
          SMU[0].enableCurrentMeasurement = false;
          SMU[0].enableVoltageMeasurement = true;
           
        }
        else if (voltageOnly <= 30000) {
          voltageOnly = 0; 
          //TODO: Avoid overwrite by other mechanism !
          SMU[0].enableCurrentMeasurement = true;
          SMU[0].enableVoltageMeasurement = true;
                   

        }
        break;
      case 244: // close settings
        showSettings = false;
        break;

    }


   //   DEBUG.println(tag);
   //   GD.swap(); 
   // GD.__end();
   // return;
  }

  PUSHBUTTONS.handle();
  PUSHBUTTON_ENC.handle();

  if (V_CALIBRATION.autoCalInProgress or C_CALIBRATION.autoCalInProgress) {
    notification("Auto calibration in progress...", -1);
  }
  if (ZEROCALIBRATION.autoNullStarted && !ZEROCALIBRATION.nullCalibrationReady) {
    int progress = ZEROCALIBRATION.getProgress();
    notification("Wait for null adjustment...", progress);
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

void rotaryChangedDontCareFn(float changeVal) {
}

void pushButtonEncDontCareFn(int key, bool quickPress, bool holdAfterLongPress, bool releaseAfterLongPress) {
  DEBUG.println("pushButtonEncDontCareFn disabled pushbutton");

}

void useCurrentFeedback() {
  SMU[0].setGPIO(0, 1);
}

void useVoltageFeedback() {
  SMU[0].setGPIO(0, 0);
}

void closeMainMenuCallback(FUNCTION_TYPE newFunctionType) {
  
  DEBUG.println("Closed main menu callback");
  DEBUG.println("New Selected function:");
  DEBUG.println(newFunctionType);
  DEBUG.println("Old function:");
  DEBUG.println(functionType);
  DEBUG.flush();

  // "unregister" function to be called when rotaty encoder is detected.
  // Has to be reinitiated by the function that needs it.
  // This is to avoid unwanted stuff to happen when rotating the knob in
  // different function views.
  ROTARY_ENCODER.init(rotaryChangedDontCareFn); 
  PUSHBUTTON_ENC.setCallback(pushButtonEncDontCareFn); 

  // do a close on the existing function. It should do neccessary cleanup
  if (functionType == SOURCE_PULSE) {
    FUNCTION_PULSE.close(); // Hmmm... how to let something go in the background while showing logger ????
    initDefaultSamplingIfByInterrupt();
  } else if (functionType == SOURCE_SWEEP) {
    FUNCTION_SWEEP.close(); //Hmmm... how to let something go in the background while showing logger ????
  } else if (functionType == DIGITIZE) {
    //TODO: Use method when digitizer is moved out as separate class
    //      For now, just readjust sampling speed
    DIGITIZER.close();
    //SMU[0].setSamplingRate(20); //TODO: Should get back to same as before, not a default one
  } else if (functionType == SOURCE_DC_CURRENT) {
      //GD.__end();
      //SMU[0].setGPIO(0, 0); // use volt feedback
      changeDigit = 0; // disable edit mode
  } 
  else if (functionType == SOURCE_DC_VOLTAGE) {
      //GD.__end();
      //SMU[0].setGPIO(0, 0); // use volt feedback
      changeDigit = 0; // disable edit mode
  }
 
  // The newly selected function...
  if (newFunctionType == SOURCE_PULSE) {
    #ifdef SAMPLING_BY_INTERRUPT
    GD.__end();
    ROTARY_ENCODER.init(FUNCTION_PULSE.rotaryEncChanged);  
    PUSHBUTTON_ENC.setCallback(FUNCTION_PULSE.rotaryEncButtonChanged);
    normalSamplingTimer.end();  // stop normal voltage mesurement sampling
    #endif


    FUNCTION_PULSE.open(operationType, closedPulse);
  } else if (newFunctionType == SOURCE_SWEEP) {
    GD.__end();
    FUNCTION_SWEEP.open(operationType, closedSweep);
    GD.resume();
  } else if (newFunctionType == DIGITIZE) {
    ROTARY_ENCODER.init(DIGITIZER.rotaryEncChanged); 
    PUSHBUTTON_ENC.setCallback(DIGITIZER.rotaryEncButtonChanged);
    DIGITIZER.open();
    
  }
  else if (newFunctionType == SOURCE_DC_VOLTAGE) {   
    //disable_ADC_DAC_SPI_units();
    ROTARY_ENCODER.init(rotaryChangedVoltCurrentFn);
    PUSHBUTTON_ENC.setCallback(pushButtonEncInterrupt); 
    GD.__end();
    
    useVoltageFeedback();
    //SMU[0].setGPIO(0, 0); // use voltage feedback
    
    if (SMU[0].operationType == SOURCE_VOLTAGE) {
      // If previous SMU operation was sourcing voltage, use that voltage
      if (SMU[0].fltSetCommitVoltageSource(SETTINGS.setMilliVoltage*1000, true)) printError(_PRINT_ERROR_VOLTAGE_SOURCE_SETTING);
      if (SMU[0].fltSetCommitCurrentLimit(SETTINGS.setCurrentLimit*1000)) printError(_PRINT_ERROR_VOLTAGE_SOURCE_SETTING);
    } else {
      // If previous SMU operation was sourcing current, use a predefined voltage
      if (SMU[0].fltSetCommitVoltageSource(SETTINGS.setMilliVoltage*1000, true)) printError(_PRINT_ERROR_VOLTAGE_SOURCE_SETTING);
      if (SMU[0].fltSetCommitCurrentLimit(SETTINGS.setCurrentLimit*1000)) printError(_PRINT_ERROR_VOLTAGE_SOURCE_SETTING);
    }
    GD.resume();
  }
  else if (newFunctionType == SOURCE_DC_CURRENT) {
    ROTARY_ENCODER.init(rotaryChangedVoltCurrentFn);
    PUSHBUTTON_ENC.setCallback(pushButtonEncInterrupt); 
    //disable_ADC_DAC_SPI_units();
    GD.__end();
    
    useCurrentFeedback();
    //SMU[0].setGPIO(0, 1); // use current feedback

    if (SMU[0].operationType == SOURCE_CURRENT) {
      // If previous SMU operation was sourcing current, use that current
      fltCommitCurrentSourceAutoRange(SETTINGS.setMilliAmpere*1000, true);
      if (SMU[0].fltSetCommitVoltageLimit(SETTINGS.setVoltageLimit*1000)) printError(_PRINT_ERROR_VOLTAGE_SOURCE_SETTING);
    } else {
      // If previous SMU operation was sourcing voltage, use a predefined current
      fltCommitCurrentSourceAutoRange(SETTINGS.setMilliAmpere*1000, true);
      if (SMU[0].fltSetCommitVoltageLimit(SETTINGS.setVoltageLimit*1000)) printError(_PRINT_ERROR_VOLTAGE_SOURCE_SETTING);
    }

    GD.resume();
  }
  else if (newFunctionType == DATALOGGER) {
     //ROTARY_ENCODER.init(TRENDGRAPH.rotaryChangedFn);
      GD.__end();
      ROTARY_ENCODER.init(LOGGER.rotaryChangedFn);
      PUSHBUTTON_ENC.setCallback(LOGGER.rotarySwitchFn); 
      GD.resume();
  }
  else if (newFunctionType == SHOWSETTINGS) {
      // Just added to make it possible to toggle settings also when physical buttons available...
      showSettings = true;
  }
  // else if (newFunctionType == DIGITIZE) {
  //   GD.resume();
  //} 
  else {
    GD.resume();
  }
  functionType = newFunctionType;

  
}

void fltCommitCurrentSourceAutoRange(float uV, bool autoRange) {
   // auto current range when sourcing current
   if (autoRange) {
      if (abs(uV) < 8000 ) {  // TODO: should theoretically be 10mA full scale, but there seem to be some limitations... i.e. reference volt a bit less that 5V...
        //current_range = MILLIAMP10;
        SMU[0].setCurrentRange(MILLIAMP10, operationType);
      } else {
        //current_range = AMP1;
        SMU[0].setCurrentRange(AMP1,operationType);
      }
      
      if (SMU[0].fltSetCommitCurrentSource(uV)) printError(_PRINT_ERROR_VOLTAGE_SOURCE_SETTING);
   }
   else {
      if (SMU[0].fltSetCommitCurrentSource(uV)) printError(_PRINT_ERROR_VOLTAGE_SOURCE_SETTING);
   }
}

void closeSourceDCCallback(int set_or_limit, bool cancel) {

  if (cancel) {
      DEBUG.println("Closed SET/LIMIT dialog by cancel");
    return;
  }
  GD.__end();
  disable_ADC_DAC_SPI_units();
  if (set_or_limit == SET) {
    float mv = SOURCE_DIAL.getMv(); // TODO: get current value from another place ?
    DEBUG.print("Closed SET dialog, value=");
    DEBUG.println(mv);
    if (operationType == SOURCE_VOLTAGE) {
       if (SMU[0].fltSetCommitVoltageSource(mv * 1000, true)) printError(_PRINT_ERROR_VOLTAGE_SOURCE_SETTING);
    } else {
      fltCommitCurrentSourceAutoRange(mv * 1000, false);
    }
  }
  if (set_or_limit == LIMIT) {
    float mv = LIMIT_DIAL.getMv(); // TODO: get current value from another place ?
    DEBUG.print("Closed LIMIT dialog, value=");
    DEBUG.println(mv);
    if (operationType == SOURCE_VOLTAGE) {
     if (SMU[0].fltSetCommitCurrentLimit(mv * 1000)) printError(_PRINT_ERROR_CURRENT_SOURCE_SETTING);
    } else {
     if (SMU[0].fltSetCommitVoltageLimit(mv * 1000)) printError(_PRINT_ERROR_CURRENT_SOURCE_SETTING);
    }
  }
  GD.resume();
}
