/************************************************
 *  Poor mans smu
 *  
 *  more to come...
 *  
 *  
 *  
 *  External library:
 *  
 *  FT81x graphics driver is initially copied (2016) from 
 *  jamesbowman / gd2-lib
 *  (https://github.com/jamesbowman/gd2-lib.git)
 *  -compile om 8266
 *  -support 800x480 pixels for all graphics
 * 
 ************************************************/

#include <SPI.h>
#include "GD2.h"
#include "colors.h"
#include "volt_display.h"
#include "current_display.h"
//#include "Dac.h"
#include "Stats.h"
#include "digit_util.h"


#include "Arduino.h"
#include "LT_I2C.h"
#include "Wire.h"
#include "LTC2485.h"
#include "LTC2655.h"
#include "EasySMU.h"
#include "EasySMU2.h"

//#include "PrintError.h"
//#include "MasterAddressConstants.h"

#define _PRINT_ERROR_VOLTAGE_SOURCE_SETTING 0
#define _PRINT_ERROR_CURRENT_SOURCE_SETTING 1
#define _PRINT_ERROR_SERIAL_COMMAND 2
#define _PRINT_ERROR_NO_CALIBRATION_DATA_FOUND 3
#define _PRINT_ERROR_FACTORY_CALIBRATION_RESTORE_FAILED_CATASTROPHICALLY

//EEPROM functions take 8-bit address, which is inconsistent with all other code
#define I2C_Board0_EEPROM_ADDR0    (0x51<<1)
#define I2C_Board0_LTC2485_Iout_LH_ADDR0    0x15
#define I2C_Board0_LTC2485_Iout_LH_Global    0x76
#define I2C_Board0_LTC2485_Vout_LF_ADDR0    0x14
#define I2C_Board0_LTC2485_Vout_LF_Global    0x76
#define I2C_Board0_LTC2655_LLL_ADDR0    0x11
#define I2C_Board0_LTC2655_LLL_Global    0x72
#define I2C_Board1_EEPROM_ADDR0    (0x59<<1)
#define I2C_Board1_LTC2485_Iout_LH_ADDR0    0x1D
#define I2C_Board1_LTC2485_Iout_LH_Global    0x7E
#define I2C_Board1_LTC2485_Vout_LF_ADDR0    0x1C
#define I2C_Board1_LTC2485_Vout_LF_Global    0x7E
#define I2C_Board1_LTC2655_LLL_ADDR0    0x19
#define I2C_Board1_LTC2655_LLL_Global    0x7A
#define I2C_Board2_EEPROM_ADDR0    (0x41<<1)
#define I2C_Board2_LTC2485_Iout_LH_ADDR0    0x05
#define I2C_Board2_LTC2485_Iout_LH_Global    0x66
#define I2C_Board2_LTC2485_Vout_LF_ADDR0    0x04
#define I2C_Board2_LTC2485_Vout_LF_Global    0x66
#define I2C_Board2_LTC2655_LLL_ADDR0    0x01
#define I2C_Board2_LTC2655_LLL_Global    0x62
#define I2C_Board3_EEPROM_ADDR0    (0x49<<1)
#define I2C_Board3_LTC2485_Iout_LH_ADDR0    0x0D
#define I2C_Board3_LTC2485_Iout_LH_Global    0x6E
#define I2C_Board3_LTC2485_Vout_LF_ADDR0    0x0C
#define I2C_Board3_LTC2485_Vout_LF_Global    0x6E
#define I2C_Board3_LTC2655_LLL_ADDR0    0x09
#define I2C_Board3_LTC2655_LLL_Global    0x6A
#define I2C_Board4_EEPROM_ADDR0    (0x71<<1)
#define I2C_Board4_LTC2485_Iout_LH_ADDR0    0x35
#define I2C_Board4_LTC2485_Iout_LH_Global    0x56
#define I2C_Board4_LTC2485_Vout_LF_ADDR0    0x34
#define I2C_Board4_LTC2485_Vout_LF_Global    0x56
#define I2C_Board4_LTC2655_LLL_ADDR0    0x31
#define I2C_Board4_LTC2655_LLL_Global    0x52
#define I2C_Board5_EEPROM_ADDR0    (0x79<<1)
#define I2C_Board5_LTC2485_Iout_LH_ADDR0    0x3D
#define I2C_Board5_LTC2485_Iout_LH_Global    0x5E
#define I2C_Board5_LTC2485_Vout_LF_ADDR0    0x3C
#define I2C_Board5_LTC2485_Vout_LF_Global    0x5E
#define I2C_Board5_LTC2655_LLL_ADDR0    0x39
#define I2C_Board5_LTC2655_LLL_Global    0x5A
#define I2C_Board6_EEPROM_ADDR0    (0x61<<1)
#define I2C_Board6_LTC2485_Iout_LH_ADDR0    0x25
#define I2C_Board6_LTC2485_Iout_LH_Global    0x46
#define I2C_Board6_LTC2485_Vout_LF_ADDR0    0x24
#define I2C_Board6_LTC2485_Vout_LF_Global    0x46
#define I2C_Board6_LTC2655_LLL_ADDR0    0x21
#define I2C_Board6_LTC2655_LLL_Global    0x42
#define I2C_Board7_EEPROM_ADDR0    (0x69<<1)
#define I2C_Board7_LTC2485_Iout_LH_ADDR0    0x2D
#define I2C_Board7_LTC2485_Iout_LH_Global    0x4E
#define I2C_Board7_LTC2485_Vout_LF_ADDR0    0x2C
#define I2C_Board7_LTC2485_Vout_LF_Global    0x4E
#define I2C_Board7_LTC2655_LLL_ADDR0    0x29
#define I2C_Board7_LTC2655_LLL_Global    0x4A
//!touchscreen I2C address
#define I2C_FT6206_ADDR0    0x38


//EasySMU2 SMU[1] = {
//      EasySMU2()
//};

EasySMU SMU[1] =
{
    EasySMU(I2C_Board0_EEPROM_ADDR0, I2C_Board0_LTC2655_LLL_ADDR0, I2C_Board0_LTC2485_Vout_LF_ADDR0, I2C_Board0_LTC2485_Iout_LH_ADDR0)
};

StatsClass V_STATS;
//StatsClass C_STATS;


float DACVout;  // TODO: Dont use global

float setMv;
float setMa;

#include "dial.h"

void setup()
{
  //wdt_disable();
  Serial.begin(9600);
  //DAC.init(); // Not using any separate DAC. Using easySMU instead
  Serial.println("Initializing WeatherNG graphics controller FT81x...");
  GD.begin(0);
  Serial.println("Done!");

 

   i2c_enable(); //Set Linduino default I2C speed instead of Wire default settings.
  
    pinMode(QUIKEVAL_MUX_MODE_PIN, OUTPUT);   //Set Linduino MUX pin to disable the I2C MUX. Otherwise it can cause unpredictable behavior.
    digitalWrite(QUIKEVAL_MUX_MODE_PIN, LOW);
  
    SMU[0].ReadCalibration();
    setMv = 0.0;
    setMa = 10.0;
    SMU[0].fltSetCommitCurrentSource(setMa / 1000.0,_SOURCE_AND_SINK);
    SMU[0].fltSetCommitVoltageSource(setMv / 1000.0);
    SMU[0].EnableOutput();



 
  GD.cmd_romfont(1, 34); // put FT81x font 34 in slot 1
  //GD.wr(REG_PWM_DUTY, 10);

  
  
}

void voltagePanel(int x, int y) {
  GD.ColorA(70);
  GD.Begin(LINE_STRIP);
  GD.LineWidth(32);
  GD.ColorRGB(0,255,150);
  GD.Vertex2ii(x+10, y+30); 
  GD.Vertex2ii(x+790, y+30);
  GD.Vertex2ii(x+790, y+210);
  GD.Vertex2ii(x+10, y+210);
  GD.Vertex2ii(x+10, y+30);

  // clear area behind heading
  GD.ColorA(255);
  GD.Begin(RECTS);
  GD.ColorRGB(00,00,00);
  GD.Vertex2ii(x+56, y+20);
  GD.Vertex2ii(x+260, y+50);

  // heading
  GD.ColorRGB(200,255,200);
  GD.cmd_text(x+56, y+16 ,   29, 0, "SOURCE VOLTAGE");

  //float rawMv = DACVout; // TODO: Dont use global
  //float rawMv = SMU[0].MeasureVoltage() * 1000.0;
  float rawMv = SMU[0].MeasureVoltage() * 1000.0;
  //float rawMv = 56.233;
  V_STATS.addSample(rawMv);
 
  VOLT_DISPLAY.renderMeasured(x + 17,y , rawMv);
  VOLT_DISPLAY.renderSet(x + 120, y+150, setMv);

  renderDeviation(x+667,y+147, rawMv, setMv, false);
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
  if (setM != 0.0) {
    if (devPercent < 1.0) {
      GD.cmd_text(x, y+16, 30, 0, "0.");
      GD.cmd_number(x+30, y+16, 30, 3, devPercent * 1000.0);
      GD.cmd_text(x+30+50, y+16, 30, 0, "%");
    } else if (devPercent >= 10.0){
      if (!cur) {
        GD.ColorRGB(255,0,0); // RED
      }
      GD.cmd_text(x, y+16, 30, 0, ">=10%");
    } else if (devPercent >= 1.0 && devPercent <10.0){
      int whole = (int)devPercent;
      //GD.cmd_text(x, y+16, 30, 0, "0.");
      GD.cmd_number(x, y+16, 30, 1, whole );
      GD.cmd_text(x+20, y+16, 30, 0, ".");
      GD.cmd_number(x+30, y+16, 30, 2, (devPercent - (float)whole) * 100.0);
      GD.cmd_text(x+65, y+16, 30, 0, "%");
    }
  }
}

/*
void renderVariousDummyFields(int x, int y) {

  int v, mV, uV;
  bool neg;
  
  GD.ColorRGB(200,255,200);
  GD.cmd_text(x+486, y+147, 27, 0, "Average");
  GD.ColorRGB(COLOR_VOLT);

  DIGIT_UTIL.separate(&v, &mV, &uV, &neg, averageVolt);
  if(neg) {
    GD.cmd_text(x+486 - 20, y+163, 30, 0, "-");
}
  GD.cmd_number(x+486 + 0, y+163, 30, 2, v);
  GD.cmd_text(x+486 + 32, y+163, 30, 0, ".");
  GD.cmd_number(x+486 + 40, y+163, 30, 3, mV);
  GD.cmd_number(x+486 + 96, y+163, 30, 3, uV);
//GD.cmd_text(x+486, y+163, 30, 0, "0.500 075V");
}
*/

void renderVoltageTrend() {
//  TODO make flexible....

    int x = 613;
    int y = 26;
    
    GD.ColorA(255);
    GD.ColorRGB(255,255,255);

    int v, mV, uV;
    bool neg;

//    Serial.print("minimum: ");  
//    Serial.print(V_STATS.visibleMin, 3);
//    Serial.print(", maximum: ");  
//    Serial.print(V_STATS.visibleMax, 3);

    GD.ColorRGB(200,255,200);

//    DIGIT_UTIL.separate(&v, &mV, &uV, &neg, V_STATS.span);
//    GD.cmd_text(x, y+10, 26, 0, "Span ");
//    GD.cmd_number(x+40,y+10, 26, 2, v);
//    GD.cmd_number(x+60, y+10, 26, 3, mV);
//    GD.cmd_number(x+90, y+10, 26, 3, uV);

    DIGIT_UTIL.separate(&v, &mV, &uV, &neg, V_STATS.visibleMax);
    GD.cmd_text(x, y+29, 26, 0,  "Max:");

    if(neg) {
      GD.cmd_text(x+25, y+29, 26, 0,  "-");
    }
    GD.cmd_number(x+7+25, y+29, 26, 2, v);
    GD.cmd_text(x+24+25, y+29, 26, 0,  ".");
    GD.cmd_number(x+27+25, y+29, 26, 3, mV);
    GD.cmd_number(x+57+25, y+29, 26, 3, uV);

    GD.Begin(LINE_STRIP);
    GD.LineWidth(10);
    GD.Vertex2ii(x+7, y+44); 
    GD.Vertex2ii(x+167, y+44); 
    
    V_STATS.renderTrend(x+17, y+49, true);
    //V_STATS.renderTrend(20, y+49+200, false);

    GD.Begin(RECTS);
    GD.ColorA(200); // some transparance
    GD.ColorRGB(0); 
    GD.Vertex2ii(x+30, y+60);
    GD.Vertex2ii(x+140, y+80);
    GD.ColorA(255); // No transparent
    GD.ColorRGB(COLOR_VOLT);
    DIGIT_UTIL.separate(&v, &mV, &uV, &neg, V_STATS.span);
    GD.cmd_text(x+25, y+63, 26, 0, "Span:");
    GD.cmd_number(x+40+25,y+63, 26, 2, v);
    GD.cmd_number(x+60+25, y+63, 26, 3, mV);
    GD.cmd_number(x+90+25, y+63, 26, 3, uV);

    GD.ColorRGB(200,255,200);
   
    GD.Begin(LINE_STRIP);
    GD.LineWidth(10);
    GD.Vertex2ii(x+7, y+99); 
    GD.Vertex2ii(x+167, y+99);
    
    DIGIT_UTIL.separate(&v, &mV, &uV, &neg, V_STATS.visibleMin);
    GD.cmd_text(x, y+102, 26, 0,  "Min:");
    if(neg) {
      GD.cmd_text(x+25, y+102+25, 26, 0, "-");
    }
    GD.cmd_number(x+7+25, y+102, 26, 2, v);
    GD.cmd_text(x+24+25, y+102, 26, 0, ".");
    GD.cmd_number(x+27+25, y+102, 26, 3, mV);
    GD.cmd_number(x+57+25, y+102, 26, 3, uV);



  
 
   

  
}

void currentPanel(int x, int y) {
  GD.Begin(LINE_STRIP);
  GD.LineWidth(32);
  GD.ColorRGB(50,50,0); // yellow
  GD.ColorA(230);
  GD.Vertex2ii(x+10, y+20); 
  GD.Vertex2ii(x+790, y+20);
  GD.Vertex2ii(x+790, y+200);
  GD.Vertex2ii(x+10, y+200);
  GD.Vertex2ii(x+10, y+20);

  GD.Begin(RECTS);
  GD.ColorA(255);
  GD.ColorRGB(00,00,00);
  GD.Vertex2ii(x+56, y);
  GD.Vertex2ii(x+280, y+30);

  // heading
  GD.ColorRGB(232,202,158);
  GD.cmd_text(x+56, y+5, 29, 0, "MEASURE CURRENT");

  //float rawMa = 56.0 +  random(0, 199) / 1000.0;
            float rawMa = SMU[0].MeasureCurrent() * 1000.0;

  CURRENT_DISPLAY.renderMeasured(x + 17, y, rawMa);
  CURRENT_DISPLAY.renderSet(x+120, y+135, setMa);

      //
     // C_STATS.addSample(rawMa);

    renderDeviation(x+667,y+130, rawMa, setMa, true);

}

void drawBall(int x, int y, bool set) {
  GD.PointSize(16 * 7); 
  GD.ColorRGB(255,255,255);
  GD.Begin(POINTS);
  GD.Vertex2ii(x, y);
  GD.PointSize(16 * 6);  
  if (set == true) {
    GD.ColorRGB(255,255,255); 
  } else {
    GD.ColorRGB(0,0,0); 
  }
  GD.Vertex2ii(x, y);
}

void scrollIndication(int x, int y) {
  drawBall(x,y,false);
  drawBall(x+30,y,false);
  drawBall(x+60,y,true);
  drawBall(x+90,y,false);
  drawBall(x+120,y,false);
}

  float avgVout; //TODO: fix global...
  int smoothingSamples = 10;//TODO: fix global...

void renderDisplay() {
 
  int x = 0;
  int y = 0;
  voltagePanel(x,y);
  renderVoltageTrend();
  //renderVariousDummyFields(x,y);

  currentPanel(x,y+260);

  scrollIndication(340,250);
    
  GD.cmd_fgcolor(0xaaaa90);  
  GD.Tag(BUTTON_VOLT_SET);
  GD.cmd_button(20,143,90,58,30,0,"SET"); 

  GD.Tag(BUTTON_CUR_SET);
  GD.cmd_button(20,393,90,58,30,0,"LIM");

  GD.cmd_button(350,143,90,58,30,0,"AUTO");
  GD.cmd_button(350,393,90,58,30,0,"AUTO");
}

unsigned long previousMillis = 0; 
unsigned long previousMillisSlow = 0; 
const long interval = 1; // should it account for time taken to perform ADC sample ?
void loop2()
{

  GD.ClearColorRGB(0x103000);
  GD.Clear();
  GD.cmd_text(GD.w / 2, GD.h / 2, 31, OPT_CENTER, "Hello world 2");
  GD.swap();  
         delay(10000);

}
void loop()
{
  //GD.wr(REG_PWM_DUTY, 20);

  unsigned long currentMillis = millis();

  // restore SPI
  SPI.setDataMode(SPI_MODE0);
  //SPI.setClockDivider(SPI_CLOCK_DIV2);
  GD.resume();

      if (GD.inputs.tag == BUTTON_VOLT_SET) {
      DIAL.open(BUTTON_VOLT_SET, closeCallback);
    } else if (GD.inputs.tag == BUTTON_CUR_SET) {
      DIAL.open(BUTTON_CUR_SET, closeCallback);
    }

    
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    GD.ClearColorRGB(0x000000); // black
    GD.get_inputs();
    GD.Clear();

    renderDisplay();
      Serial.print("GD.inputs ");  
      Serial.println(GD.inputs.tag);  

//    if (GD.inputs.tag == BUTTON_VOLT_SET) {
//      DIAL.open(BUTTON_VOLT_SET, closeCallback);
//    } else if (GD.inputs.tag == BUTTON_CUR_SET) {
//      DIAL.open(BUTTON_CUR_SET, closeCallback);
//    }
    if (DIAL.isDialogOpen()) {
      DIAL.checkKeypress();
      DIAL.handleKeypadDialog();
    }
  
    GD.swap();    
    GD.__end();


   /*
    // change SPI mode for other spi devices ! Needed because the gd2 lib uses spi
    SPI.setDataMode(SPI_MODE1);
    if(DAC.checkDataAvilable() == true) {
      float Vout = DAC.convertToMv();
     
      Serial.print("Vout in mV : ");  
      Serial.println(Vout, 3);

      DACVout = Vout;
      
      avgVout = DAC.smoothing(avgVout, smoothingSamples, Vout);
      //Serial.print("AvgOut in mV : ");  
      //Serial.println(avgVout, 3);
      //DACVout = avgVout;

      V_STATS.addSample(DACVout);

       delay(100);
    }
   */
      //delay(5);

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
       if (SMU[0].fltSetCommitCurrentSource(DIAL.getMv() / 1000.0,_SOURCE_AND_SINK)) printError(_PRINT_ERROR_CURRENT_SOURCE_SETTING);
       setMa = DIAL.getMv();
    }
     
}




void printError(int16_t  errorNum)
{
  Serial.print(F("Error("));
  Serial.print(errorNum);
  Serial.println(")");
}







