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
void CurrentDisplayClass::renderMeasured(int x, int y, float rawMa, boolean compliance, bool show_nA) {
 
  int complianceColor = blinkColor(0xff4522, 0x991002, 1000);

  int a, ma, ua, na;
  bool neg;

  DIGIT_UTIL.separate2(&a, &ma, &ua, &na, &neg, rawMa);

  GD.ColorA(255);
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
  } else if (ma > 0 or (ma==0 &&  ua>100 && show_nA==false) or show_nA == false){
    boldNumber(x, y, 3, ma);
    boldText(x+162, y+30-30, ".");
    GD.cmd_number(x+192, y+2, 1, 3, ua);

    GD.cmd_number(x+370, y+2, 1, 1, (int)(na/100.0)); // use only one digit for nano amps !
    GD.cmd_text(x+470, y+2 ,  1, 0, "mA");
    } 
    else if (show_nA) {
    // Show uA with nA decimals...
      boldNumber(x, y, 3, ua);
      boldText(x+162, y+30-30, ".");
      GD.cmd_number(x+192, y+2, 1, 2, (int)(na/10.0)); // use only two digit for nano amps !
      GD.cmd_text(x+320, y+2 ,  1, 0, "uA");
      //GD.cmd_number(x+192, y+2, 1, 3, (int)(na)); 
      //GD.cmd_text(x+355, y+2 ,  1, 0, "uA");
    }


  
}

void CurrentDisplayClass::renderSet(int x, int y, float rawMa) {
  
  int a, ma, ua;
  bool neg;

  DIGIT_UTIL.separate(&a, &ma, &ua, &neg, rawMa);
  
  GD.ColorRGB(COLOR_CURRENT);
  //Serial.print("RENDER SET:");
  //Serial.print(rawMa);

  if (rawMa < 0.0) {
    GD.cmd_text(x, y, 31, 0, "-");
    x=x+20;
  }
  if (abs(rawMa) >= 10.0) {
    GD.cmd_number(x+25-20, y, 31, 1, a);
    GD.cmd_text(x+50-20, y, 31, 0, ".");
    GD.cmd_number(x+60-20, y, 31, 3, ma);
    GD.cmd_number(x+140-20, y, 31, 1, ua / 100); // need to do it like this to show only first digit in a three digit value
    GD.cmd_text(x+170-20, y, 31, 0, "A");
  } else {
    GD.cmd_number(x+25-20, y, 31, 2, ma);
    GD.cmd_text(x+50+20-20, y, 31, 0, ".");
    GD.cmd_number(x+60+20-20, y, 31, 3, ua);
    GD.cmd_text(x+140+20-20, y, 31, 0, "mA");
  }
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
