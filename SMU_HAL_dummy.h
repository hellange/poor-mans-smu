#include "Arduino.h"

class SMU_HAL_dummy
{
  private:

  public:
    int8_t fltSetCommitVoltageSource(float fVoltage);
    int8_t fltSetCommitCurrentSource(float fCurrent, int8_t up_down_both);
    float MeasureVoltage();
    bool dataReady();
    float MeasureCurrent();
    boolean Overflow();
};
