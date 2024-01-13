#include "SMU_HAL_717x.h"

// Set global because it's used by interrupts... other ways to fix this ?
static IntervalTimer myPulseTimer;

class FunctionPulseClass {

  void (*closedFn)(OPERATION_TYPE type);
  private:
    static float measuredLow;
    static float measuredHigh;
    static void sourceVoltagePulse(float high, float low, int duration);
    static void sourceCurrentPulse(float high, float low, int duration);
    static void updateSamplingPeriod(int hz);
    static int pulseHigh;
    static int hz;
    static float duration;
    static OPERATION_TYPE operationType;
    static int knobOperation; //TODO: enum ?
  public:
    static float max;
    static float min;
    static void sourcePulse();
    static void init();
    void open(OPERATION_TYPE operationType,void (*closedFn)(OPERATION_TYPE type));
    void close();
    void handleButtonAction(int inputTag);
    static void render(int x, int y, bool reduceDetails);
    static void rotaryEncChanged(float changeValue);
    static void rotaryEncButtonChanged(int key, bool quickPress, bool holdAfterLongPress, bool releaseAfterLongPress);
  };


extern FunctionPulseClass FUNCTION_PULSE;
