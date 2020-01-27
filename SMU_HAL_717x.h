#ifndef __ADCCLASS_H__
#define __ADCCLASS_H__

#include <stdint.h>
#include <Arduino.h>

#include "AD7176_regs.h"
#include "AD7176_Comm.h"
#include "AD7176.h"
#include "LTC2758.h"
#include "operations.h"

//#define FSR (((long int)1<<23)-1)

// goal is to be "compatible" with the Poor Mans SMU hardware abstraction layer API
// to make it easy to test different ADC without changing the application software...
class ADCClass {

  long int FSR = (((long int)1<<23)-1);
  unsigned long lastSampleMilli;

  float setValueI = 0.0;
  float setValueV = 0.0;
  
  private:
    float VREF = 5.0;            
    float VFSR = VREF; 
    CURRENT_RANGE current_range = AMP1;
    int samplingRate = 5;
    int oldSamplingRate = 5;
    void writeSamplingRate();

  public:

    bool compliance;
    int init();
    int initADC();
    int initDAC();
 
    int8_t fltSetCommitVoltageSource(float fVoltage, bool dynamicRange);
    int8_t fltSetCommitCurrentSource(float fVoltage);
    int8_t fltSetCommitLimit(float fCurrent, int8_t up_down_both);
    float measureMilliVoltage();
    float measureMilliVoltageRaw();
    void setCurrentRange(CURRENT_RANGE range);
    void setSamplingRate(int value);
    int dataReady();
    float measureCurrent(CURRENT_RANGE range);
    
    float getSetValuemV();
    float getLimitValue();

};

#endif
