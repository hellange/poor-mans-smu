#ifndef __HAL_DUMMY_CLASS_H__
#define __HAL_DUMMY_CLASS_H__

#include "Arduino.h"
#include "operations.h"
#include "SMU_HAL.h"

#define SMU_HAL_DUMMY_DEVICE_TYPE_ID 111
class SMU_HAL_dummy: public SMU_HAL
{
  unsigned long lastSampleMilli;
  float setValueI = 0.0;
  float setValuemV = 0.0;
  int volt_current = 0;
  float drift = 0.0;
  int driftTimer = millis();
  int driftDirection = 1;
  bool compliance;

private:     
    CURRENT_RANGE current_range = AMP1;
    int samplingRate = 5;
    float simulatedLoad = 100.0; // ohm
    float simulatedOffset = 0.110; // simulate some uV offset

    bool voltageMeasurementEnabled = true;
    bool currentMeasurementEnabled = true;
  public:

    int deviceTypeId = SMU_HAL_DUMMY_DEVICE_TYPE_ID; // // Identifying ADC type. For simulator, just set something else that 0

    OPERATION_TYPE operationType;
    //TODO: SHould not be needed in simulator !!!! Indicates poor architecture...
    float DAC_RANGE_LOW = 10.0;
    float DAC_RANGE_HIGH = 10.0;
    int64_t setLimit_micro = 0;
    int64_t setValue_micro = 0;
  
    int init();
    int64_t fltSetCommitVoltageSource(int64_t voltage_uV, bool dynamicRange);
    int64_t fltSetCommitCurrentSource(int64_t current_uA);
    int64_t fltSetCommitCurrentLimit(int64_t current_uA);
    int64_t fltSetCommitVoltageLimit(int64_t voltage_uV);
    int64_t getSetValue_micro();

    int64_t getLimitValue_micro();

    double measureMilliVoltage();

    void updateSettings();

    CURRENT_RANGE getCurrentRange();

    void setCurrentRange(CURRENT_RANGE range, OPERATION_TYPE operationType);

    void setSamplingRate(int value);
    int getSamplingRate();

    bool hasCompliance();

    int dataReady();
    double measureCurrent(CURRENT_RANGE range);

    void setGPIO(int nr, bool on);

    //float getLimitValue();
    bool use100uVSetResolution();

    void disable_ADC_DAC_SPI_units();
    
    void shortAdcInput(bool setShort);
    void internalRefInput(bool setInternalRef);



    void enableVoltageMeasurement(bool enable);
    void enableCurrentMeasurement(bool enable);
    bool isVoltageMeasurementEnabled();
    bool isCurrentMeasurementEnabled();
    // TODO make as method
    bool voltageMeasurementGainX2 = false;

};

#endif 
