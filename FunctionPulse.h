#include "SMU_HAL_717x.h"


class FunctionPulseClass {

  void (*closedFn)(int type);
  
  private:
    ADCClass smu;
    void sourceVoltagePulse(float high, float low, int duration);
    void sourceCurrentPulse(float high, float low, int duration);

    int pulseTimer = millis();
    int pulseHigh = false;
    OPERATION_TYPE operationType;
  public:
    void init(ADCClass& smu);
    void open(OPERATION_TYPE operationType,void (*closedFn)(int type));
    void close();
    void handleButtonAction(int inputTag);
    void render(int x, int y);
  };

extern FunctionPulseClass FUNCTION_PULSE;
