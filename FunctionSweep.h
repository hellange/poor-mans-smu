#include "SMU_HAL_717x.h"


class FunctionSweepClass {

  void (*closedFn)(int type);
  
  private:
    ADCClass smu;
    float currentSweepValue = 0.0;
    int currentSweepDir = 1;
    int pulseTimer;
    void operateSmu(float high, float low, float step, int duration);
  public:
    void init(ADCClass& smu);
    void open(void (*closedFn)(int type));
    void close();
    void handleButtonAction(int inputTag);
    void render(int x, int y);
  };

extern FunctionSweepClass FUNCTION_SWEEP;
