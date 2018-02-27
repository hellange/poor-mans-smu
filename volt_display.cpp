#include "volt_display.h"
#include "GD2.h"
#include "colors.h"


void VoltDisplayClass::separate(int *v, int *mv, int *uv, float rawMv) {
  *v = (int)(rawMv / 1000.0f);
  *mv = (int)(rawMv - *v * 1000.0f);
  *uv = (int)((rawMv - *mv) * 1000.0f);
}

void VoltDisplayClass::renderMeasured(int x, int y, float rawMv) {
  if (rawMv < 0.0f) {
    rawMv = 0.0f - rawMv;
    *sign = '-';
  } else {
    *sign = '+';
  }

  int v, mv, uv;
  separate(&v, &mv, &uv, rawMv);
  

  GD.ColorRGB(COLOR_VOLTAGE_SHADDOW);
  GD.cmd_number(x+80+6-17, y+48 , 1, 2, v);
  GD.cmd_text(x+183+6-17-1, y+48 , 1, 0, ".");
  GD.cmd_number(x+205+6-17, y+48 , 1, 3, mv);

  GD.ColorRGB(COLOR_VOLT);
  boldText(x,y+42, sign);
  boldNumber(x+63,y+42, 2, v);
  boldText(x+167,y+42, ".");
  boldNumber(x+188,y+42, 3, mv);

  GD.cmd_number(x+357, y+44, 1, 3, uv);
  GD.cmd_text(x+530, y+44 ,  1, 0, "V");  
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
  GD.cmd_number(x+85, y, 31, 3, mv);
  GD.cmd_text(x+73, y, 31, 0, ".");
  GD.cmd_number(x+165, y, 31, 1, uv / 100); // need to do it like this to show only first digit in a three digit value
  GD.cmd_number(x+25, y, 31, 2, v);
  GD.cmd_text(x+195, y, 31, 0, "V");
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
