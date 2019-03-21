
#include <stdint.h>
#include <Arduino.h>

#include "AD7176_regs.h"
#include "AD7176_Comm.h"
#include "AD7176.h"

#define FSR (((long int)1<<23)-1)

// goal is to be "compatible" with the Poor Mans SMU hardware abstraction layer API
// to make it easy to test different ADC without changing the application software...
class ADCClass {
  private:
    float VREF = 2.5000;             
    float VFSR = VREF; 
  public:
    void init(void);
    bool dataReady();
    float measureMilliVoltage();
};

extern ADCClass ADC2;

