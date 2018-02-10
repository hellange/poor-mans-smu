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
  //y=y+20;
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
  GD.Vertex2ii(x+260, y+50);

  GD.ColorA(255);
  GD.ColorRGB(200,255,200);
  GD.cmd_text(x+56, y+16 ,   29, 0, "SOURCE VOLTAGE");

  GD.ColorRGB(0,60,0);
  GD.cmd_text(x+86, y+48 ,   1, 0, "01.501        V");
  GD.ColorRGB(0,240,110);
  boldText(x+17,y+42, "+");
  boldText(x+80,y+42, "01.501        V");
  
  GD.cmd_number(x+377, y+44, 1, 3, random(0, 299));

  GD.cmd_text(x+20, y+150, 31, 0, "SET   01.500 0 V");

  //GD.ColorRGB(240,255,240);
  
  //GD.cmd_text(x+486, y+152, 30, 0, "DEV 0.");
  //GD.cmd_number(x+575, y+152, 30, 2,  random(0, 10));
  //GD.cmd_text(x+615, y+152, 30, 0, "%");

  GD.ColorRGB(200,255,200);
  GD.cmd_text(x+456+30, y+147, 27, 0, "Average");
  GD.ColorRGB(0+30,240,110);
  GD.cmd_text(x+456+30, y+163, 30, 0, "0.501 095V");

  GD.ColorRGB(200,255,200);
  GD.cmd_text(x+667, y+147, 27, 0, "Deviation");
  GD.ColorRGB(0+30,240,110);
  GD.cmd_text(x+667, y+163, 30, 0, "0.01%");
  
  GD.ColorRGB(200,255,200);
  GD.cmd_text(x+680, y+36, 26, 0, "Last 1 min");
  GD.cmd_text(x+620, y+55, 26, 0, "+5uV");
  GD.cmd_text(x+620, y+128, 26, 0, "-7uV");
  GD.Begin(LINE_STRIP);
  GD.LineWidth(10);
  GD.Vertex2ii(x+620, y+70); 
  GD.Vertex2ii(x+780, y+70); 
  GD.Begin(LINE_STRIP);
  GD.LineWidth(10);
  GD.Vertex2ii(x+620, y+125); 
  GD.Vertex2ii(x+780, y+125);
  

}
//
//void button(int x, int y, char* text){
//
//GD.Begin(RECTS);
//GD.ColorA(255);
//GD.ColorRGB(220,220,200);
//GD.LineWidth(10 * 16); // corner radius
//GD.Vertex2ii(x,y);
//GD.Vertex2ii(x+70, y+35);
//GD.ColorRGB(0,0,0); // yellow_orange shaddow
//GD.cmd_text(x+5,y,30,0, text);
//}


static void transButton(int x, int y, int sz, byte label)
{
GD.Tag(label);
GD.Begin(RECTS);
GD.ColorA(255);
GD.ColorRGB(200,200,200);
GD.LineWidth(16 * 20);
GD.Vertex2ii(x - sz, y - sz);
GD.Vertex2ii(x + sz, y + sz);

GD.ColorA(200);
GD.ColorRGB(255,255,255);
GD.LineWidth(16 * 15);
GD.Vertex2ii(x - sz, y - sz);
GD.Vertex2ii(x + sz, y + sz);
GD.ColorA(0xff);
GD.ColorRGB(0,0,0);
GD.cmd_number(x, y, 31, OPT_CENTER, label);
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

  GD.ColorRGB(232,202,158);
  GD.cmd_text(x+56, y+5 ,   29, 0, "MEASURE CURRENT");
  
  GD.ColorRGB(50,50,0); // yellow_orange shaddow
  //GD.ColorRGB(0,50,50); // blue shaddow
  GD.cmd_text(x+26, y+36 ,   1, 0, "+ 0.020         A");
  GD.ColorRGB(232,202,58); // yellow_orange
  //GD.ColorRGB(20,170,255); // blue
  boldText(x+20, y+30, "+ 0.020         A");
  GD.cmd_number(x+360, y+32, 1, 3, random(0, 199));

  GD.ColorA(200);
  GD.cmd_text(x+20, y+135, 31, 0, "LIM   1.000 0 A");
  GD.cmd_text(x+456, y+140, 30, 0, "PWR 5.00%");
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

void drawMainText() {


  //GD.ColorRGB(90,89,120);

  // put FT81x font 35 in slot 1
  GD.cmd_romfont(1, 34);


int x = 0;
int y = 0;
voltagePanel(x,y);
currentPanel(x,y+260);

//scrollIndication(340,20);
scrollIndication(340,250);


GD.cmd_fgcolor(0xaaaa90);
//GD.cmd_bgcolor(0x040404);

//GD.ColorRGB(255,255,255);

GD.cmd_button(20,143,90,58,30,0,"SET");
GD.cmd_button(20,393,90,58,30,0,"LIM");

GD.cmd_button(350,143,90,58,30,0,"AUTO");
GD.cmd_button(350,393,90,58,30,0,"AUTO");



/*** dial **/
/*
int screenWidth = 800;
int screenHeight = 480;
int width = 400;
int height = 400;
int margin = 10;

int startx, starty, endx, endy;
startx=(screenWidth-width) / 2;
starty=(screenHeight-height) / 2;
endx=screenWidth - (screenWidth-width) / 2;
endy=screenHeight - (screenHeight-height) / 2;

GD.ColorRGB(0x000000);
GD.Begin(RECTS);
GD.Vertex2ii(startx, starty); 
GD.Vertex2ii(endx, endy);

GD.ColorRGB(0xaaaaaa);
GD.Begin(LINE_STRIP);
GD.Vertex2ii(startx, starty); 
GD.Vertex2ii(startx+width, starty); 
GD.Vertex2ii(startx+width, starty+height); 
GD.Vertex2ii(startx, starty+height); 
GD.Vertex2ii(startx, starty); 



x=280;
y=120;
int spacing = 100;
transButton(x+0, y+0,20, 1); 
transButton(x+spacing, y+0, 20, 2);
transButton(x+spacing*2, y+0, 20, 3);
transButton(x+0, y+spacing, 20, 4); 
transButton(x+spacing, y+spacing, 20, 5); 
transButton(x+spacing*2, y+spacing, 20, 6);
transButton(x+0, y+spacing*2, 20, 7); 
transButton(x+spacing, y+spacing*2, 20, 8); 
transButton(x+spacing*2, y+spacing*2, 20, 9);
*/





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
