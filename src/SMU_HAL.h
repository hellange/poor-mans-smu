#ifndef __HAL_CLASS_H__
#define __HAL_CLASS_H__

#include "Arduino.h"
#include "operations.h"

class SMU_HAL
{

  public:
    //TODO: Rename to id refering to ADC ?
    int deviceTypeId = 99; // something else that 0

    OPERATION_TYPE operationType;

    float DAC_RANGE_LOW = 10.0;
    float DAC_RANGE_HIGH = 10.0;
    int64_t setLimit_micro = 0;
    int64_t setValue_micro = 0;
  
    int init();
    virtual int64_t fltSetCommitVoltageSource(int64_t voltage_uV, bool dynamicRange) = 0;
    virtual int64_t fltSetCommitCurrentSource(int64_t current_uA)= 0;
    virtual int64_t fltSetCommitCurrentLimit(int64_t current_uA)= 0;
    virtual int64_t fltSetCommitVoltageLimit(int64_t voltage_uV)= 0;
    
    virtual int64_t getSetValue_micro();
    virtual int64_t getLimitValue_micro();

    virtual double measureMilliVoltage() = 0;
    virtual double measureCurrent(CURRENT_RANGE range) = 0;

    virtual void updateSettings();

    virtual CURRENT_RANGE getCurrentRange() = 0;
    virtual void setCurrentRange(CURRENT_RANGE range, OPERATION_TYPE operationType) = 0;

    virtual void setSamplingRate(int value) = 0;
    virtual int getSamplingRate();

    virtual bool hasCompliance() = 0;
    virtual int dataReady() = 0;
    virtual void setGPIO(int nr, bool on) = 0;
    virtual bool use100uVSetResolution() = 0;
    virtual void disable_ADC_DAC_SPI_units() = 0;
    virtual void shortAdcInput(bool setShort) = 0;
    virtual void internalRefInput(bool setInternalRef) = 0;

    bool enableVoltageMeasurement = true;
    bool enableCurrentMeasurement = true;
    bool voltageMeasurementGainX2 = false;

};

#endif 
