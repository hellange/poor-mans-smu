#ifndef __FUNCTIONSWEEPCLASS_H__
#define __FUNCTIONSWEEPCLASS_H__

#include "SMU_HAL.h"

class FunctionSweepClass {

  void (*closedFn)(OPERATION_TYPE type);
  
  private:
    SMU_HAL *SMU1;
    float previousSweepValues[2];
    float currentSweepValue = 0.0;
    int currentSweepDir = 1;
    OPERATION_TYPE operationType;
    int pulseTimer;
    void operateSmuVoltage(float high, float low, float step, int duration);
    void operateSmuCurrent(float high, float low, float step, int duration);

  public:
    void init(SMU_HAL &SMU);
    void open(OPERATION_TYPE operationType, void (*closedFn)(OPERATION_TYPE type));
    void close();
    void handleButtonAction(int inputTag);
    void render(int x, int y, bool reduceDetails);
    void handle();
    float duration = 5000;
  float step = 100.0;
  float high = 1000.0;
  float low = -1000.0;

  };

extern FunctionSweepClass FUNCTION_SWEEP;

#endif