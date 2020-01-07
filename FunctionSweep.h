#include "SMU_HAL_717x.h"


class FunctionSweepClass {

  void (*closedFn)(OPERATION_TYPE type);
  
  private:
    //ADCClass smu;
    float currentSweepValue = 0.0;
    int currentSweepDir = 1;
    OPERATION_TYPE operationType;
    int pulseTimer;
    void operateSmuVoltage(float high, float low, float step, int duration);
    void operateSmuCurrent(float high, float low, float step, int duration);

  public:
    void init(/*ADCClass& smu*/);
    void open(OPERATION_TYPE operationType, void (*closedFn)(OPERATION_TYPE type));
    void close();
    void handleButtonAction(int inputTag);
    void render(int x, int y);
  };

extern FunctionSweepClass FUNCTION_SWEEP;
