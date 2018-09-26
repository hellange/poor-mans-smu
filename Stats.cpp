#include "Stats.h"
#include "GD2.h"

void StatsClass::addSample(float rawMv) {
      value[endPtr] = rawMv;

      endPtr ++;
      if (endPtr > nrOfTrendPoints - 1) {
        endPtr = 0;
      } 
      minimum = 100000.0;  // +100V
      maximum = -100000.0; // -100V
      updateMinMax(rawMv);
    
      span = maximum - minimum;
}


void StatsClass::updateMinMax(float rawMv) {
    for (int i=0;i<nrOfTrendPoints;i++) {
        if (value[i]<=minimum) {
          minimum = value[i];
        }
        if (value[i]>=maximum) {
          maximum = value[i];
        }
      }
}

void StatsClass::renderTrend(int x, int y , bool small) {  
    
    visibleMax = maximum;
    visibleMin = minimum;

//    // set a mimimum span so very small changes in uV sonw show up in full scale...
//    float diff = maximum - minimum;
//    float minVisibleSpan = 0.200f;
//    if (diff < minVisibleSpan) {
//      visibleMax = maximum + ((minVisibleSpan - diff)/2.0f);
//      visibleMin = minimum - ((minVisibleSpan - diff)/2.0f);
//    }
    
    uispan = visibleMax - visibleMin;

    int multiplyBy = 1;
    if (!small) {
      multiplyBy = 2;
    }
    
    int i = endPtr;
    GD.Begin(LINE_STRIP);
    GD.LineWidth(multiplyBy * 10);
 
    for (int pos=0; pos<nrOfTrendPoints;pos++) { 

      float height = 45.0f * ( (visibleMax - value[i]) / uispan);
      if (!small) {
        height=height*multiplyBy;
      }
      GD.Vertex2ii(x + pos*(1+multiplyBy), y + height); 
      i=i+1;
      if (i>nrOfTrendPoints - 1) {
        i=0;
      }  
    }

}


