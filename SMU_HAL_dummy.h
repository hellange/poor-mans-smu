#include "Arduino.h"
#include "operations.h"

class SMU_HAL_dummy
{
  int lastSampleMilli;
  float samplingDur;
  float nowValuemV = 0.0;
  float nowValueI = 0.0;
  float setValueI = 0.0;
  float setValuemV = 0.0;
  int volt_current = 0;
  public:
  bool compliance;
    int init();
    int8_t fltSetCommitVoltageSource(float mv, bool dynamicRange);
    int8_t fltSetCommitCurrentSource(float fVoltage);
    int8_t fltSetCommitLimit(float fCurrent, int8_t up_down_both);
    float measureMilliVoltage();
        void setCurrentRange(CURRENT_RANGE range);
            void setSamplingRate(int value);



    int dataReady();
    float measureCurrent(int range);

    float getSetValuemV();
    float getLimitValue();

};
