#ifndef DIGITIZER_H
#define DIGITIZER_H
#include "operations.h"
#include "SMU_HAL.h"
#include <SPI.h>
#include "GD2.h"
#include "volt_display.h"
#include "Mainmenu.h"
#include "digit_util.h"
#include "colors.h"

extern MainMenuClass MAINMENU;

class DigitizerClass {

private:
  SMU_HAL *SMU1;
  float v; 

  float maxDigV = -100000.00, minDigV = 100000.00;
  int ramAdrPtr = 0;
  int nrOfFloats = 400;
  float mva[400];
  float mia[400]; 
  bool triggered = false;
  
  // Used for edge detect
  float lastVoltage = 0.0;
  float lastVoltageOld = 0.0;
  float lastVoltageOld2 = 0.0;
    float lastVoltageOld3 = 0.0;

bool waitForLower = false;

  float ramEmulator[1000];
  float ramEmulator2[1000];




  // for test
  float simulatedWaveform;

  void copyDataBufferToDisplayBuffer();

  void updateSmuWhenVoltageCurrentFocusChange();


  // for test
  void updateModulation();
  void updateMaxMin(float v);

  int prevSamplingRate;
public:
    static bool zoomed;
    static int adjustLevel;
    static int ampLevel;
    static float triggerLevel;
    static int multiplyBy;

  bool digitizeVoltage = true; // set to true to digitize voltage 

  bool continuous = true; // default continous sampling, not trigger

  void clearMaxMin();
  void init(SMU_HAL &SMU, OPERATION_TYPE operationType_);
  void open();
  void loopDigitize(bool reduceDetails);
  void renderGraph(bool reduceDetails);
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

  static void rotaryEncChanged(float changeValue);
  static void rotaryEncButtonChanged(int key, bool quickPress, bool holdAfterLongPress, bool releaseAfterLongPress);

};



extern DigitizerClass DIGITIZER;
#endif
