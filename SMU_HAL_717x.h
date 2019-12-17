
#include <stdint.h>
#include <Arduino.h>

#include "AD7176_regs.h"
#include "AD7176_Comm.h"
#include "AD7176.h"
#include "LTC2758.h"
#include "operations.h"

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
    CURRENT_RANGE current_range = AMP1;
    uint32_t sourcecurrent_to_code_adj(float dac_voltage, float min_output, float max_output, bool serialOut);
  public:

    bool compliance;
    int init();
    int8_t fltSetCommitVoltageSource(float fVoltage);
    int8_t fltSetCommitCurrentSource(float fVoltage);
    int8_t fltSetCommitLimit(float fCurrent, int8_t up_down_both);
    float measureMilliVoltage();
    void setCurrentRange(CURRENT_RANGE range);

    int dataReady();
    float measureCurrent(CURRENT_RANGE range);
    
    float getSetValuemV();
    float getLimitValue();

     void pulse(float v1, float v2, int duration);
      void sweep(float high, float low, float step, int duration);

};

//extern ADCClass ADC2;
