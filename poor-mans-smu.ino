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
//#include "LT_I2C.h"
#include "Wire.h"
//#include "LTC2485.h"
//#include "LTC2655.h"
//#include "EasySMU.h"
#include "EasySMU2.h"


#define GEST_NONE 0
#define GEST_MOVE_LEFT 1
#define GEST_MOVE_RIGHT 2


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

#define _SOURCE_AND_SINK 99
EasySMU2 SMU[1] = {
      EasySMU2()
};

//EasySMU SMU[1] =
//{
//    EasySMU(I2C_Board0_EEPROM_ADDR0, I2C_Board0_LTC2655_LLL_ADDR0, I2C_Board0_LTC2485_Vout_LF_ADDR0, I2C_Board0_LTC2485_Iout_LH_ADDR0)
//};

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

//   i2c_enable(); //Set Linduino default I2C speed instead of Wire default settings.
  /*
    pinMode(QUIKEVAL_MUX_MODE_PIN, OUTPUT);   //Set Linduino MUX pin to disable the I2C MUX. Otherwise it can cause unpredictable behavior.
    digitalWrite(QUIKEVAL_MUX_MODE_PIN, LOW);
  
    SMU[0].ReadCalibration();
    setMv = 0.0;
    setMa = 10.0;
    SMU[0].fltSetCommitCurrentSource(setMa / 1000.0,_SOURCE_AND_SINK);
    SMU[0].fltSetCommitVoltageSource(setMv / 1000.0);
    SMU[0].EnableOutput();

*/

 
  GD.cmd_romfont(1, 34); // put FT81x font 34 in slot 1
  //GD.wr(REG_PWM_DUTY, 10);

  
  
}

void voltagePanel(int x, int y) {
  //GD.ColorA(70);
  /*
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
  */

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

  GD.ColorRGB(0,0,0);

  GD.cmd_fgcolor(0xaaaa90);  
  GD.Tag(BUTTON_VOLT_SET);
  GD.cmd_button(x+20,y+143,90,58,30,0,"SET"); 
  GD.cmd_button(x+350,y+143,90,58,30,0,"AUTO");
  

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

void renderValue(int x,int y,float val, int size = 0) {

    int font = 26;
    int fontWidth = 10; 
    
    if (size == 1) {
       font = 28;
       fontWidth = 15;
    }
    else if (size == 2) {
       font = 29;
       fontWidth = 18;
    }
    else if (size == 3) {
       font = 30;
       fontWidth = 21;
    }
    else if (size == 4) {
       font = 31;
       fontWidth = 29;
    }
    
    int v, mV, uV;
    bool neg;
    DIGIT_UTIL.separate(&v, &mV, &uV, &neg, val);
    if(neg) {
      GD.cmd_text(x, y, 28, 0,  "-");
    }

    
    x=x+fontWidth;
    if (v>0) {
      GD.cmd_number(x, y, font, 2, v);
      x = x + fontWidth*1.7;
      GD.cmd_text(x, y, font, 0,  ".");
      x = x + fontWidth/3;
      GD.cmd_number(x, y, font, 3, mV);
      x = x + fontWidth * 2.9;
      GD.cmd_number(x, y, font, 3, uV);
      x = x + fontWidth * 2.6;
      GD.cmd_text(x, y, font, 0,  "V");
    } else {
      GD.cmd_number(x, y, font, 3, mV);
      x = x + fontWidth*2.5;
      GD.cmd_text(x, y, font, 0,  ".");
      x = x + fontWidth/3;
      GD.cmd_number(x, y, font, 3, uV);
      x = x + fontWidth * 2.6;
      GD.cmd_text(x, y, font, 0,  "mV");
    }

}



void renderGraph(int x,int y) {

    GD.ColorRGB(0xffffff); 

    float span = V_STATS.visibleMax - V_STATS.visibleMin;
    renderValue(x, y, V_STATS.visibleMax, 1);
    renderValue(x, y+50, V_STATS.visibleMax - (span/3.0), 1);
    renderValue(x, y+100, V_STATS.visibleMax- 2.0*(span/3.0), 1);
    renderValue(x, y+150, V_STATS.visibleMin, 1);

    y = y + 15;

    GD.ColorRGB(COLOR_VOLT); // yellow

    V_STATS.renderTrend(x + 180, y, false);

    int left = x + 150;
    if (left > 800) {
      return;
    }
    int right = x + 800;
    if (right > 800) {
      right = 800;
    }

    GD.LineWidth(6);
    GD.ColorRGB(0xffffff); 

    GD.Begin(LINE_STRIP);
    GD.Vertex2ii(left, y); 
    GD.Vertex2ii(right, y); 

    GD.Begin(LINE_STRIP);
    //GD.LineWidth(2);
    GD.Vertex2ii(left, y+50); 
    GD.Vertex2ii(right, y+50); 

    GD.Begin(LINE_STRIP);
    //GD.LineWidth(2);
    GD.Vertex2ii(left, y+100); 
    GD.Vertex2ii(right, y+100); 

    GD.Begin(LINE_STRIP);
    //GD.LineWidth(2);
    GD.Vertex2ii(left, y+150); 
    GD.Vertex2ii(right, y+150); 
    
}

void renderVoltageTrend() {
//  TODO make flexible....

    int x = 613;
    int y = 26;
    
    //GD.ColorA(255);
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

    //DIGIT_UTIL.separate(&v, &mV, &uV, &neg, V_STATS.visibleMax);
    
    GD.cmd_text(x, y+29, 26, 0,  "Max:");
    renderValue(x+25, y+29, V_STATS.visibleMax);

    GD.Begin(LINE_STRIP);
    GD.LineWidth(10);
    GD.Vertex2ii(x+7, y+44); 
    GD.Vertex2ii(x+167, y+44); 
    
    V_STATS.renderTrend(x+17, y+49, true);


    GD.Begin(RECTS);
    //GD.ColorA(200); // some transparance
    GD.ColorRGB(0); 
    GD.Vertex2ii(x+30, y+60);
    GD.Vertex2ii(x+140, y+80);
    //GD.ColorA(255); // No transparent
    GD.ColorRGB(COLOR_VOLT);
    //DIGIT_UTIL.separate(&v, &mV, &uV, &neg, V_STATS.span);
    GD.cmd_text(x+25, y+63, 26, 0, "Span:");
    renderValue(x+40+25, y+63, V_STATS.span);
    
    GD.ColorRGB(200,255,200);
   
    GD.Begin(LINE_STRIP);
    GD.LineWidth(10);
    GD.Vertex2ii(x+7, y+99); 
    GD.Vertex2ii(x+167, y+99);
    
    DIGIT_UTIL.separate(&v, &mV, &uV, &neg, V_STATS.visibleMin);
    GD.cmd_text(x, y+102, 26, 0,  "Min:");
    renderValue(x+25, y+102, V_STATS.visibleMin);
}

void currentPanel(int x, int y) {
  if (x >= 800) {
    return;
  }
  GD.Begin(LINE_STRIP);
  GD.LineWidth(32);
  GD.ColorRGB(50,50,0); // yellow
  /*
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
*/
  // heading
  GD.ColorRGB(232,202,158);
  GD.cmd_text(x+56, y+5, 29, 0, "MEASURE CURRENT");

  //float rawMa = 56.0 +  random(0, 199) / 1000.0;
  float rawMa = SMU[0].MeasureCurrent() * 1000.0;

  CURRENT_DISPLAY.renderMeasured(x + 17, y, rawMa);
  CURRENT_DISPLAY.renderSet(x+120, y+135, setMa);

  //
  // C_STATS.addSample(rawMa);

  GD.ColorRGB(0,0,0);

  GD.cmd_fgcolor(0xaaaa90);  
  GD.Tag(BUTTON_CUR_SET);
  GD.cmd_button(x+20,y+130,90,58,30,0,"LIM"); 
  GD.cmd_button(x+350,y+130,90,58,30,0,"AUTO");
  
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




int scroll = 0;
int scrollDir = 0;

int gestureDetected = GEST_NONE;

void renderDisplay() {
 
  int x = 0;
  int y = 0;

  voltagePanel(x,y);
  renderVoltageTrend();

  if (gestureDetected == GEST_MOVE_LEFT) {
    scrollDir = -1;
  } 
  else if (gestureDetected == GEST_MOVE_RIGHT) {
    scrollDir = 1;
  } 

  if (scroll <= -800 && scrollDir == -1) {
    Serial.println("reached left end");
  }
  else if (scroll >= 0 && scrollDir == 1) {
    Serial.println("reached right end");
  } else {
      scroll = scroll + scrollDir * 100;

  }
  
  
  if (scroll <= -800) {
    scrollDir = 0;
  } else if (scroll >= 0) {
    scrollDir = 0;
  }

    GD.Tag(101);
  GD.Begin(RECTS);
  GD.ColorRGB(00,00,00);
  GD.Vertex2ii(0,260);
  GD.Vertex2ii(800, 480);

  
  currentPanel(scroll + 800,y+260);
  renderGraph(scroll, 260);
  


  


  scrollIndication(340,250);
    

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




int oldTouchX = 0;
int times = 0;
void detectGestures() {
  GD.get_inputs();
  //Serial.println(GD.inputs.tag);
  int touchX = GD.inputs.x;

  if (GD.inputs.tag == 101) {
    if (gestureDetected == GEST_NONE && touchX > 0 && touchX - oldTouchX < -15 && scrollDir == 0) {
      if (++times >= 2) {
        Serial.println("move left");
        gestureDetected = GEST_MOVE_LEFT;
        times = 0;
      }
    }
    else if (gestureDetected == GEST_NONE && touchX > 0 && touchX - oldTouchX > 15 && scrollDir == 0) {
      if (++times >= 2) {
        Serial.println("move right");
        gestureDetected = GEST_MOVE_RIGHT;
        times = 0;
      }
    } else {
      times = 0;
    }
    oldTouchX = GD.inputs.x;  
  } else {
    gestureDetected = GEST_NONE;
  }

}


void loop()
{

  unsigned long currentMillis = millis();

  // restore SPI
  SPI.setDataMode(SPI_MODE0);
  //SPI.setClockDivider(SPI_CLOCK_DIV2);
  GD.resume();

  if (!gestureDetected) {
    if (GD.inputs.tag == BUTTON_VOLT_SET) {
      DIAL.open(BUTTON_VOLT_SET, closeCallback);
    } else if (GD.inputs.tag == BUTTON_CUR_SET) {
      DIAL.open(BUTTON_CUR_SET, closeCallback);
    }
  }


  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    //GD.ClearColorRGB(0x000000); // black
    GD.get_inputs();
    GD.Clear();

    if (DIAL.isDialogOpen()) {
      GD.ColorA(0x44);
    }
    renderDisplay();
    // Serial.print("GD.inputs ");  
    // Serial.println(GD.inputs.tag);  

//    if (GD.inputs.tag == BUTTON_VOLT_SET) {
//      DIAL.open(BUTTON_VOLT_SET, closeCallback);
//    } else if (GD.inputs.tag == BUTTON_CUR_SET) {
//      DIAL.open(BUTTON_CUR_SET, closeCallback);
//    }
    if (DIAL.isDialogOpen()) {
      DIAL.checkKeypress();
      DIAL.handleKeypadDialog();
    }

  detectGestures();


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







