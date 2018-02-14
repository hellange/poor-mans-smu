#include "volt_display.h"
#include "GD2.h"
#include "colors.h"

void VoltDisplayClass::renderMeasured(int x, int y, float rawMv) {
  
  int v = rawMv / 1000;
  int mv = (rawMv - v * 1000);
  int uv = (rawMv - mv) * 1000;

  GD.ColorRGB(COLOR_VOLTAGE_SHADDOW);
  GD.cmd_number(x+80+6-17, y+48 , 1, 2, v);
  GD.cmd_text(x+183+6-17-1, y+48 , 1, 0, ".");
  GD.cmd_number(x+205+6-17, y+48 , 1, 3, mv);

  GD.ColorRGB(COLOR_VOLT);
  boldText(x+17-17,y+42, "+");
  boldNumber(x+80-17,y+42, 2, v);
  boldText(x+183-17-1,y+42, ".");
  boldNumber(x+205-17,y+42, 3, mv);

  GD.cmd_number(x+374-17, y+44, 1, 3, uv);
  GD.cmd_text(x+547-17, y+44 ,  1, 0, "V");  
}

void VoltDisplayClass::renderSet(int x, int y, float rawMv) {
  
  int v = rawMv / 1000;
  int mv = (rawMv - v * 1000);
  int uv = (rawMv - mv) * 1000;
  if (rawMv < 0) {
      GD.cmd_text(x, y, 31, 3, "-");
  } else {
      GD.cmd_text(x, y, 31, 3, "+");
  }
  GD.cmd_number(x+60+25, y, 31, 3, mv);
  GD.cmd_text(x+50+25-2, y, 31, 0, ".");
  GD.cmd_number(x+140+25, y, 31, 1, uv / 100); // need to do it like this to show only first digit in a three digit value
  GD.cmd_number(x+25, y, 31, 2, v);
  GD.cmd_text(x+170+25, y, 31, 0, "V");
}

void VoltDisplayClass::boldText(int x, int y, char* text) {
  GD.cmd_text(x, y, 1, 0, text);
  GD.cmd_text(x+3, y, 1, 0, text);
  GD.cmd_text(x, y+3, 1, 0, text);
  GD.cmd_text(x+3, y+3 , 1, 0, text);
}

void VoltDisplayClass::boldNumber(int x, int y, int digits, int number) {
  GD.cmd_number(x, y , 1, digits, number);
  GD.cmd_number(x+3, y , 1, digits, number);
  GD.cmd_number(x, y+3 , 1, digits, number);
  GD.cmd_number(x+3, y+3 , 1, digits, number);
}

VoltDisplayClass VOLT_DISPLAY;
