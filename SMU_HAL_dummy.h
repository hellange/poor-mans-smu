#include "Arduino.h"

class SMU_HAL_dummy
{

  unsigned long lastSampleMilli;

  float nowValueV = 0.0;
  float nowValueI = 0.0;
  float setValueI = 0.0;
  float setValueV = 0.0;

  public:
    int init();
    int8_t fltSetCommitVoltageSource(float fVoltage);
    int8_t fltSetCommitCurrentSource(float fCurrent, int8_t up_down_both);
    float measureVoltage();
    bool dataReady();
    float measureCurrent();
    boolean compliance();

    float getSetValuemV();
};
