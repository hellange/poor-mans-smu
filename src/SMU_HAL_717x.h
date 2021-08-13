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

  int64_t setLimit_micro = 0;
  int64_t setValue_micro = 0;

  
  private:
    double VREF = 5.013; //4.96625;  //5.01156          
    double VFSR = VREF; 
    CURRENT_RANGE current_range = AMP1;
    int samplingRate = 5;
    int oldSamplingRate = 5;
    void writeSamplingRate();

  
  public:
    OPERATION_TYPE operationType;

    // TODO: Why public ? Indicates poor architecture
    float DAC_RANGE_LOW;
    float DAC_RANGE_HIGH;


    void init();
    void initADC();
    void initDAC();
    void disable_ADC_DAC_SPI_units();
    int64_t fltSetCommitVoltageSource(int64_t voltage_uV, bool dynamicRange);
    int64_t fltSetCommitCurrentSource(int64_t current_uA);

    int64_t fltSetCommitCurrentLimit(int64_t setValue_uA, int8_t up_down_both);
    int64_t fltSetCommitVoltageLimit(int64_t voltage_uV, int8_t up_down_both);

    double measureMilliVoltage();
    double measureMilliVoltageRaw();
    void setCurrentRange(CURRENT_RANGE range, OPERATION_TYPE operationType);
    void setSamplingRate(int value);
    int getSamplingRate();
    int dataReady();
    double measureCurrent(CURRENT_RANGE range);

    bool hasCompliance();
    int64_t getSetValue_micro();
    int64_t getLimitValue_micro();

    void setGPIO(int nr, bool on);

    bool use100uVSetResolution();
};

#endif
