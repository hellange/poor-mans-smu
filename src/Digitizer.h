#ifndef DIGITIZER_H
#define DIGITIZER_H
#include "operations.h"
#include "SMU_HAL_717x.h"
#include <SPI.h>
#include "GD2.h"
#include "volt_display.h"
#include "Mainmenu.h"

extern MainMenuClass MAINMENU;

class DigitizerClass {

private:

  float maxDigV = -100000.00, minDigV = 100000.00;
  int ramAdrPtr = 0;
  int nrOfFloats = 400;
  float mva[400];
  float mia[400]; 
  bool triggered = false;
  float lastVoltage = 0.0;
  float ramEmulator[1000];
  float ramEmulator2[1000];

  // for test
  float simulatedWaveform;

  void copyDataBufferToDisplayBuffer();

public:
  void clearMaxMin();
  void init(OPERATION_TYPE operationType_);
  void loopDigitize();
  void renderGraph();
  void a();
  void handleSamplingForDigitizer(int dataR);

  bool bufferOverflow = false;
  bool digitize = false;
  int samplesAfterTrigger;
  int countSinceLastSample;
  void close();

  int adrAtTrigger;
  bool allowTrigger = true;
  bool rendering = false;

};



extern DigitizerClass DIGITIZER;
#endif