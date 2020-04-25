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
    float VREF = 4.96625;            
    float VFSR = VREF; 
    CURRENT_RANGE current_range = AMP1;
    int samplingRate = 5;
    int oldSamplingRate = 5;
    void writeSamplingRate();

  public:
float DAC_RANGE_LOW;
  float DAC_RANGE_HIGH;
    int init();
    int initADC();
    int initDAC();
    void disable_ADC_DAC_SPI_units();
    int8_t fltSetCommitVoltageSource(float fVoltage, bool dynamicRange);
    int8_t fltSetCommitCurrentSource(float fVoltage);
    int8_t fltSetCommitCurrentLimit(float fCurrent, int8_t up_down_both);
        int8_t fltSetCommitVoltageLimit(float fCurrent, int8_t up_down_both);

    float measureMilliVoltage();
    float measureMilliVoltageRaw();
    void setCurrentRange(CURRENT_RANGE range, OPERATION_TYPE operationType);
    void setSamplingRate(int value);
    int getSamplingRate();
    int dataReady();
    float measureCurrent(CURRENT_RANGE range);

    bool hasCompliance();
    float getSetValuemV();
    float getLimitValue();

    void setGPIO(int nr, bool on);

    bool use100uVSetResolution();
};

#endif
