#include "SimpleStats.h"


void SimpleStatsClass::init() {
  minimum = undefinedValue;
  maximum = undefinedValue;
  samples = 0;
}

void SimpleStatsClass::clear() {
  minimum = undefinedValue;
  maximum = undefinedValue;
  samples = 0;
}

void SimpleStatsClass::registerValue( float value) {
  samples ++;
  // Don't trust the first few samples...
  if ((minimum == undefinedValue || maximum == undefinedValue) && samples>10 ) {
    minimum = value;
    maximum = value;
  } else if (value < minimum) {
    minimum = value;
  } else if (value > maximum) {
    maximum = value;
  }
   
}
