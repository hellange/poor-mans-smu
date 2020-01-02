#include "SMU_HAL_717x.h"


class FunctionPulseClass {

  void (*closedFn)(int type);
  
  private:
    ADCClass smu;
  public:
    void init(ADCClass& smu);
    void open(OPERATION_TYPE operationType,void (*closedFn)(int type));
    void close();
    void handleButtonAction(int inputTag);
    void render(int x, int y);
  };

extern FunctionPulseClass FUNCTION_PULSE;
