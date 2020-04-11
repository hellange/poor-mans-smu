
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
 
};

extern TrendGraphClass TRENDGRAPH;
