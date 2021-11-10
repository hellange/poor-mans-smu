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



public:
  void init(OPERATION_TYPE operationType_);
  bool loopDigitize();
  void a();
  bool handleSamplingForDigitizer(int dataR);

  bool bufferOverflow = false;
  bool digitize = false;
int samplesAfterTrigger;
  int countSinceLastSample;

  int adrAtTrigger;
  int allowTrigger;

};



extern DigitizerClass DIGITIZER;
#endif
