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

void boldNumber(int x, int y, int digits, int number) {
  GD.cmd_number(x, y ,   1, digits, number);
    GD.cmd_number(x+3, y ,   1, digits, number);
    GD.cmd_number(x, y+3 ,   1, digits, number);
    GD.cmd_number(x+3, y+3 ,   1, digits, number);
}


int COLOR_VOLT = 0x00F06E;
int COLOR_VOLTAGE_SHADDOW = 0x003C00;
int COLOR_CURRENT = 0xE8CA3A; // yellow
int COLOR_CURRENT_SHADDOW = 0x323200; // yellow shaddow
//GD.ColorRGB(20,170,255); // light blueish

void displayVoltage(int x, int y, float rawMv) {
  int v = getV(rawMv);
  int uv = getuV(rawMv);
  int mv = getmV(rawMv);

  GD.ColorRGB(COLOR_VOLTAGE_SHADDOW);
  GD.cmd_number(x+80+6, y+48 ,   1, 2, getV(rawMv));
  GD.cmd_text(x+183+6, y+48 ,   1, 0, ".");
  GD.cmd_number(x+205+6, y+48 ,   1, 3, getmV(rawMv));

  GD.ColorRGB(COLOR_VOLT);
  boldText(x+17,y+42, "+");
  boldNumber(x+80,y+42, 2, getV(rawMv));
  boldText(x+183,y+42, ".");
  boldNumber(x+205,y+42, 3, getmV(rawMv));

  GD.cmd_number(x+374, y+44, 1, 3, random(0, 299));
  //GD.cmd_number(x+374, y+44, 1, 3, getuV(rawMv));
  GD.cmd_text(x+547, y+44 ,  1, 0, "V");  
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

  float rawMv = 1501.001;
  displayVoltage(x,y , rawMv);

  // various other values
  GD.cmd_text(x+20, y+150, 31, 0, "SET   01.500 0 V");

  GD.ColorRGB(200,255,200);
  GD.cmd_text(x+486, y+147, 27, 0, "Average");
  GD.ColorRGB(COLOR_VOLT);
  GD.cmd_text(x+456+30, y+163, 30, 0, "0.500 075V");

  GD.ColorRGB(200,255,200);
  GD.cmd_text(x+667, y+147, 27, 0, "Deviation");
  GD.ColorRGB(COLOR_VOLT);
  GD.cmd_text(x+667, y+163, 30, 0, "0.005%");
  
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

int getV(float mv) {
  return mv / 1000;
}
int getmV(float mv) {
  return (mv - getV(mv)*1000);
}
int getuV(float mv) {
  return (mv - getmV(mv)) * 1000;
}

static void transButton(int x, int y, int sz, char* label)
{
//GD.Tag(label);
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
GD.cmd_text(x, y, 31, OPT_CENTER, label);
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
  GD.cmd_text(x+56, y+5 ,   29, 0, "MEASURE CURRENT");

  // main value
  GD.ColorRGB(COLOR_CURRENT_SHADDOW);
  GD.cmd_text(x+80+6, y+36 ,   1, 0, "0");
  GD.cmd_text(x+130+6, y+36 ,   1, 0, ".");
  GD.cmd_text(x+153+6, y+36 ,   1, 0, "020");

  GD.ColorRGB(COLOR_CURRENT);
  
  boldText(x+17,y+30, "+");
  boldText(x+80, y+30, "0");
  boldText(x+130, y+30, ".");
  boldText(x+153, y+30, "020");

  GD.cmd_text(x+495, y+32 ,  1, 0, "A");
  GD.cmd_number(x+322, y+32, 1, 3, random(0, 199));

  GD.ColorA(200);
  GD.cmd_text(x+20, y+135, 31, 0, "LIM   1.000 0 A");
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

void showDial() {

int screenWidth = 800;
int screenHeight = 480;
int width = 520;
int height = 450;
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
GD.LineWidth(32);
GD.Vertex2ii(startx, starty); 
GD.Vertex2ii(startx+width, starty); 
GD.Vertex2ii(startx+width, starty+height); 
GD.Vertex2ii(startx, starty+height); 
GD.Vertex2ii(startx, starty); 

int x=220;
int y=150;
int spacing = 82;
transButton(x+0, y+0,18, "1"); 
transButton(x+spacing, y+0, 18, "2");
transButton(x+spacing*2, y+0, 18, "3");
transButton(x+spacing*3, y+0, 18, " ");
transButton(x+spacing*4, y+0, 18, "Back");

transButton(x+0, y+spacing, 18, "4"); 
transButton(x+spacing, y+spacing, 18, "5"); 
transButton(x+spacing*2, y+spacing, 18, "6");
transButton(x+spacing*3, y+spacing, 18, "m");
transButton(x+spacing*4, y+spacing, 18, "Clear");

transButton(x+0, y+spacing*2, 18, "7"); 
transButton(x+spacing, y+spacing*2, 18, "8"); 
transButton(x+spacing*2, y+spacing*2, 18, "9");
transButton(x+spacing*3, y+spacing*2, 18, "u");
transButton(x+spacing*4, y+spacing*2, 18, "/10");

transButton(x+0, y+spacing*3, 18, "0"); 
transButton(x+spacing, y+spacing*3, 18, "+/-"); 
transButton(x+spacing*2, y+spacing*3, 18, ".");
transButton(x+spacing*3, y+spacing*3, 18, " ");
transButton(x+spacing*4, y+spacing*3, 18, "x10");

// entry display
GD.ColorRGB(0xaaaaaa);
GD.Begin(LINE_STRIP);
GD.LineWidth(16);
GD.Vertex2ii(startx+20, starty+20); 
GD.Vertex2ii(startx+width-20, starty+20); 
GD.Vertex2ii(startx+width-20, starty+80); 
GD.Vertex2ii(startx+20, starty+80); 
GD.Vertex2ii(startx+20, starty+20); 

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


//showDial();

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
    GD.ClearColorRGB(0x000000); // black

    GD.Clear();
    drawMainText();
    if (currentMillis - previousMillisSlow >= 10000) {
      previousMillisSlow = currentMillis;
    }
    GD.swap(); 

    GD.get_inputs();

  Serial.print(GD.inputs.x);
  Serial.print(" ");
  Serial.print(GD.inputs.y);
  Serial.print(",");
  
 
  Serial.println("");



  }
}
