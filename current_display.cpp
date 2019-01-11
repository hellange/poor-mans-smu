#include "current_display.h"
#include <SPI.h>
#include "GD2.h"
#include "colors.h"
#include "digit_util.h"

void CurrentDisplayClass::renderMeasured(int x, int y, float rawMa, boolean overflow) {

  if (rawMa < 0.0f) {
    rawMa = 0.0f - rawMa;
    sign[0] = '-';
  } else {
    sign[0] = '+';
  }
  
  int a = rawMa / 1000;
  int ma = (rawMa - a * 1000);
  int ua = (rawMa - ma) * 1000;
  
//  GD.ColorRGB(COLOR_CURRENT_SHADDOW);
//  GD.cmd_number(x+80+6-17, y+36-30 ,   1, 1, a);
//  GD.cmd_text(x+130+6-17, y+36-30 ,   1, 0, ".");
//  GD.cmd_number(x+153+6-17, y+36-30 ,   1, 3, ma);
  
  if (overflow) {
    GD.ColorRGB(0xFF4500); //0xdd1120
    
  } else {
    GD.ColorRGB(COLOR_CURRENT);
  }
  GD.ColorA(255);


    GD.cmd_text(x, y,  1, 0, sign);
    //boldText(x,y, sign);
  x=x+55;
  
  if (a>0) {
    boldNumber(x, y, 1, a);
    boldText(x+50, y, ".");
    boldNumber(x+70, y, 3, ma);
    GD.cmd_number(x+242, y+2, 1, 3, ua);
    GD.cmd_text(x+410, y+2 ,  1, 0, "A");
  } else {
    boldNumber(x, y, 3, ma);
    boldText(x+162, y+30-30, ".");
    GD.cmd_number(x+192, y+2, 1, 3, ua);
    GD.cmd_text(x+360, y+2 ,  1, 0, "mA");
  }


  
}

void CurrentDisplayClass::renderSet(int x, int y, float rawMa) {
  
  int a, ma, ua;
  bool neg;

  DIGIT_UTIL.separate(&a, &ma, &ua, &neg, rawMa);

    // Currently dont show sign for current. Positive value is used both for neg and pos (sink/source). Preliminary ?
//    if (neg) {
//        GD.cmd_text(x, y, 31, 3, "-");
//    } else {
//        GD.cmd_text(x, y, 31, 3, "+");
//    }
  
  GD.ColorRGB(COLOR_CURRENT);

  GD.cmd_number(x+25, y, 31, 1, a);
  GD.cmd_text(x+48, y, 31, 0, ".");
  GD.cmd_number(x+60, y, 31, 3, ma);
  GD.cmd_number(x+140, y, 31, 1, ua / 100); // need to do it like this to show only first digit in a three digit value
  GD.cmd_text(x+170, y, 31, 0, "A");
}

void CurrentDisplayClass::boldText(int x, int y, const char* text) {
  GD.cmd_text(x, y, 1, 0, text);
  GD.cmd_text(x+3, y, 1, 0, text);
  GD.cmd_text(x, y+3, 1, 0, text);
  GD.cmd_text(x+3, y+3 , 1, 0, text);
}

void CurrentDisplayClass::boldNumber(int x, int y, int digits, int number) {
  GD.cmd_number(x, y , 1, digits, number);
  GD.cmd_number(x+3, y , 1, digits, number);
  GD.cmd_number(x, y+3 , 1, digits, number);
  GD.cmd_number(x+3, y+3 , 1, digits, number);
}

CurrentDisplayClass CURRENT_DISPLAY;
