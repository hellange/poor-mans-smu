#include "SMU_HAL_717x.h"


class FunctionPulseClass {

  void (*closedFn)(int type);
  IntervalTimer myPulseTimer;

  private:
    ADCClass smu;

    static void sourceVoltagePulse(float high, float low, int duration);
    static void sourceCurrentPulse(float high, float low, int duration);
     void updateSamplingPeriod(int hz);
    static int pulseTimer;
    static int pulseHigh;
    float hz = 20;
   
    static float duration;
    static OPERATION_TYPE operationType;
  public:
   static float max;
    static float min;
  static void sourcePulse();
    void init(ADCClass& smu);
    void open(OPERATION_TYPE operationType,void (*closedFn)(int type));
    void close();
    void handleButtonAction(int inputTag);
    void render(int x, int y);
  };


extern FunctionPulseClass FUNCTION_PULSE;
