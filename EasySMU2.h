#include "Arduino.h"

class EasySMU2
{
  private:

  public:
   
    int8_t fltSetCommitVoltageSource(float fVoltage);
    int8_t fltSetCommitCurrentSource(float fCurrent, int8_t up_down_both);
    float MeasureVoltage();
    float MeasureCurrent();
};
