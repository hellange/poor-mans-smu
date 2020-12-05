#ifndef __HAL_DUMMY_CLASS_H__
#define __HAL_DUMMY_CLASS_H__

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
  float drift = 0.0;
  int driftTimer = millis();
  int driftDirection = 1;

  public:

  //TODO: SHould not be needed in simulator !!!! Indicates poor architecture...
  float DAC_RANGE_LOW;
  float DAC_RANGE_HIGH;

  
  bool compliance;
    int init();
    int8_t fltSetCommitVoltageSource(float mv, bool dynamicRange);
    int8_t fltSetCommitCurrentSource(float fVoltage);
    int8_t fltSetCommitCurrentLimit(float fCurrent, int8_t up_down_both);
    int8_t fltSetCommitVoltageLimit(float fCurrent, int8_t up_down_both);
    
    float measureMilliVoltage();
    //void setCurrentRange(CURRENT_RANGE range);
    void setCurrentRange(CURRENT_RANGE range, OPERATION_TYPE operationType);

    void setSamplingRate(int value);
    int getSamplingRate();

    bool hasCompliance();

    int dataReady();
    float measureCurrent(int range);

    void setGPIO(int nr, bool on);

    float getSetValuemV();
    float getLimitValue();
    void use100uVSetResolution();

    void disable_ADC_DAC_SPI_units();


};

#endif 
