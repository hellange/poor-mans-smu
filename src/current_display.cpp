#include "current_display.h"
#include <SPI.h>
#include "GD2.h"
#include "colors.h"
#include "digit_util.h"
#include "Debug.h"


void CurrentDisplayClass::renderOverflowSW(int x, int y) {
  GD.ColorRGB(COLOR_CURRENT);
  boldText(x, y, " Overflow A (sw)");
}

void CurrentDisplayClass::renderMeasured(int x, int y, float rawMa, boolean compliance, bool show_nA, CURRENT_RANGE current_range, bool reduceDetails) {
 
  int complianceColor = DIGIT_UTIL.blinkColor(0xff4522, 0x991002, 1000);

  int a, ma, ua, na;
  bool neg;

  DIGIT_UTIL.separate2(&a, &ma, &ua, &na, &neg, rawMa);

  compliance ? GD.ColorRGB(complianceColor) : GD.ColorRGB(COLOR_CURRENT);

  GD.cmd_text(x, y,  1, 0, neg ? "-" : "+");

  x = x + 55;
  
  if (a > 0) {
    boldNumber(x, y, 1, a);
    boldText(x + 50, y, ".");
    boldNumber(x + 70, y, 3, ma);
    GD.cmd_number(x + 242, y + 2, 1, 3, ua);
    GD.cmd_text(x + 410, y + 2 ,  1, 0, "A");
  } else if (ma > 0 or (ma==0 &&  ua>100 && show_nA==false) or show_nA == false) {

    if (current_range == MILLIAMP10) {
      boldNumber(x, y, 2, ma); // dont show most significant digit because ma<100
      x=x-50; // can the move rest od the displayed text to the left
    } else {
      boldNumber(x, y, 3, ma);
    }

    boldText(x+162, y, ".");
    GD.cmd_number(x + 192, y + 2, 1, 3, ua);
  
    if (current_range == MILLIAMP10) {
      GD.cmd_number(x + 370, y + 2, 1, 2, (int)(na / 10.0)); // use only two digit for nano amps !
      GD.cmd_text(x + 480, y + 2 ,  1, 0, "mA");
    } else {
      GD.cmd_number(x + 370, y+2, 1, 1, (int)(na / 100.0)); // use only one digit for nano amps !
      GD.cmd_text(x + 430, y+2 ,  1, 0, "mA");
    }
    //GD.cmd_number(x+370, y+2, 1, 1, (int)(na/100.0)); // use only one digit for nano amps !
    //GD.cmd_text(x+470-40, y+2 ,  1, 0, "mA");
  } else if (show_nA) {
    // Show uA with nA decimals...
    boldNumber(x, y, 3, ua);
    boldText(x+162, y+30-30, ".");
     
    GD.cmd_number(x+192, y+2, 1, 2, (int)(na/10.0)); // use only two digit for nano amps !
    //GD.cmd_number(x+192, y+2, 1, 3, na); // show three digits in nA

    GD.cmd_text(x+320, y+2 ,  1, 0, "uA");
    //GD.cmd_number(x+192, y+2, 1, 3, (int)(na)); 
    //GD.cmd_text(x+355, y+2 ,  1, 0, "uA");
  }
  
}

void CurrentDisplayClass::renderSet(int x, int y, int64_t raw_uA, CURRENT_RANGE current_range) {
  
  float rawMa = raw_uA / 1000.0;
  int a, ma, ua, na;
  bool neg;

  DIGIT_UTIL.separate64t(&a, &ma, &ua, &na, &neg, rawMa);

  a = abs(raw_uA) / 1000 / 1000;

  float rest_uA = abs(raw_uA) - a * 1000 * 1000;

  ma = rest_uA / 1000;

  rest_uA = abs(raw_uA) - ma * 1000;

  ua = rest_uA;

  /*
  DEBUG.print("renderSet rawuA=");
  DEBUG.print("a=");
  DEBUG.print(a);
  DEBUG.print(", mA=");
  DEBUG.print(ma);
  DEBUG.print(", uA=");
  DEBUG.println(ua);
  */

  //GD.ColorRGB(COLOR_CURRENT);
  //DEBUG.print("RENDER SET:");
  //DEBUG.print(rawMa);

  if (rawMa < 0.0) {
    GD.cmd_text(x, y, 31, 0, "-");
    x = x + 20;
  }
  if (rawMa >= 0.0) {
    x = x + 20;
  }
  if (current_range == AMP1) {
  //if (abs(rawMa) >= 10.0) {
    GD.cmd_number(x + 5, y, 31, 1, a);
    GD.cmd_text(x + 30, y, 31, 0, ".");
    GD.cmd_number(x + 40, y, 31, 3, ma);
    //GD.cmd_number(x + 120, y, 31, 1, ua / 100); // need to do it like this to show only first digit in a three digit value
    GD.cmd_number(x + 120, y, 31, 2, ua / 10); // need to do it like this to show only first two digit in a three digit value

    GD.cmd_text(x + 165, y, 31, 0, "A");
  } else {
    GD.cmd_number(x + 5, y, 31, 2, ma);
    GD.cmd_text(x + 50, y, 31, 0, ".");
    GD.cmd_number(x + 60, y, 31, 3, ua);
    
    //  Don't show nA for now...
    GD.cmd_number(x+136, y, 31, 1, na/100); // need to do it like this to show only first digit in a three digit value
    GD.cmd_text(x+160, y, 31, 0, "mA");

   // GD.cmd_text(x+140, y, 31, 0, "mA");
  }
  // } else {
  //   GD.cmd_number(x+25-20, y, 31, 1, ma);
  //   GD.cmd_text(x+50-20, y, 31, 0, ".");
  //   GD.cmd_number(x+60-20, y, 31, 3, ua);
  //   //GD.cmd_number(x+140-20, y, 31, 1, ua / 100); // need to do it like this to show only first digit in a three digit value
 
  //   GD.cmd_text(x+170-20+15, y, 31, 0, "mA");
  // }
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
