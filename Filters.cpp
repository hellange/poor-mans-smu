#include "Filters.h"
#include "Arduino.h"

void FiltersClass::init() {
  filterSize = 1;
  Serial.print("Set initial filter length to:");
  Serial.println(filterSize);
  for (int i=0; i<FILTER_MAX_LENGTH;i++){
    meanRaws[i] = 0.0;
  }
}

void FiltersClass::setFilterSize(int size) {
  if (size<1) {
    Serial.println("WARNING: Set to minium allowed filter size:");
    filterSize = 1;
  } else if(size <= FILTER_MAX_LENGTH) {
    filterSize = size;
    Serial.print("Set filter size to:");
  } else  {
    Serial.println("WARNING: Reduced filter size to max allowed:");
  }
  Serial.println(filterSize);
}


float FiltersClass::updateMean(float v){
  int fz = filterSize;
  
  // shift everything one step
  // TODO: Improve to use pointer into latest field instead of having to shift all entries...
  for (int i=fz-1;i>0;i--){
    meanRaws[i] = meanRaws[i-1];
  }
  meanRaws[0] = v;

  // Do the actual calculation from relevant samples
  float tot = 0;
  for (int i=0;i<fz;i++){
    tot = tot + meanRaws[i];
  }
  mean = tot / fz;

  return mean;
}
