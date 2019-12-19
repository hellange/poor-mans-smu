#include "FunctionPulse.h"

#include <SPI.h>
#include "GD2.h"
#include "tags.h"
#include "colors.h"

FunctionPulseClass FUNCTION_PULSE;

void FunctionPulseClass::init(ADCClass& smu_) {
  smu = smu_;
}
void FunctionPulseClass::open(void (*closedFn_)(int type)) {
  closedFn = closedFn_;
}

void FunctionPulseClass::close() {
      closedFn(999); 
}

void FunctionPulseClass::handleButtonAction(int inputTag) {
}

void FunctionPulseClass::render(int x, int y) {

  float max = 2000.0;
  float min = -2000.0;
    // heading
      GD.ColorRGB(COLOR_VOLT);

  GD.ColorA(120);
  GD.cmd_text(x+20, y + 2 ,   29, 0, "SOURCE PULSE");
  GD.cmd_text(x+20 + 1, y + 2 + 1 ,   29, 0, "SOURCE PULSE");
  float hz = 2;
  float duration = 1000.0/hz;
  GD.ColorA(255);
  y=y+30;
  GD.cmd_number(x+242, y, 1, 3, hz);
  GD.cmd_text(x+410, y ,  1, hz, "Hz");
  y=y+110;
  if (min < 0) {
    GD.cmd_text(x+220, y ,  31, hz, "-");
  }
  GD.cmd_number(x+242, y, 31, 4, abs(min));
  GD.cmd_text(x+345, y ,  31, hz, "to");
  GD.cmd_number(x+385, y, 31, 4, max);
  GD.cmd_text(x+480, y ,  31, hz, "mV");

  GD.__end();
  smu.pulse(min, max, duration);
  GD.resume();
  //SMU[0].sweep(5.00, -5.00, 0.1, 5000);
  GD.ColorA(255);

}
