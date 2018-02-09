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
 *  Reduced and modified to
 *  -compile om 8266
 *  -support 800x480 pixels for all graphics
 * 
 ************************************************/

#include <SPI.h>
#include "GD2.h"
#include "Wire.h" 

void setup()
{
  //wdt_disable();
  Serial.begin(9600);
  Serial.println("Initializing WeatherNG graphics controller FT81x...");
  GD.begin(0);
  Serial.println("Done!");


}

void boldText(int x, int y, char* text) {
  GD.cmd_text(x, y ,   1, 0, text);
    GD.cmd_text(x+3, y ,   1, 0, text);
    GD.cmd_text(x, y+3 ,   1, 0, text);
    GD.cmd_text(x+3, y+3 ,   1, 0, text);
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

GD.Begin(RECTS);
GD.ColorA(255);
GD.ColorRGB(00,00,00);
//GD.LineWidth(10 * 16); // corner radius 20.0 pixels
GD.Vertex2ii(x+56, y+20);
GD.Vertex2ii(x+320, y+50);


    GD.ColorA(255);

  GD.ColorRGB(200,255,200);
      GD.cmd_text(x+56, y+15 ,   30, 0, "SOURCE VOLTAGE");



  GD.ColorRGB(0,40,0);
      GD.cmd_text(x+56, y+51 ,   1, 0, "+ 01.501        V");
       GD.ColorRGB(0,240,110);

  boldText(x+50,y+45, "+ 01.501        V");
  


  GD.cmd_number(x+425, y+47, 1, 3, random(0, 299));

  GD.cmd_text(x+50, y+150, 31, 0, "SET   01.500 0 V");

  //GD.ColorRGB(240,255,240);
  GD.cmd_text(x+486, y+155, 30, 0, "DEV 0.");
  GD.cmd_number(x+575, y+155, 30, 2,  random(0, 10));
  GD.cmd_text(x+615, y+155, 30, 0, "%");


}

void button(int x, int y, char* text){

GD.Begin(RECTS);
GD.ColorA(255);
GD.ColorRGB(220,220,200);
GD.LineWidth(10 * 16); // corner radius
GD.Vertex2ii(x,y);
GD.Vertex2ii(x+70, y+35);
GD.ColorRGB(0,0,0); // yellow_orange shaddow
GD.cmd_text(x+5,y,30,0, text);
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
GD.Vertex2ii(x+340, y+30);

  GD.ColorRGB(232,202,158);
  
  GD.cmd_text(x+56, y ,   30, 0, "MEASURE CURRENT");


  GD.ColorRGB(50,50,0); // yellow_orange shaddow
  //GD.ColorRGB(0,50,50); // blue shaddow

  GD.cmd_text(x+56, y+36 ,   1, 0, "+ 0.020         A");
  
  GD.ColorRGB(232,202,58); // yellow_orange
  //GD.ColorRGB(20,170,255); // blue

  boldText(x+50, y+30, "+ 0.020         A");



  GD.cmd_number(x+390, y+32, 1, 3, random(0, 199));

  GD.ColorA(200);
  GD.cmd_text(x+50, y+135, 31, 0, "LIM   1.000 0 A");

  GD.cmd_text(x+486, y+140, 30, 0, "PWR 5.00%");

}

void drawBall(int x, int y, bool set) {
  GD.PointSize(16 * 10); // means 30 pixels
  GD.ColorRGB(255,255,255);
  GD.Begin(POINTS);
  GD.Vertex2ii(x, y);
  GD.PointSize(16 * 7); // means 30 pixels
  if (set == true) {
      GD.ColorRGB(255,255,255); 
  } else {
      GD.ColorRGB(0,0,0); 
  }
  GD.Vertex2ii(x, y);
}
void drawMainText() {

 
  //GD.ColorRGB(90,89,120);

  // put FT81x font 35 in slot 1
  GD.cmd_romfont(1, 34);


int x = 0;
int y = 0;
voltagePanel(x,y);
currentPanel(x,y+260);

drawBall(340,240,false);
drawBall(370,240,false);
drawBall(400,240,true);
drawBall(430,240,false);
drawBall(460,240,false);


button(60, 155,"SET");

button(60, 400,"LIM");
 

}

unsigned long previousMillis = 0; 
unsigned long previousMillisSlow = 0; 

const long interval = 100; 
void loop()
{
//GD.wr(REG_PWM_DUTY, 20);

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    //GD.ClearColorRGB(0x205000);  // greenish
    GD.ClearColorRGB(0x000000); // black

    GD.Clear();
    drawMainText();
    if (currentMillis - previousMillisSlow >= 10000) {
      previousMillisSlow = currentMillis;
    }
    GD.swap();   
  }
}
