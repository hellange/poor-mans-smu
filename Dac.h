#include "SPI.h" 
#include "Protocentral_ADS1220.h"

#define FSR (((long int)1<<23)-1)
#define PGA 1                 // Programmable Gain = 1
#define VREF 2.048            // Internal reference of 2.048V
#define VFSR VREF/PGA   

class DacClass {

  volatile byte MSB;
  volatile byte data;
  volatile byte LSB;
  volatile byte *SPI_RX_Buff_Ptr;
  Protocentral_ADS1220 ADS1220;
  int nrBeforeAveraging;

private:
  float convertToMv();
  float smoothing(float average, int size, float value);

public:
  void init();
  bool dataReady();
  float MeasureVoltage();
};

extern DacClass DAC;
