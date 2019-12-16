
#include <stdint.h>
#include <Arduino.h>

#include "AD7176_regs.h"
#include "AD7176_Comm.h"
#include "AD7176.h"
#include "LTC2758.h"

#define FSR (((long int)1<<23)-1)

// goal is to be "compatible" with the Poor Mans SMU hardware abstraction layer API
// to make it easy to test different ADC without changing the application software...
class ADCClass {

  unsigned long lastSampleMilli;

  float setValueI = 0.0;
  float setValueV = 0.0;
  
  private:
    float VREF = 5.0; // 2.500; //2.5357;             
    float VFSR = VREF; 
  public:
    bool compliance;
    int init();
    int8_t fltSetCommitVoltageSource(float fVoltage);
    int8_t fltSetCommitCurrentSource(float fVoltage);
    int8_t fltSetCommitLimit(float fCurrent, int8_t up_down_both);
    float measureMilliVoltage();
    void setCurrentRange(int range);

    int dataReady();
    float measureCurrent(int range);
    float getSetValuemV();
    float getLimitValue();
};

//extern ADCClass ADC2;
