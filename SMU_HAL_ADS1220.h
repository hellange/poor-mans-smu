#include "SPI.h" 
#include "Protocentral_ADS1220.h"

#define FSR (((long int)1<<23)-1)
#define PGA 1                 // Programmable Gain = 1
#define VREF 2.048            // Internal reference of 2.048V
#define VFSR VREF/PGA   

class SMU_HAL_ADS1220 {

  volatile byte MSB;
  volatile byte data;
  volatile byte LSB;
  volatile byte *SPI_RX_Buff_Ptr;
  Protocentral_ADS1220 ADS1220;
  int nrBeforeAveraging;

  // used to simulate nonexisting features
  float nowValueI = 0.0;
  float setValueI = 0.0;
  float nowValueV = 0.0;
  float setValueV = 0.0;

  //--------------------------------------
  
  float convertToMv();
  float smoothing(float average, int size, float value);

public:
  void init();
  bool dataReady();
  float measureVoltage();

  // used to simulate nonexisting features
  int8_t fltSetCommitVoltageSource(float fVoltage);
  int8_t fltSetCommitCurrentSource(float fCurrent, int8_t up_down_both);
  float measureCurrent();
  bool compliance();

  float getSetValuemV();
};


