#include "Filters.h"


void FiltersClass::init() {
}


float FiltersClass::updateMean(float v){
  float tot = 0;
  for (int i=FILTER_LENGTH-1;i>0;i--){
    meanRaws[i] = meanRaws[i-1];
    tot = tot + meanRaws[i+1];
  }
  meanRaws[0] = v;
  tot = tot + v;
  
  mean = tot / FILTER_LENGTH;
  return mean;
}



