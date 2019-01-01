#include "current_display.h"
#include "GD2.h"
#include "colors.h"
#include "digit_util.h"

void CurrentDisplayClass::renderMeasured(int x, int y, float rawMa) {

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
//  GD.cmd_number(x+80+6-17, y+36 ,   1, 1, a);
//  GD.cmd_text(x+130+6-17, y+36 ,   1, 0, ".");
//  GD.cmd_number(x+153+6-17, y+36 ,   1, 3, ma);
  
  GD.ColorRGB(COLOR_CURRENT);

  if (a>0) {

    // TODO: Preliminary dont show sign. Set current goes for both pos and neg (sing/source). Preliminary?
    boldText(x,y+30, sign);
    
    boldNumber(x+63, y+30, 1, a);
    boldText(x+113, y+30, ".");
    boldNumber(x+136, y+30, 3, ma);

    GD.cmd_number(x+305, y+32, 1, 3, ua);
    GD.cmd_text(x+478, y+32 ,  1, 0, "A");
  } else {
    boldText(x,y+30, sign);

    boldNumber(x+63, y+30, 3, ma);
    boldText(x+305-50-30, y+30, ".");
    GD.cmd_number(x+305-50, y+32, 1, 3, ua);
    GD.cmd_text(x+478-50, y+32 ,  1, 0, "mA");
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
