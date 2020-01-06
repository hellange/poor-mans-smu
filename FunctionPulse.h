#include "SMU_HAL_717x.h"


class FunctionPulseClass {

  void (*closedFn)(OPERATION_TYPE type);
  IntervalTimer myPulseTimer;



  private:
    ADCClass smu;
    static float measuredLow;
    static float measuredHigh;
    static void sourceVoltagePulse(float high, float low, int duration);
    static void sourceCurrentPulse(float high, float low, int duration);
     void updateSamplingPeriod(int hz);
    static int pulseTimer;
    static int pulseHigh;
    static int hz;
   
    static float duration;
    static OPERATION_TYPE operationType;
  public:
    
   static float max;
    static float min;
  static void sourcePulse();
    void init(ADCClass& smu);
    void open(OPERATION_TYPE operationType,void (*closedFn)(OPERATION_TYPE type));
    void close();
    void handleButtonAction(int inputTag);
    static void render(int x, int y);
  };


extern FunctionPulseClass FUNCTION_PULSE;
