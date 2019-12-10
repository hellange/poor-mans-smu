#include "current_display.h"
#include <SPI.h>
#include "GD2.h"
#include "colors.h"
#include "digit_util.h"

unsigned long compliance_blink_timer = millis();
unsigned long old_compliance_blink_timer = compliance_blink_timer ;

int blinkColor(int colorHigh, int colorLow, int period) {
  int compliance_blink_timer = millis();
  int color = colorHigh;
  if (compliance_blink_timer - old_compliance_blink_timer > period) {
    old_compliance_blink_timer = compliance_blink_timer;
  } else if (compliance_blink_timer - old_compliance_blink_timer > period/2) {
    color = colorLow;
  }
  return color;
}

void CurrentDisplayClass::renderOverflow(int x, int y) {
  GD.ColorRGB(COLOR_CURRENT);
  boldText(x, y, " Overflow A");
}
void CurrentDisplayClass::renderMeasured(int x, int y, float rawMa, boolean compliance) {
 
  int complianceColor = blinkColor(0xff4522, 0x991002, 1000);
  
  
//  // blink
//  if (compliance_blink_timer - old_compliance_blink_timer > 1000) {
//    old_compliance_blink_timer = compliance_blink_timer;
//    color = 0xff4522;
//  } else if (compliance_blink_timer - old_compliance_blink_timer > 500) {
//    color = 0x991002;
//  }
   

  int a, ma, ua, na;
  bool neg;
  bool uA_mode_enabled = false; // set true if you want uA scale
  DIGIT_UTIL.separate2(&a, &ma, &ua, &na, &neg, rawMa);
  
  if (compliance) {
    GD.ColorRGB(complianceColor); //0xFF4522); //0xdd1120
  } else {
    GD.ColorRGB(COLOR_CURRENT);
  }

  GD.cmd_text(x, y,  1, 0, neg ? "-":"+");
  x=x+55;
  
  if (a>0) {
    boldNumber(x, y, 1, a);
    boldText(x+50, y, ".");
    boldNumber(x+70, y, 3, ma);
    GD.cmd_number(x+242, y+2, 1, 3, ua);
    GD.cmd_text(x+410, y+2 ,  1, 0, "A");
  } else if (ma > 0 or (ma==0 &&  ua>100 && uA_mode_enabled==false) or uA_mode_enabled == false){
    boldNumber(x, y, 3, ma);
    boldText(x+162, y+30-30, ".");
    GD.cmd_number(x+192, y+2, 1, 3, ua);

    GD.cmd_number(x+370, y+2, 1, 1, (int)(na/100.0)); // use only one digit for nano amps !
    GD.cmd_text(x+470, y+2 ,  1, 0, "mA");
    } 
    else if (uA_mode_enabled == true) {
    // Not sure if a separate uA is needed. nA is probably ruined by noise and offset anyway...
      boldNumber(x, y, 3, ua);
      boldText(x+162, y+30-30, ".");
      GD.cmd_number(x+192, y+2, 1, 2, (int)(na/10.0)); // use only two digit for nano amps !
      GD.cmd_text(x+320, y+2 ,  1, 0, "uA");
    }


  
}

void CurrentDisplayClass::renderSet(int x, int y, float rawMa) {
  
  int a, ma, ua;
  bool neg;

  DIGIT_UTIL.separate(&a, &ma, &ua, &neg, rawMa);
  
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
