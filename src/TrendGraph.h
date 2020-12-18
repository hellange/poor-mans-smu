
#include "Arduino.h"
#include <SPI.h>
#include "operations.h"
#include "RamClass.h"
#include "digit_util.h"


class TrendGraphClass {

private:
 
public:
 
  void init();
  void loop(OPERATION_TYPE operationType);
  static void rotaryChangedFn(float changeVal);

};

extern TrendGraphClass TRENDGRAPH;
