#include "volt_display.h"
#include <SPI.h>
#include "GD2.h"
#include "colors.h"
#include "digit_util.h"

void VoltDisplayClass::renderMeasured(int x, int y, float rawMv, bool compliance, bool reduceDetails) {

  int v, mv, uv;
  bool neg;
  DIGIT_UTIL.separate(&v, &mv, &uv, &neg, rawMv);

  int complianceColor = DIGIT_UTIL.blinkColor(0xff4522, 0x991002, 1000);

  GD.ColorA(255);

  compliance ? GD.ColorRGB(complianceColor) : GD.ColorRGB(COLOR_VOLT);
  
  GD.cmd_text(x, y,  1, 0, neg ? "-":"+");
  x = x + 55;
 
  if ( (v==0 && mv > 99) || v>0 ) {
    boldNumber(x,y, 2, v);
    boldText(x+104,y, ".");
    boldNumber(x+130,y, 3, mv);
  
    //GD.ColorA(100);
    //GD.cmd_number(x+307, y+2, 1, 3, uv);
    GD.ColorA(255);
    GD.cmd_number(x + 307, y + 2, 1, 2, uv/10.0);

    // show last digit in uV. Note that the HW is NOT designed with that precision !!!!
    GD.ColorA(100);
    GD.cmd_number(x + 417, y + 50, 31, 1, (uv/10.0 - ((int)(uv/10.0)))*10.0);

    GD.ColorA(255);

    GD.cmd_text(x + 440, y+2,  1, 0, "V"); 
  } else {
    boldNumber(x + 4,y, 3, mv);
    boldText(x + 164,y, ".");
    GD.ColorA(100);
    GD.cmd_number(x+194, y+2, 1, 3, uv);
    GD.ColorA(255);
    GD.cmd_number(x+194, y+2, 1, 2, uv/10.0);
    GD.cmd_text(x+360, y+2,  1, 0, "mV"); 
  }
 GD.ColorRGB(COLOR_VOLT);
}

//TODO: Resistance should be moved to another place...
void VoltDisplayClass::renderMeasuredResistance(int x, int y, float rawMv, float rawMa, bool compliance) {

  float ohm = rawMv / rawMa;
 // int v, mv, uv;
 // bool neg;
 // DIGIT_UTIL.separate(&v, &mv, &uv, &neg, ohm * 1000.0);

  int complianceColor = DIGIT_UTIL.blinkColor(0xff4522, 0x991002, 1000);

  GD.ColorA(255);
  compliance ? GD.ColorRGB(complianceColor) : GD.ColorRGB(COLOR_VOLT);

  x = x + 55;

    GD.cmd_number(x + 194, y+2, 1, 8, ohm);
/*
  if (ohm >= 1000000) {
    GD.cmd_number(x + 194, y+2, 1, 3, ohm/1000000.0);
    GD.cmd_text(x + 360, y+2,  1, 0, "MOhm");

  //GD.cmd_number(x + 194, y+2, 1, 8, 9999);
  }
  else if (ohm > 100000) {
    GD.cmd_number(x + 194, y+2, 1, 3, ohm/1000.0);
    GD.cmd_text(x + 360, y+2,  1, 0, "KOhm");
  } 
  else if (ohm < 999.0) {
    int v, mv, uv;
    bool neg;
    DIGIT_UTIL.separate(&v, &mv, &uv, &neg, ohm * 1000.0);
    boldNumber(x + 4,y, 3, ohm);
    boldText(x+164,y, ".");
    GD.cmd_number(x + 194, y+2, 1, 3, mv);
    GD.cmd_text(x + 360, y+2,  1, 0, "Ohm");
  } else {
    int v, mv, uv;
    bool neg;
    DIGIT_UTIL.separate(&v, &mv, &uv, &neg, ohm * 1000.0);
    boldNumber(x+104-100,y, 5, ohm);
    boldText(x+164+100,y, ".");
    GD.cmd_number(x + 294, y+2 , 1, 2, mv/10.0);
    GD.cmd_text(x + 410, y+2,  1, 0, "Ohm");
  }
  */
  GD.ColorRGB(COLOR_VOLT);
}

void VoltDisplayClass::renderSet(int x, int y, int64_t raw_uV) {

  float rawMv = raw_uV/1000.0;
  int v, mv, uv, nV;
  bool neg;

  DIGIT_UTIL.separate2(&v, &mv, &uv, &nV, &neg, rawMv);
 
  v = abs(raw_uV) / 1000 / 1000;

  float rest_uV = abs(raw_uV) - v * 1000 * 1000;

  mv = rest_uV / 1000;

  rest_uV = abs(raw_uV) - mv * 1000;

  uv = rest_uV;
  /*
  DEBUG.print("renderSet ramMv=");
  DEBUG.print("v=");
  DEBUG.print(v);
  DEBUG.print(", mV=");
  DEBUG.print(mv);
  DEBUG.print(", uV=");
  DEBUG.println(uv);
  */

  GD.cmd_text(x, y, 31, 3, neg ? "-" : "+");

  GD.cmd_number(x + 25, y, 31, 2, v);
  GD.cmd_number(x + 85, y, 31, 3, mv);
  GD.cmd_text(x + 73, y, 31, 0, ".");
  //GD.cmd_number(x+165, y, 31, 1, uv / 100); // need to do it like this to show only first digit in a three digit value
  GD.cmd_number(x + 165, y, 31, 2, uv / 10); // need to do it like this to show only two first digit in a three digit value

  GD.cmd_text(x + 215, y, 31, 0, "V");
}

void VoltDisplayClass::boldText(int x, int y, const char* text) {
  GD.cmd_text(x, y, 1, 0, text);
  GD.cmd_text(x + 3, y, 1, 0, text);
  GD.cmd_text(x, y + 3, 1, 0, text);
  GD.cmd_text(x+3, y + 3 , 1, 0, text);
}

void VoltDisplayClass::boldNumber(int x, int y, int digits, int number) {
  GD.cmd_number(x, y , 1, digits, number);
  GD.cmd_number(x + 3, y , 1, digits, number);
  GD.cmd_number(x, y + 3 , 1, digits, number);
  GD.cmd_number(x + 3, y + 3 , 1, digits, number);
}

VoltDisplayClass VOLT_DISPLAY;
