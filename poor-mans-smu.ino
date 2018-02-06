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

void drawMainText() {

  //GD.ColorRGB(90,89,120);

  // put FT81x font 35 in slot 1
  GD.cmd_romfont(1, 34);

  
//GD.Begin(LINES);
GD.Begin(LINE_STRIP);
GD.LineWidth(16);
GD.ColorRGB(0,255,150);
GD.Vertex2ii(10, 30); 
GD.Vertex2ii(690, 30);
GD.Vertex2ii(690, 200);
GD.Vertex2ii(10, 200);
GD.Vertex2ii(10, 30);

  
  GD.ColorRGB(0,255,150);
  GD.ColorA(255);
  GD.cmd_text(50, 50 ,   1, 0, "+ 01.501        V");
    GD.cmd_number(430, 50, 1, 3, random(0, 299));

 GD.ColorRGB(0,255,150);
  GD.ColorA(200);

GD.Begin(RECTS);

GD.LineWidth(10 * 16); // corner radius 20.0 pixels
GD.Vertex2ii(40, 160);
GD.Vertex2ii(410, 185);

  GD.ColorA(255);

   GD.ColorRGB(90,89,120);

  GD.cmd_text(50, 150, 31, 0, "SET     01.500 0 V");

  GD.ColorRGB(232,202,58);
  GD.ColorA(200);

  GD.cmd_text(550, 150, 31, 0, "%0.");
  GD.cmd_number(625, 150, 31, 2,  random(0, 20));


  GD.Begin(LINE_STRIP);
GD.LineWidth(16);
GD.ColorRGB(0,150,255);
GD.Vertex2ii(10, 230); 
GD.Vertex2ii(690, 230);
GD.Vertex2ii(690, 400);
GD.Vertex2ii(10, 400);
GD.Vertex2ii(10, 230);


  GD.ColorRGB(232,202,58);
  GD.ColorA(255);
  GD.cmd_text(50, 250 ,   1, 0, "+ 0.020         A");
  GD.cmd_number(390, 250, 1, 3, random(0, 199));

 GD.ColorRGB(0,150,255);
  GD.ColorA(200);
  GD.cmd_text(50, 350, 31, 0, "LIM     1.000 000 A");


  
  GD.ColorRGB(50,50,50);
  GD.ColorA(255);
  GD.cmd_text(10, 440, 28, 0, "Poor Mans Source Measure Unit");
  GD.cmd_text(10, 460, 27, 0, "by Helge Langehaug");

}

unsigned long previousMillis = 0; 
unsigned long previousMillisSlow = 0; 

const long interval = 100; 
void loop()
{

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    GD.ClearColorRGB(30, 30, 30);
    GD.Clear();
    drawMainText();
    if (currentMillis - previousMillisSlow >= 10000) {
      previousMillisSlow = currentMillis;
    }
    GD.swap();   
  }
}
