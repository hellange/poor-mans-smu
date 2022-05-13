#ifndef __HAL_DUMMY_CLASS_H__
#define __HAL_DUMMY_CLASS_H__

#include "Arduino.h"
#include "operations.h"

class SMU_HAL_dummy
{
  int lastSampleMilli;
  float samplingDur;
  //float nowValuemV = 0.0;
  //float nowValueI = 0.0;
  float setValueI = 0.0;
  float setValuemV = 0.0;
  int volt_current = 0;
  float drift = 0.0;
  int driftTimer = millis();
  int driftDirection = 1;

private:     
CURRENT_RANGE current_range = AMP1;

  public:
      int deviceTypeId =111; // something else that 0
        float simulatedLoad = 10.0;
          float simulatedOffset = 0.110; // simulate some uV offset


    OPERATION_TYPE operationType;

  //TODO: SHould not be needed in simulator !!!! Indicates poor architecture...
  float DAC_RANGE_LOW = 10.0;
  float DAC_RANGE_HIGH = 10.0;
  int64_t setLimit_micro = 0;
  int64_t setValue_micro = 0;

  
  bool compliance;
    int init();
    int64_t fltSetCommitVoltageSource(int64_t voltage_uV, bool dynamicRange);
    int64_t fltSetCommitCurrentSource(int64_t current_uA);
    int64_t fltSetCommitCurrentLimit(int64_t current_uA, int8_t up_down_both);
    int64_t fltSetCommitVoltageLimit(int64_t voltage_uV, int8_t up_down_both);
        int64_t getSetValue_micro();

        int64_t getLimitValue_micro();

    float measureMilliVoltage();
    //void setCurrentRange(CURRENT_RANGE range);
        CURRENT_RANGE getCurrentRange();

    void setCurrentRange(CURRENT_RANGE range, OPERATION_TYPE operationType);

    void setSamplingRate(int value);
    int getSamplingRate();

    bool hasCompliance();

    int dataReady();
    float measureCurrent(int range);

    void setGPIO(int nr, bool on);

    //float getSetValuemV();
    float getLimitValue();
    bool use100uVSetResolution();

    void disable_ADC_DAC_SPI_units();


};

#endif 
