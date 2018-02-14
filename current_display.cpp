#include "current_display.h"
#include "volt_display.h" // borrows bold method

#include "GD2.h"
#include "colors.h"

void CurrentDisplayClass::renderMeasured(int x, int y, float rawMa) {

  int a = rawMa / 1000;
  int ma = (rawMa - a * 1000);
  int ua = (rawMa - ma) * 1000;
  
  GD.ColorRGB(COLOR_CURRENT_SHADDOW);
  GD.cmd_number(x+80+6-17, y+36 ,   1, 1, a);
  GD.cmd_text(x+130+6-17, y+36 ,   1, 0, ".");
  GD.cmd_number(x+153+6-17, y+36 ,   1, 3, ma);
  
  GD.ColorRGB(COLOR_CURRENT);
  
  VOLT_DISPLAY.boldText(x+17-17,y+30, "+");
  VOLT_DISPLAY.boldNumber(x+80-17, y+30, 1, a);
  VOLT_DISPLAY.boldText(x+130-17, y+30, ".");
  VOLT_DISPLAY.boldNumber(x+153-17, y+30, 3, ma);

  GD.cmd_number(x+322-17, y+32, 1, 3, ua);
  GD.cmd_text(x+495-17, y+32 ,  1, 0, "A");
}

void CurrentDisplayClass::renderSet(int x, int y, float rawMa) {
  
  int a = rawMa / 1000;
  int ma = (rawMa - a * 1000);
  int ua = (rawMa - ma) * 1000;

  if (rawMa < 0) {
      GD.cmd_text(x, y, 31, 3, "-");
  } else {
      GD.cmd_text(x, y, 31, 3, "+");
  }
  GD.cmd_number(x+25, y, 31, 1, a);
  GD.cmd_text(x+25+25-2, y, 31, 0, ".");
  GD.cmd_number(x+35+25, y, 31, 3, ma);
  GD.cmd_number(x+115+25, y, 31, 1, ua / 100); // need to do it like this to show only first digit in a three digit value
  GD.cmd_text(x+145+25, y, 31, 0, "A");
}

CurrentDisplayClass CURRENT_DISPLAY;
