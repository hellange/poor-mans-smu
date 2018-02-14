#include "current_display.h"
#include "volt_display.h" // borrows bold method

#include "GD2.h"
#include "colors.h"

void CurrentDisplayClass::render(int x, int y, float rawMa) {

  int a = rawMa / 1000;
  int ma = (rawMa - a * 1000);
  int ua = (rawMa - ma) * 1000;
  
  GD.ColorRGB(COLOR_CURRENT_SHADDOW);
  GD.cmd_number(x+80+6, y+36 ,   1, 1, a);
  GD.cmd_text(x+130+6, y+36 ,   1, 0, ".");
  GD.cmd_number(x+153+6, y+36 ,   1, 3, ma);
  
  GD.ColorRGB(COLOR_CURRENT);
  
  VOLT_DISPLAY.boldText(x+17,y+30, "+");
  VOLT_DISPLAY.boldNumber(x+80, y+30, 1, a);
  VOLT_DISPLAY.boldText(x+130, y+30, ".");
  VOLT_DISPLAY.boldNumber(x+153, y+30, 3, ma);

  GD.cmd_number(x+322, y+32, 1, 3, random(0, 199));
  GD.cmd_text(x+495, y+32 ,  1, 0, "A");
}

CurrentDisplayClass CURRENT_DISPLAY;
